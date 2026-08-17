#include "stdafx.h"
#include "../inc/SDLWrapper.h"

using namespace Gaming;

SDLWrapper::SDLWrapper(bool verbose /* = false */) {
	verifySDLCall(::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC), "Failed to initialise SDL: ");
	const auto priority = verbose ? SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE : SDL_LogPriority::SDL_LOG_PRIORITY_ERROR;
	SDL_LogSetAllPriority(priority);
}

SDLWrapper::~SDLWrapper() {
	::SDL_Quit();
}