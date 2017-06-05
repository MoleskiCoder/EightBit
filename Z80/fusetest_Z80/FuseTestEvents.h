#pragma once

#include <vector>
#include "FuseTestEvent.h"

namespace Fuse {
	class TestEvents {
	public:
		std::vector<TestEvent> events;

		void read(std::ifstream& file);
	};
}