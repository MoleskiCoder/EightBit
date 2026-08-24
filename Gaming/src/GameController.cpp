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
		assert(::SDL_IsGamepad(m_index));
		m_gamepad.reset(::SDL_OpenGamepad(m_index), ::SDL_CloseGamepad);
		Wrapper::maybeThrowException(m_gamepad.get(), "Unable to open gamepad");
		const auto* name = ::SDL_GetGamepadName(m_gamepad.get());
		Wrapper::maybeThrowException(name, "Unable to obtain gamepad name");
		::SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Game controller name: %s", name);
	}

	void GameController::close() noexcept {
		m_gamepad.reset();
	}

	void GameController::startRumble() noexcept {
		const auto success = ::SDL_RumbleGamepad(m_gamepad.get(), 0xFFFF, 0x0000, 1000);
		Wrapper::maybeThrowException(success, "Unable to start haptic rumble");
	}

	void GameController::stopRumble() noexcept {
	}
}