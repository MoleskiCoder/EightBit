#include "stdafx.h"
#include "FuseTestEvents.h"

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
