/*
* gitManager 
* Copyright (C) <2026>  <Isra222>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "Change/Add.h"
#include "Env/Data.h"
#include <cstring>
#include <openssl/sha.h>

#ifdef WIN32
	#include <direct.h>
	#include <winsock2.h>
	#define mkdir(path) _mkdir(path)
#else
	#include <sys/stat.h>
	#include <arpa/inet.h>
	#define mkdir(path) mkdir(path, 0777)
#endif


using namespace git;

void change::Add::updateIndex(std::string_view name, const unsigned char* rawHash, size_t fileSize, const struct stat& st) {
    std::string index;
    uint32_t currentCount = 1;
    std::string indexPath = std::string(path) + "/index";

    FILE* existingFile = fopen(indexPath.c_str(), "rb");
    if (existingFile) {
        fseek(existingFile, 0, SEEK_END);
        long size = ftell(existingFile);
        rewind(existingFile);

        if (size > 20) {
            index.resize(size - 20);
            fread(index.data(), 1, size - 20, existingFile);
            
            uint32_t diskCount;
            memcpy(&diskCount, index.data() + 8, 4);
            currentCount = ntohl(diskCount) + 1;
            
            uint32_t newCountNet = htonl(currentCount);
            memcpy(index.data() + 8, &newCountNet, 4);
        }
        fclose(existingFile);
    } else {
        index.append("DIRC");
        uint32_t ver = htonl(2);
        uint32_t countNet = htonl(1);
        index.append(reinterpret_cast<char*>(&ver), 4);
        index.append(reinterpret_cast<char*>(&countNet), 4);
    }

    std::string entry;
    
    uint32_t ctime_sec = htonl(static_cast<uint32_t>(st.st_ctime));
    uint32_t ctime_nsec = 0;
    entry.append(reinterpret_cast<char*>(&ctime_sec), 4);
    entry.append(reinterpret_cast<char*>(&ctime_nsec), 4);

    uint32_t mtime_sec = htonl(static_cast<uint32_t>(st.st_mtime));
    uint32_t mtime_nsec = 0;
    entry.append(reinterpret_cast<char*>(&mtime_sec), 4);
    entry.append(reinterpret_cast<char*>(&mtime_nsec), 4);

    uint32_t dev = htonl(static_cast<uint32_t>(st.st_dev));
    uint32_t ino = htonl(static_cast<uint32_t>(st.st_ino));
    entry.append(reinterpret_cast<char*>(&dev), 4);
    entry.append(reinterpret_cast<char*>(&ino), 4);

    uint32_t mode = htonl(0100644); 
    entry.append(reinterpret_cast<char*>(&mode), 4);

    uint32_t uid = htonl(static_cast<uint32_t>(st.st_uid));
    uint32_t gid = htonl(static_cast<uint32_t>(st.st_gid));
    entry.append(reinterpret_cast<char*>(&uid), 4);
    entry.append(reinterpret_cast<char*>(&gid), 4);

    uint32_t netSize = htonl(static_cast<uint32_t>(fileSize));
    entry.append(reinterpret_cast<char*>(&netSize), 4);

    entry.append(reinterpret_cast<const char*>(rawHash), SHA_DIGEST_LENGTH);

    uint16_t flags = htons(static_cast<uint16_t>(name.size() & 0xFFF));
    entry.append(reinterpret_cast<char*>(&flags), 2);

    entry.append(name);
    entry.push_back('\0');

    size_t entrySize = 62 + name.size() + 1;
    while (entrySize % 8 != 0) {
        entry.push_back('\0');
        entrySize++;
    }

    index.append(entry);

    unsigned char indexChecksum[SHA_DIGEST_LENGTH];
    getRawHash(index, indexChecksum);
    index.append(reinterpret_cast<char*>(indexChecksum), SHA_DIGEST_LENGTH);

    FILE* f = fopen(indexPath.c_str(), "wb");
    if (f) {
        fwrite(index.data(), 1, index.size(), f);
        fclose(f);
    }
}


void change::Add::addFile(std::string_view name){
    std::string fullPath = std::string(path) + std::string("../") +std::string(name);
    
    struct stat fileInfo;
    if (stat(fullPath.c_str(), &fileInfo) != 0) return;

    FILE* file = fopen(fullPath.c_str(), "rb");
    if(!file) return;

    fseek(file, 0, SEEK_END);
    long sizeBytes = ftell(file);
    rewind(file);

    std::string header = "blob " + std::to_string(sizeBytes);
    header.push_back('\0');

    std::string content;
    content.resize(sizeBytes);
    fread(content.data(), 1, sizeBytes, file);
    fclose(file);

    std::string finalContent = header + content;
    
    unsigned char rawHash[SHA_DIGEST_LENGTH];
    getRawHash(finalContent, rawHash);

    static constexpr char hex_chars[] = "0123456789abcdef";
    std::string hash;
    hash.resize(40);
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        hash[2 * i]     = hex_chars[(rawHash[i] >> 4) & 0x0F];
        hash[2 * i + 1] = hex_chars[rawHash[i] & 0x0F];
    }

    std::string compress_str = compress(finalContent);  

    std::string dir = hash.substr(0, 2);
    std::string filename = hash.substr(2);

    std::string objectPath = std::string(path) + std::string("/objects/") + dir;
    mkdir(objectPath.c_str());

    std::string filenamePath = objectPath + "/" + filename;
    FILE* newFile = fopen(filenamePath.c_str(), "wb");
    if (newFile) {
        fwrite(compress_str.data(), 1, compress_str.size(), newFile);
        fclose(newFile);
    }

    updateIndex(name, rawHash, sizeBytes, fileInfo);
}

void change::Add::removeFile(std::string_view name) {
    std::string indexPath = path + "/index";
    FILE* file = fopen(indexPath.c_str(), "rb");
    if (!file) return;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size < 28) {
        fclose(file);
        return;
    }

    std::string content;
    content.resize(size);
    fread(content.data(), 1, size, file);
    fclose(file);

    if (content.compare(0, 4, "DIRC") != 0) return;

    uint32_t diskCount;
    memcpy(&diskCount, content.data() + 8, 4);
    uint32_t count = ntohl(diskCount);

    size_t offset = 12;
    std::string newEntries;
    uint32_t newCount = 0;

    for (uint32_t i = 0; i < count && offset < content.size() - 20; ++i) {
        if (offset + 62 > content.size() - 20) break;

        size_t nameStart = offset + 62;
        size_t nullPos = content.find('\0', nameStart);
        if (nullPos == std::string::npos) break;

        std::string entryName = content.substr(nameStart, nullPos - nameStart);

        size_t entrySize = (nullPos + 1 - offset);
        while (entrySize % 8 != 0) {
            entrySize++;
        }

        if (offset + entrySize > content.size() - 20) break;

        std::string currentEntry = content.substr(offset, entrySize);

        if (entryName != name) {
            newEntries.append(currentEntry);
            newCount++;
        }

        offset += entrySize;
    }

    std::string newIndex;
    newIndex.append("DIRC", 4);
    uint32_t verNet = htonl(2);
    uint32_t countNet = htonl(newCount);
    newIndex.append(reinterpret_cast<char*>(&verNet), 4);
    newIndex.append(reinterpret_cast<char*>(&countNet), 4);
    newIndex.append(newEntries);

    unsigned char checksum[SHA_DIGEST_LENGTH];
    getRawHash(newIndex, checksum);
    newIndex.append(reinterpret_cast<char*>(checksum), SHA_DIGEST_LENGTH);

    FILE* f = fopen(indexPath.c_str(), "wb");
    if (f) {
        fwrite(newIndex.data(), 1, newIndex.size(), f);
        fclose(f);
    }
}
std::vector<std::string> change::Add::getStagedFiles() {
    std::vector<std::string> stagedFiles;
    std::string indexPath = path + "/index";
    
    FILE* file = fopen(indexPath.c_str(), "rb");
    if (!file) return stagedFiles;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size < 28) {
        fclose(file);
        return stagedFiles;
    }

    std::string content;
    content.resize(size);
    fread(content.data(), 1, size, file);
    fclose(file);

    if (content.compare(0, 4, "DIRC") != 0) return stagedFiles;

    uint32_t diskCount;
    memcpy(&diskCount, content.data() + 8, 4);
    uint32_t count = ntohl(diskCount);

    size_t offset = 12;
    for (uint32_t i = 0; i < count && offset < content.size() - 20; ++i) {
        if (offset + 62 > content.size() - 20) break;

        size_t nameStart = offset + 62;
        size_t nullPos = content.find('\0', nameStart);
        if (nullPos == std::string::npos) break;

        std::string entryName = content.substr(nameStart, nullPos - nameStart);
        stagedFiles.push_back(entryName);

        size_t entrySize = (nullPos + 1 - offset);
        while (entrySize % 8 != 0) {
            entrySize++;
        }

        offset += entrySize;
    }

    return stagedFiles;
}