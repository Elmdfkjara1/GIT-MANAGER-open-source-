#include "Change/Branch.h"
#include "Env/Data.h"

#include <filesystem>
#include <stdio.h>

#include <string>

using namespace git;

namespace fs = std::filesystem;

void change::Branch::updateCurrentCommit(){
	std::string path = std::string(currentPath) + "/.git/refs/heads/" + user.branch;
	FILE* branch = fopen(path.c_str(), "r");
	if(!branch){
		lastCommit = " ";
		return;
	}

	char _buffer[128];
	if(fgets(_buffer, sizeof(_buffer), branch) == nullptr){
		lastCommit = " ";
		fclose(branch);
		return;
	}

	lastCommit = _buffer;
	lastCommit.erase(lastCommit.find_last_not_of("\n\r") + 1);

	fclose(branch);
}

bool change::Branch::createNewBranch(std::string_view name){
	updateCurrentCommit();

	std::string path = std::string(currentPath) + "/.git/refs/heads/" + std::string(name);
	FILE* newBranch = fopen(path.c_str(), "w");

	if(!newBranch){
		return false;
	}

	fprintf(newBranch, "%s", lastCommit.c_str());

	fclose(newBranch);
	return true;
}
bool change::Branch::changeBranch(std::string_view name){
	std::string branchObj = std::string(currentPath) + "/.git/refs/heads/" + std::string(name);
	
	if(!fs::exists(branchObj))
		return false;


	std::string currentBranch = std::string(currentPath) + "/.git/HEAD";
	FILE* branch = fopen(currentBranch.c_str(), "w");

	std::string ref_content = std::string("ref: refs/heads/") + std::string(name);
	fprintf(branch, "%s", ref_content.c_str());

	fclose(branch);

	user.branch = name;
	updateCurrentCommit();
	return true;
}
