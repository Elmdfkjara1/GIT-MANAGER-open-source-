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

bool env::Detector::check(){
	if(!checkGit()){
		return false;
	}
	return true;
}
