#pragma once

#include <stdint.h>

namespace git {
	enum envState : uint8_t {
		unknown = 0,
		all_right,
		without_git_file
	};
}