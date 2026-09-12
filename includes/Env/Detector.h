#pragma once

#include "Data.h"
#include "Basic.h"
#include <string_view>

namespace git::env {

	class Detector final {
		std::string_view currentPath;
		Data user;
		bool checkGit();
		void checkBranch();
	public:
		Detector(std::string_view path) : currentPath(path) {}
	
		envState check();
		const Data& getUser() const { return user; }
		Data& getUser() { return user; }
	};

}//namespace gitManager