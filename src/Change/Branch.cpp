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
