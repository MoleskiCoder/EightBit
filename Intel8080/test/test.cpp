#include "stdafx.h"
#include "TestHarness.h"
#include "Board.h"
#include "Configuration.h"

int main(int, char*[]) {

	Configuration configuration;

#ifdef _DEBUG
	configuration.setDebugMode(true);
	configuration.setProfileMode(true);
#endif

	EightBit::TestHarness<Configuration, Board> harness(configuration);
	harness.initialise();
	harness.runLoop();

	return 0;
}