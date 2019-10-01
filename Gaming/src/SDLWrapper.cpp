#include "pch.h"
#include "SDLWrapper.h"

using namespace Gaming;

SDLWrapper::SDLWrapper() {
	verifySDLCall(::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC), "Failed to initialise SDL: ");
}

SDLWrapper::~SDLWrapper() {
	::SDL_Quit();
}