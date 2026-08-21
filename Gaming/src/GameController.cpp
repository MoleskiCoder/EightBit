#include "stdafx.h"
#include "../inc/GameController.h"
#include "../inc/Game.h"

namespace Gaming {

	GameController::GameController(int index)
	: m_index(index) {
		open();
	}

	GameController::~GameController() {
		close();
	}

	void GameController::open() {

		int count;
		auto* joysticks = SDL_GetJoysticks(&count);
		Wrapper::maybeThrowException(joysticks, "Unable to obtain joystick information");
		::SDL_free(joysticks);
		assert(count > 0);
		assert(m_index < count);
	
		if (::SDL_IsGamepad(m_index)) {
			m_gamepad.reset(::SDL_OpenGamepad(m_index), ::SDL_CloseGamepad);
			Wrapper::maybeThrowException(m_gamepad.get(), "Unable to open gamepad");
			openHaptic();
			const auto* name = ::SDL_GetGamepadName(m_gamepad.get());
			Wrapper::maybeThrowException(name, "Unable to obtain gamepad name");
			::SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Game controller name: %s", name);
		} else {
			::SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Joystick is not a game controller");
		}
	}

	void GameController::openHaptic() {
		m_haptic.reset(::SDL_OpenHaptic(m_index), ::SDL_CloseHaptic);
		Wrapper::maybeThrowException(m_haptic.get(), "Unable to open haptic gamepad");
		const auto success = ::SDL_InitHapticRumble(m_haptic.get());
		Wrapper::maybeThrowException(success, "Unable to initialise haptic gamepad");
		m_hapticRumbleSupported = ::SDL_HapticRumbleSupported(m_haptic.get());
	}

	void GameController::closeHaptic() noexcept {
		m_haptic.reset();
		m_hapticRumbleSupported = false;
	}

	void GameController::close() noexcept {
		m_gamepad.reset();
		closeHaptic();
	}

	void GameController::startRumble() noexcept {
		if (m_hapticRumbleSupported && !::SDL_PlayHapticRumble(m_haptic.get(), 1.0, 1000))
			::SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Unable to start haptic rumble: %s", ::SDL_GetError());
	}

	void GameController::stopRumble() noexcept {
		if (m_hapticRumbleSupported && !::SDL_StopHapticRumble(m_haptic.get()))
			::SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Unable to stop haptic rumble: %s", ::SDL_GetError());
	}
}