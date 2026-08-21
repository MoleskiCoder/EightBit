#include "stdafx.h"
#include "../inc/Wrapper.h"

using namespace Gaming;

Wrapper::Wrapper(bool verbose /* = false */) {
	const auto success = ::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
	maybeThrowException(success, "Unable to initialise SDL library");
	const auto priority = verbose ? SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE : SDL_LogPriority::SDL_LOG_PRIORITY_ERROR;
	::SDL_SetLogPriorities(priority);
}

Wrapper::~Wrapper() {
	::SDL_Quit();
}