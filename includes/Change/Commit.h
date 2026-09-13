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
#include <vector>

namespace git {
	struct Data;

	namespace change {
		class Commit final {
			std::string path;
			const Data& userData;

			std::string makeTree();
			std::string saveObject(const std::vector<unsigned char>& fullObject);
		public:
			Commit(const std::string& p, const Data& userData)
			: userData(userData) {
				path = p + std::string("/.git/");
			}

			std::string commit(std::string_view message);
		};
	}//namespace change
}//namespace git
