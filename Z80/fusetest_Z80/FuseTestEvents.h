#pragma once

#include <vector>
#include "FuseTestEvent.h"

namespace Fuse {
	class TestEvents {
	public:
		std::vector<TestEvent> events;

		bool operator==(const TestEvents& rhs) const;

		void read(std::ifstream& file);

		void dump() const;
	};
}