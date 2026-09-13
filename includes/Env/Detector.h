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
#pragma once

#include "Data.h"
#include <string_view>
#include <vector>

namespace git::env {

	class Detector final {
		std::string_view currentPath;
		Data user;
	public:
		Detector(std::string_view path) : currentPath(path) {}
	
		bool check();
		bool checkGit();
		void checkBranch();
		bool checkGitVar();

		std::vector<std::string> scanBranches();
		
		const Data& getUser() const { return user; }
		Data& getUser() { return user; }
	};

}//namespace gitManager