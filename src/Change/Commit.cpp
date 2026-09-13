#include "Change/Commit.h"
#include "Env/Data.h"
#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <chrono>
#include <cstring>

#ifdef WIN32
    #include <direct.h>
    #define mkdir(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define mkdir(path) mkdir(path, 0777)
#endif

using namespace git;

std::string change::Commit::saveObject(const std::vector<unsigned char>& fullObject) {
    std::string_view content_view(
        reinterpret_cast<const char*>(fullObject.data()), 
        fullObject.size()
    );

    std::string hexHash = calculateHash(content_view);
    std::string compressed = compress(content_view);

    if (compressed == " ") return "";

    std::string dir = hexHash.substr(0, 2);
    std::string filename = hexHash.substr(2);

    std::string objectPath = path + std::string("objects/") + dir;
    mkdir(objectPath.c_str());

    std::string filenamePath = objectPath + "/" + filename;
    FILE* newFile = fopen(filenamePath.c_str(), "wb");
    if (newFile) {
        fwrite(compressed.data(), 1, compressed.size(), newFile);
        fclose(newFile);
    }

    return hexHash;
}
std::string change::Commit::makeTree(){
    std::string indexPath = path + std::string("index");
    FILE* index = fopen(indexPath.c_str(), "rb");
    if(!index) return "";

    char header[12];
    if (fread(header, 1, 12, index) != 12) {
        fclose(index);
        return "";
    }
    uint32_t entryCount = 0;
    memcpy(&entryCount, header + 8, sizeof(uint32_t));

    entryCount = __builtin_bswap32(entryCount);
    std::vector<unsigned char> treeContent;

    for (uint32_t i = 0; i < entryCount; ++i) {
        long entryStart = ftell(index);

        fseek(index, entryStart + 40, SEEK_SET);
        unsigned char sha1[20];
        fread(sha1, 1, 20, index);

        fseek(index, entryStart + 62, SEEK_SET);
        std::string filename;
        char c;
        while ((c = fgetc(index)) != EOF && c != '\0') {
            filename.push_back(c);
        }

        size_t entrySize = 62 + filename.size() + 1;
        size_t paddedSize = (entrySize + 7) & ~7;
        fseek(index, entryStart + paddedSize, SEEK_SET);

        std::string modeStr = "100644";
        treeContent.insert(treeContent.end(), modeStr.begin(), modeStr.end());
        treeContent.push_back(' ');
        treeContent.insert(treeContent.end(), filename.begin(), filename.end());
        treeContent.push_back('\0');
        treeContent.insert(treeContent.end(), sha1, sha1 + 20);
    }

    fclose(index);

    std::string headerTree = "tree " + std::to_string(treeContent.size()) + '\0';
    std::vector<unsigned char> fullObject;
    fullObject.insert(fullObject.end(), headerTree.begin(), headerTree.end());
    fullObject.insert(fullObject.end(), treeContent.begin(), treeContent.end());

    return saveObject(fullObject);
}
std::string change::Commit::commit(std::string_view message){
    std::string treeHash = makeTree();
    if (treeHash.empty()) return "";

    std::string authorName = userData.authorName;
    std::string authorEmail = userData.authorEmail;

    std::time_t now = std::time(nullptr);
    std::string timestamp = std::to_string(now) + " -0300";

    std::string parentHash = "";
    std::string refPath = path + std::string("refs/heads/") + userData.branch;

    FILE* refFile = fopen(refPath.c_str(), "r");
    if (refFile) {
        char buf[41];
        if (fgets(buf, 41, refFile)) {
            parentHash = std::string(buf);
            parentHash.erase(parentHash.find_last_not_of(" \n\r\t") + 1);
        }
        fclose(refFile);
    }

    std::string commitContent = "";
    commitContent += "tree " + treeHash + "\n";
    if (!parentHash.empty() && parentHash.size() == 40) {
        commitContent += "parent " + parentHash + "\n";
    }
    commitContent += "author " + authorName + " <" + authorEmail + "> " + timestamp + "\n";
    commitContent += "committer " + authorName + " <" + authorEmail + "> " + timestamp + "\n\n";
    commitContent += std::string(message) + "\n";

    std::string headerCommit = "commit " + std::to_string(commitContent.size()) + '\0';
    std::vector<unsigned char> fullCommitObject;
    fullCommitObject.insert(fullCommitObject.end(), headerCommit.begin(), headerCommit.end());
    fullCommitObject.insert(fullCommitObject.end(), commitContent.begin(), commitContent.end());

    std::string commitHash = saveObject(fullCommitObject);

    FILE* outRef = fopen(refPath.c_str(), "w");
    if (outRef) {
        fwrite(commitHash.data(), 1, commitHash.size(), outRef);
        fputc('\n', outRef);
        fclose(outRef);
    }

    return commitHash;
}
