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
#include "Change/Git.h"
#include <stdio.h>
#include <filesystem> 
#include <vector>

namespace fs = std::filesystem;

#ifdef WIN32
	#include <direct.h>
	#define mkdir(path, mode) _mkdir(path)
#else
	#include <sys/stat.h>
#endif

using namespace git;

namespace {
	const char infoBuffer[] = "# git ls-files --others --exclude-from=.git/info/exclude\n\
# Lines that start with '#' are comments.\n\
# For a project mostly in C, the following would be a good set of\n\
# exclude patterns (uncomment them if you want to use them):\n\
# *.[oa]\n\
# *~\n";

	const char configBuffer[] = "[core]\n\
\trepositoryformatversion = 0\n\
\tfilemode = true\n\
\tbare = false\n\
\tlogallrefupdates = true\n";
}


void change::Git::makeInfo(){
    mkdir((gitPath + "/info/").c_str(), 0777);

    FILE* Finfo = fopen((gitPath + "/info/exclude").c_str(), "w");
    if(!Finfo) return;

    fprintf(Finfo, "%s", infoBuffer);

    fclose(Finfo);
}

void change::Git::makeobjects(){
    mkdir((gitPath + "/objects/").c_str(), 0777);
    mkdir((gitPath + "/objects/info/").c_str(), 0777);
    mkdir((gitPath + "/objects/pack/").c_str(), 0777);
}

void change::Git::makeRefs(){
    mkdir((gitPath + "/refs/").c_str(), 0777);
    mkdir((gitPath + "/refs/heads/").c_str(), 0777);
    auto* _ = fopen((gitPath + "/refs/heads/master").c_str(), "w");
    fclose(_);
    mkdir((gitPath + "/refs/tags/").c_str(), 0777);
} 

void change::Git::makeConfig(){
    FILE* Fconfig = fopen((gitPath + "/config").c_str(), "w");
    if(!Fconfig) return;

    fprintf(Fconfig, "%s", configBuffer);

    fclose(Fconfig);
}

void change::Git::makeDescription(){
    FILE* Fdescription = fopen((gitPath + "/description").c_str(), "w");
    if(!Fdescription) return;

    fprintf(Fdescription, "%s\n", "Unnamed repository; edit this file 'description' to name the repository.");

    fclose(Fdescription);
}

void change::Git::makeHEAD(){
    FILE* Fhead = fopen((gitPath + "/HEAD").c_str(), "w");
    if(!Fhead) return;

    fprintf(Fhead, "%s\n", "ref: refs/heads/master");

    fclose(Fhead);
}

void change::Git::makeGit(){
	mkdir(gitPath.c_str(), 0777);

	makeInfo();
	makeobjects();
	makeRefs(); 
	makeConfig();
	makeDescription();
	makeHEAD();
}
void change::Git::makeGitConfig(std::string_view name, std::string_view email) {
    fs::path homeDir;
    #ifdef WIN32
        if (const char* userProfile = std::getenv("USERPROFILE")) {
            homeDir = userProfile;
        }
    #else
        if (const char* home = std::getenv("HOME")) {
            homeDir = home;
        }
    #endif

    if (homeDir.empty()) return;

    fs::path globalConfigPath = homeDir / ".gitconfig";
    std::vector<std::string> lines;
    bool userSectionFound = false;
    bool nameUpdated = false;
    bool emailUpdated = false;

    if (fs::exists(globalConfigPath)) {
        FILE* file = fopen(globalConfigPath.string().c_str(), "r");
        if (file) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), file) != nullptr) {
                lines.push_back(buffer);
            }
            fclose(file);
        }
    }

    bool inUserSection = false;
    for (auto& line : lines) {
        std::string trimmed = line;
        size_t first = trimmed.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        size_t last = trimmed.find_last_not_of(" \t\r\n");
        trimmed = trimmed.substr(first, (last - first + 1));

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            std::string section = trimmed.substr(1, trimmed.size() - 2);
            inUserSection = (section == "user");
            if (inUserSection) userSectionFound = true;
            continue;
        }

        if (inUserSection) {
            size_t eqPos = trimmed.find('=');
            if (eqPos != std::string::npos) {
                std::string key = trimmed.substr(0, eqPos);
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);

                if (key == "name") {
                    line = "\tname = " + std::string(name) + "\n";
                    nameUpdated = true;
                } else if (key == "email") {
                    line = "\temail = " + std::string(email) + "\n";
                    emailUpdated = true;
                }
            }
        }
    }

    if (userSectionFound) {
        auto it = lines.begin();
        while (it != lines.end()) {
            if (it->find("[user]") != std::string::npos) {
                ++it;
                if (!nameUpdated) {
                    it = lines.insert(it, "\tname = " + std::string(name) + "\n");
                    ++it;
                }
                if (!emailUpdated) {
                    lines.insert(it, "\temail = " + std::string(email) + "\n");
                }
                break;
            }
            ++it;
        }
    } else {
        if (!lines.empty() && (lines.back().empty() || lines.back().back() != '\n')) {
            lines.back() += "\n";
        }
        lines.push_back("\n[user]\n");
        lines.push_back("\temail = " + std::string(email) + "\n");
        lines.push_back("\tname = " + std::string(name) + "\n");
    }

    FILE* outFile = fopen(globalConfigPath.string().c_str(), "w");
    if (outFile) {
        for (const auto& line : lines) {
            fputs(line.c_str(), outFile);
        }
        fclose(outFile);
    }
}