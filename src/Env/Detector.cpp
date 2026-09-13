#include "Env/Detector.h"
#include <filesystem>
#include <string>
#include <string_view>

using namespace git;

namespace fs = std::filesystem;

bool env::Detector::checkGit(){
	fs::path git_dir(currentPath);
	git_dir /= ".git";

	if(!fs::exists(git_dir) || !fs::is_directory(git_dir))
		return false;

	checkBranch();

	return true;
}
bool env::Detector::checkGitVar(){
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

    fs::path globalConfigPath = homeDir / ".gitconfig";
    if (!fs::exists(globalConfigPath)) return false;

    FILE* file = fopen(globalConfigPath.string().c_str(), "r");
    if (!file) return false;

    char _buffer[256];
    bool inUserSection = false;

    while (fgets(_buffer, sizeof(_buffer), file) != nullptr) {
        std::string line(_buffer);
        
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, (last - first + 1));

        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        if (line.front() == '[' && line.back() == ']') {
            std::string section = line.substr(1, line.size() - 2);
            section.erase(0, section.find_first_not_of(" \t"));
            section.erase(section.find_last_not_of(" \t") + 1);
            inUserSection = (section == "user");
            continue;
        }

        if (inUserSection) {
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);
                
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                
                value.erase(0, value.find_first_not_of(" \t\"'"));
                value.erase(value.find_last_not_of(" \t\r\n\"'") + 1);

                if (key == "name") {
                    user.authorName = value;
                } else if (key == "email") {
                    user.authorEmail = value;
                }
            }
        }
    }

    fclose(file);

    return true;
}
void env::Detector::checkBranch(){
	fs::path head_path(currentPath);
	head_path /= ".git";
	head_path /= "HEAD";

	FILE* head = fopen(head_path.string().c_str(), "r");
	if(!head){
		user.branch = "";
		return;
	}

	char _buffer[128];

	fgets(_buffer, sizeof(_buffer), head);
	
	std::string_view buffer(_buffer);
	const char* index = buffer.data();

	uint8_t count = 0;
	for(uint16_t i = 0; i < buffer.size(); ++i){
		if(index[i] == '/'){
			count++;
			if(count == 2){
				std::string raw_branch(buffer.substr(i + 1));
				raw_branch.erase(raw_branch.find_last_not_of("\n\r") + 1);
                user.branch = raw_branch;				
				break;
			}
		}
	}
	if(count == 0) user.branch = "";

	fclose(head);
}
std::vector<std::string> env::Detector::scanBranches() {
    std::vector<std::string> branches;
    fs::path heads_path(currentPath);
    heads_path /= ".git";
    heads_path /= "refs";
    heads_path /= "heads";

    if (!fs::exists(heads_path) || !fs::is_directory(heads_path)) {
        return branches;
    }

    for (const auto& entry : fs::recursive_directory_iterator(heads_path)) {
        if (fs::is_regular_file(entry.path())) {
            fs::path relative = fs::relative(entry.path(), heads_path);
            branches.push_back(relative.string());
        }
    }

    return branches;
}

bool env::Detector::check(){
	if(!checkGit()){
		return false;
	}
	return true;
}
