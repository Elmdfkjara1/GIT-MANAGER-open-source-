#pragma once

#include "Data.h"
#include <string_view>

namespace git::env {

	class Detector final {
		std::string_view currentPath;
		Data user;
	public:
		Detector(std::string_view path) : currentPath(path) {}
	
		bool check();
		bool checkGit();
		void checkBranch();
		
		const Data& getUser() const { return user; }
		Data& getUser() { return user; }
	};

}//namespace gitManager