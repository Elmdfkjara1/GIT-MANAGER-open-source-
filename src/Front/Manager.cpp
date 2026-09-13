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
#include "Front/Manager.h"

#include <stdio.h>
#include <filesystem>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

using namespace git;

enum class NavKey { Up, Down, Space, Enter, Quit, New, None };

namespace {
#ifdef _WIN32
    struct RawModeGuard { RawModeGuard() {} ~RawModeGuard() {} };

    NavKey readNavKey() {
        int ch = _getch();
        if (ch == 0 || ch == 0xE0) {
            int arrow = _getch();
            if (arrow == 72) return NavKey::Up;
            if (arrow == 80) return NavKey::Down;
            return NavKey::None;
        }
        if (ch == ' ') return NavKey::Space;
        if (ch == '\r' || ch == '\n') return NavKey::Enter;
        if (ch == 'q' || ch == 'Q') return NavKey::Quit;
        if (ch == 'n' || ch == 'N') return NavKey::New;
        return NavKey::None;
    }
#else
    struct RawModeGuard {
        termios oldt;
        RawModeGuard() {
            tcgetattr(STDIN_FILENO, &oldt);
            termios newt = oldt;
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        }
        ~RawModeGuard() { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); }
    };

    NavKey readNavKey() {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) return NavKey::Quit;
        if (c == '\033') {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) return NavKey::None;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) return NavKey::None;
            if (seq[0] == '[') {
                if (seq[1] == 'A') return NavKey::Up;
                if (seq[1] == 'B') return NavKey::Down;
            }
            return NavKey::None;
        }
        if (c == ' ') return NavKey::Space;
        if (c == '\n' || c == '\r') return NavKey::Enter;
        if (c == 'q' || c == 'Q') return NavKey::Quit;
        if (c == 'n' || c == 'N') return NavKey::New;
        return NavKey::None;
    }
#endif
}


Front::Manager::Manager()
: literalPath(fs::current_path().string()),
  dec(literalPath),
  branch(literalPath, dec.getUser()),
  gitMk(literalPath),
  add(literalPath),
  commit(literalPath, dec.getUser()) {}

void Front::Manager::wait(std::string_view message) {
    std::vector<std::string> spinner = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

    for (int i = 0; i < 15; ++i) {
        std::string frame = spinner[i % spinner.size()];

        printf("\r\033[95m%s\033[0m %s", frame.c_str(), message.data());
        fflush(stdout);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
    
    printf("\033[2J\033[H");
    fflush(stdout);
}

int Front::Manager::drive(){
    wait("\033[97mchecking files...\033[0m");
    
    if (dec.checkGit()) {
        dec.checkBranch();
        if(!dec.checkGitVar()){
            driveConfig();
            dec.checkGitVar();
        }
        return checkFiles();
    }

    printf("\033[96m::\033[0m \033[97mcannot find a git directory\033[0m\n\033[93mdo you want to create it?\033[0m (y/n): ");
    char response[10];
    
    if (!fgets(response, sizeof(response), stdin) || response[0] != 'y') {
        return -1;
    }

    printf("\033[2J\033[H");
    wait("\033[97mCreating git directory\033[0m");
    gitMk.makeGit();

    dec.checkBranch();
    return checkFiles();
}
void Front::Manager::driveConfig() {
    printf("\033[2J\033[H");
    printf("\033[93m::\033[0m \033[97mNo global Git user configuration (name/email) found.\033[0m\n\n");

    char nameBuffer[128];
    char emailBuffer[128];

    printf("\033[96mEnter your name:\033[0m ");
    if (fgets(nameBuffer, sizeof(nameBuffer), stdin)) {
        nameBuffer[strcspn(nameBuffer, "\r\n")] = '\0';
    }

    printf("\033[96mEnter your email (e.g. your_email@gmail.com):\033[0m ");
    if (fgets(emailBuffer, sizeof(emailBuffer), stdin)) {
        emailBuffer[strcspn(emailBuffer, "\r\n")] = '\0';
    }

    if (strlen(nameBuffer) > 0 && strlen(emailBuffer) > 0) {
        gitMk.makeGitConfig(nameBuffer, emailBuffer);
        wait("\033[95mSaving global configuration...\033[0m");
        printf("\033[92mGit configuration created successfully!\033[0m\n");
    } else {
        printf("\033[91mEmpty data. Automatic configuration skipped.\033[0m\n");
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int Front::Manager::checkFiles(){
    char response[8];
    do{
        printf("\033[2J\033[H");
        printf("\033[95ma\033[0m: add files to stage\n\033[95mc\033[0m: commit the files\n\033[95mb\033[0m: change to other branch\n\033[95me\033[0m: exit\n");
        
        const Data& currentData = dec.getUser();
        printf("\n\033[90m┌──[\033[93m%s\033[90m] ─── \033[96m%s\033[0m\n", 
            currentData.authorName.c_str(), 
            literalPath.c_str());
        printf("\033[90m└──\033[92m[%s]\033[0m \033[95m❯\033[0m ", 
            currentData.branch.c_str());

        if (!fgets(response, sizeof(response), stdin)) break;
        
        response[strcspn(response, "\r\n")] = 0;

        switch(response[0]){
            case 'a':
                driveStatus();
                break;
            case 'c':
                driveCommit();
                break;
            case 'b':
                driveBranch();
                break;
            case 'e':
                printf("\033[91mExiting...\033[0m\nThank you for use GitManager \033[95m:3\033[0m\n");
                break;
            default:
                printf("\033[93mUnknown option, try again.\033[0m\n");
                break;
        }

    } while(response[0] != 'e');

    return 0;
}

struct FileStatus {
    std::string name;
    std::string state;
    bool selected;
};

void Front::Manager::driveStatus(){
    RawModeGuard rawMode;

    fs::path rootPath = literalPath;

    std::vector<std::string> stagedList = add.getStagedFiles();
    
    std::vector<FileStatus> files;

    for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
        if (!entry.is_regular_file()) continue;
        
        fs::path relPath = fs::relative(entry.path(), rootPath);
        std::string relStr = relPath.string();

        if (relStr.rfind(".git", 0) == 0) continue;

        bool isStaged = false;
        for (const auto& staged : stagedList) {
            if (staged == relStr) {
                isStaged = true;
                break;
            }
        }

        std::string stateStr = isStaged ? "\033[92m(staged)\033[0m" : "\033[93m(new)\033[0m";
        files.push_back({relStr, stateStr, false});
    }

    if (files.empty()) {
        printf("\033[2J\033[H\033[91mNo files found in workspace.\033[0m\n");
        return;
    }

    size_t selectedIndex = 0;

    auto render = [&]() {
        printf("\033[2J\033[H");
        printf("\033[97mUse Up/Down to navigate, \033[95m[Space]\033[0m to select, \033[95m[Enter]\033[0m to stage, \033[91m'q'\033[0m to quit\033[0m\n\n");
        printf("\033[96mFiles:\033[0m\n");

        for (size_t i = 0; i < files.size(); ++i) {
            std::string cursor = (i == selectedIndex) ? "\033[95m>\033[0m" : " ";
            std::string checkbox = files[i].selected ? "\033[92m[x]\033[0m" : "[ ]";
            
            printf(" %s %s %s  %s\n", 
                cursor.c_str(), 
                checkbox.c_str(), 
                files[i].name.c_str(), 
                files[i].state.c_str());
        }
    };

    render();

    bool enterPressed = false;
    while (true) {
        NavKey key = readNavKey();
        if (key == NavKey::Quit) break;
        if (key == NavKey::Up) {
            if (selectedIndex == 0) selectedIndex = files.size() - 1;
            else selectedIndex--;
        } else if (key == NavKey::Down) {
            selectedIndex = (selectedIndex + 1) % files.size();
        } else if (key == NavKey::Space) {
            files[selectedIndex].selected = !files[selectedIndex].selected;
        } else if (key == NavKey::Enter) {
            enterPressed = true;
            break;
        }
        render();
    }

    if (enterPressed) {
        bool stagedAny = false;
        for (const auto& file : files) {
            if (file.selected) {
                add.addFile(file.name);
                stagedAny = true;
            }
        }
        if (stagedAny) {
            wait("\033[95mStaging selected files...\033[0m");
            printf("\033[92m✔ Files added to index successfully!\033[0m\n");
        }
    }
}

void Front::Manager::driveCommit(){
    printf("\033[2J\033[H");
    wait("\033[97mchecking files...\033[0m");

    printf("\033[95mu\033[0m: feat\n\033[95mf\033[0m: fix\n\033[95mr\033[0m: refactor\n\033[95md\033[0m: debug\n\033[95mo\033[0m: docs\n\033[95mt\033[0m: test\ndefault (update)\n\033[96mChoice:\033[0m ");
    char response[8];

    if (!fgets(response, sizeof(response), stdin)) return;
    response[strcspn(response, "\r\n")] = '\0';

    std::string prefix = "update: ";

    switch(response[0]){
        case 'u': prefix = "feat: "; break;
        case 'f': prefix = "fix: "; break;
        case 'r': prefix = "refactor: "; break;
        case 'd': prefix = "debug: "; break;
        case 'o': prefix = "docs: "; break;
        case 't': prefix = "test: "; break;
        default: break;
    }

    printf("\033[2J\033[H");
    printf("\033[96m(Write message):\033[0m ");
    
    char msgBuffer[256];
    if (!fgets(msgBuffer, sizeof(msgBuffer), stdin) || msgBuffer[0] == '\n' || msgBuffer[0] == '\r') {
        snprintf(msgBuffer, sizeof(msgBuffer), "update files");
    } else {
        msgBuffer[strcspn(msgBuffer, "\r\n")] = '\0';
    }

    std::string finalMessage = prefix + msgBuffer;

    wait("\033[95mCreating commit...\033[0m");
    commit.commit(finalMessage);

    printf("\033[92m✔ Commit made successfully!\033[0m\n");
}

void Front::Manager::driveBranch(){
    RawModeGuard rawMode;

    std::vector<std::string> branches = dec.scanBranches();
    const std::string& currentBranch = dec.getUser().branch;
    
    size_t totalOptions = branches.size() + 1; 
    size_t selectedIndex = 0;

    auto render = [&]() {
        printf("\033[2J\033[H");
        printf("\033[97mUse Up/Down arrows to switch branch, \033[95m'n'\033[0m for new branch, \033[91m'q'\033[0m to exit\033[0m\n\n");
        printf("\033[96mBranches:\033[0m\n");

        for (size_t i = 0; i < branches.size(); ++i) {
            if (i == selectedIndex) {
                printf("  \033[95m> %s\033[0m", branches[i].c_str());
            } else if (branches[i] == currentBranch) {
                printf("    \033[92m* %s\033[0m", branches[i].c_str());
            } else {
                printf("      %s", branches[i].c_str());
            }
            printf("\n");
        }

        if (selectedIndex == branches.size()) {
            printf("  \033[95m> [+] Create new branch\033[0m\n");
        } else {
            printf("      [+] Create new branch\n");
        }
    };

    render();

    bool enterPressed = false;
    while (true) {
        NavKey key = readNavKey();
        if (key == NavKey::Quit) break;

        if (key == NavKey::Up) {
            selectedIndex = (selectedIndex - 1 + totalOptions) % totalOptions;
        } else if (key == NavKey::Down) {
            selectedIndex = (selectedIndex + 1) % totalOptions;
        } else if (key == NavKey::New) {
            printf("\033[2J\033[H");
            printf("\033[96mEnter new branch name:\033[0m ");
            char nameBuffer[128];
            if (fgets(nameBuffer, sizeof(nameBuffer), stdin)) {
                nameBuffer[strcspn(nameBuffer, "\r\n")] = '\0';
                if (nameBuffer[0] != '\0') {
                    branch.createNewBranch(std::string(nameBuffer));
                    wait("\033[95mCreating branch...\033[0m");
                }
            }
            dec.checkBranch();
            return;
        } else if (key == NavKey::Enter) {
            enterPressed = true;
            break;
        }
        render();
    }

    if (enterPressed) {
        if (selectedIndex == branches.size()) {
            printf("\033[2J\033[H");
            printf("\033[96mEnter new branch name:\033[0m ");
            char nameBuffer[128];
            if (fgets(nameBuffer, sizeof(nameBuffer), stdin)) {
                nameBuffer[strcspn(nameBuffer, "\r\n")] = '\0';
                if (nameBuffer[0] != '\0') {
                    branch.createNewBranch(std::string(nameBuffer));
                    wait("\033[95mCreating branch...\033[0m");
                }
            }
        } else {
            std::string targetBranch = branches[selectedIndex];
            if (targetBranch != currentBranch) {
                branch.changeBranch(targetBranch);
                wait("\033[95mSwitching branch...\033[0m");
            }
        }
    }
    dec.checkBranch();
}