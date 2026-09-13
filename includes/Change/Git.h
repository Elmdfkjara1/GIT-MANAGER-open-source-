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
	};
}//namespace git::change