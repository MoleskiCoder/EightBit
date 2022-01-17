#include "stdafx.h"
#include "FuseTestEvents.h"

bool Fuse::TestEvents::operator==(const TestEvents& rhs) const {
	auto unequal = events.size() != rhs.events.size();
	for (int i = 0; !unequal && (i < events.size()); ++i) {
		const auto equal = events[i] == rhs.events[i];
		unequal = !equal;
	}
	return !unequal;
}

void Fuse::TestEvents::read(std::ifstream& file) {
	bool complete = false;
	do {
		TestEvent event;
		event.read(file);
		complete = !event.valid;
		if (!complete)
			events.push_back(event);
	}  while (!complete);
}

void Fuse::TestEvents::dump() const {
	for (const auto& event : events)
		event.dump();
}
