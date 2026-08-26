#include "stdafx.h"
#include "../inc/Wrapper.h"

using namespace Gaming;

Wrapper::Wrapper(SDL_LogPriority logging /* = SDL_LOG_PRIORITY_WARN */)
: m_logging(logging) {}

void Wrapper::raisePOWER() noexcept {
	base::raisePOWER();
	initialise();
}

void Wrapper::lowerPOWER() noexcept {
	terminate();
	base::lowerPOWER();
}

void Wrapper::initialise() {
	const auto success = ::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
	maybeThrowException(success, "Unable to initialise SDL library");
	::SDL_SetLogPriorities(m_logging);
	::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL library initialised");
}

void Wrapper::terminate() {
    ::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL library terminating");
    ::SDL_Quit();
}
