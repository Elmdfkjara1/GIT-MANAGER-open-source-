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