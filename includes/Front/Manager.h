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

#include "Env/Detector.h"

#include "Change/Branch.h"
#include "Change/Git.h"
#include "Change/Add.h"
#include "Change/Commit.h"

namespace git::Front{
	class Manager final {
		std::string literalPath;
		env::Detector dec;
		change::Branch branch;
		change::Git gitMk;
		change::Add add;
		change::Commit commit;

		void wait(std::string_view message);

		void driveStatus();
		void driveCommit();
		void driveBranch();
		void driveConfig();
		int checkFiles();
	public:
		Manager();

		int drive();
	};
}