#include "stdafx.h"
#include "Configuration.h"
#include "Board.h"

#include <TestHarness.h>

#include <iostream>

int main(int argc, char* argv[]) {

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