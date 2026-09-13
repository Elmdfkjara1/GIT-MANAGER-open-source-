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

#include <string>

namespace git::change{
	class Git final {
		std::string gitPath;

		//void makeHooks(); //this need support
		void makeInfo();
		void makeobjects();
		void makeRefs(); 

		void makeConfig();
		void makeDescription();
		void makeHEAD();
	public:
		Git(const std::string& path){
			gitPath = path + "/.git/";
		}

		void makeGit();
		void makeGitConfig(std::string_view name, std::string_view email);
	};
}//namespace git::change