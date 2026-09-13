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

#include <string_view>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <vector>

namespace git::change{
	class Add {
		std::string path;
	
		void updateIndex(std::string_view name, const unsigned char* rawHash, size_t fileSize, const struct stat& st);
	public:
		Add(std::string_view _path){
			path = std::string(_path) + std::string("/.git/");
		}

		void addFile(std::string_view name);
		void removeFile(std::string_view name);
		std::vector<std::string> getStagedFiles();
	};
}