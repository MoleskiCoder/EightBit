#include "stdafx.h"
#include "Configuration.h"
#include "Board.h"

#include <TestHarness.h>

int main(int, char*[]) {

	Configuration configuration;

#ifdef _DEBUG
	configuration.setDebugMode(true);
	configuration.setProfileMode(true);
#endif

	EightBit::TestHarness<Configuration, Board> harness(configuration);
	harness.run();

	return 0;
}