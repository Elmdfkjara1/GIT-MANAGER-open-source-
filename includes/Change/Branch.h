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