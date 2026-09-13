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