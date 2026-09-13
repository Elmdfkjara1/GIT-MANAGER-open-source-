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

namespace git{
	struct Data; 

	namespace change{
		class Branch final {
			Data& user;
			std::string_view currentPath;
			std::string lastCommit;
		public:
			Branch(std::string_view path, Data& u)
			: user(u), currentPath(path) {}

			bool createNewBranch(std::string_view name);
			void updateCurrentCommit();
			bool changeBranch(std::string_view name);
		};
	} // namespace change
} //namespace git