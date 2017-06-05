#include "stdafx.h"
#include "Game.h"
#include "Configuration.h"

int main(int, char*[]) {

	Configuration configuration;

#ifdef _DEBUG
	configuration.setDebugMode(true);
	configuration.setProfileMode(true);
#endif

	Game game(configuration);
	game.initialise();
	game.runLoop();

	return 0;
}