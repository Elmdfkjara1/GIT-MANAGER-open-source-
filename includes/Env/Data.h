#pragma once

#include <string>

namespace git {
	struct Data{
		std::string branch;
	};
	std::string calculateHash(std::string_view content);
	std::string compress(std::string_view content);
	void getRawHash(std::string_view content, unsigned char* outBuffer);
}//namespace git