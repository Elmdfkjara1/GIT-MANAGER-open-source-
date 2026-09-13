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
