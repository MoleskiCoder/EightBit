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
			m_gameController = ::SDL_GameControllerOpen(m_index);
			if (m_gameController == nullptr) {
				Game::throwSDLException("Unable to open game controller: ");
			}
			openHapticController();
			auto name = ::SDL_GameControllerName(m_gameController);
			::SDL_Log("Game controller name: %s", name);
		}
		else {
			::SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Joystick is not a game controller!!");
		}
	}

	void GameController::openHapticController() {
		m_hapticController = ::SDL_HapticOpen(m_index);
		if (m_hapticController == nullptr) {
			Game::throwSDLException("Unable to open haptic controller: ");
		}
		Game::verifySDLCall(::SDL_HapticRumbleInit(m_hapticController), "Unable to initialise haptic controller: ");
		m_hapticRumbleSupported = ::SDL_HapticRumbleSupported(m_hapticController) != SDL_FALSE;
	}

	void GameController::closeHapticController() noexcept {
		if (m_hapticController != nullptr) {
			::SDL_HapticClose(m_hapticController);
			m_hapticController = nullptr;
		}
		m_hapticRumbleSupported = false;
	}

	void GameController::close() noexcept {
		if (m_gameController != nullptr) {
			::SDL_GameControllerClose(m_gameController);
			m_gameController = nullptr;
		}
		closeHapticController();
	}

	void GameController::startRumble() noexcept {
		if (m_hapticRumbleSupported) {
			if (::SDL_HapticRumblePlay(m_hapticController, 1.0, 1000) < 0) {
				::SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to start haptic rumble: %s", ::SDL_GetError());
			}
		}
	}

	void GameController::stopRumble() noexcept {
		if (m_hapticRumbleSupported) {
			if (::SDL_HapticRumbleStop(m_hapticController) < 0) {
				::SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to stop haptic rumble: %s", ::SDL_GetError());
			}
		}
	}

}