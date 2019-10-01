#include "pch.h"
#include "GameController.h"
#include "Game.h"

namespace Gaming {

	GameController::GameController(int index)
	: m_index(index) {
		open();
	}

	GameController::~GameController() {
		close();
	}

	void GameController::open() {
		SDL_assert(::SDL_NumJoysticks() > 0);
		if (::SDL_IsGameController(m_index)) {
			m_gameController.reset(::SDL_GameControllerOpen(m_index), ::SDL_GameControllerClose);
			if (m_gameController == nullptr)
				SDLWrapper::throwSDLException("Unable to open game controller: ");
			openHapticController();
			auto name = ::SDL_GameControllerName(m_gameController.get());
			::SDL_Log("Game controller name: %s", name);
		} else {
			::SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Joystick is not a game controller!!");
		}
	}

	void GameController::openHapticController() {
		m_hapticController.reset(::SDL_HapticOpen(m_index), ::SDL_HapticClose);
		if (m_hapticController == nullptr)
			SDLWrapper::throwSDLException("Unable to open haptic controller: ");
		SDLWrapper::verifySDLCall(::SDL_HapticRumbleInit(m_hapticController.get()), "Unable to initialise haptic controller: ");
		m_hapticRumbleSupported = ::SDL_HapticRumbleSupported(m_hapticController.get()) != SDL_FALSE;
	}

	void GameController::closeHapticController() noexcept {
		m_hapticController.reset();
		m_hapticRumbleSupported = false;
	}

	void GameController::close() noexcept {
		m_gameController.reset();
		closeHapticController();
	}

	void GameController::startRumble() noexcept {
		if (m_hapticRumbleSupported) {
			if (::SDL_HapticRumblePlay(m_hapticController.get(), 1.0, 1000) < 0)
				::SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to start haptic rumble: %s", ::SDL_GetError());
		}
	}

	void GameController::stopRumble() noexcept {
		if (m_hapticRumbleSupported) {
			if (::SDL_HapticRumbleStop(m_hapticController.get()) < 0)
				::SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to stop haptic rumble: %s", ::SDL_GetError());
		}
	}
}