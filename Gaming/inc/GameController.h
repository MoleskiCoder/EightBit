#pragma once

#include <SDL.h>

namespace Gaming {
	class GameController final {
	public:
		GameController(int index);
		virtual ~GameController();

		void startRumble() noexcept;
		void stopRumble() noexcept;

		static auto buildJoystickId(SDL_GameController* controller) noexcept {
			auto joystick = ::SDL_GameControllerGetJoystick(controller);
			return ::SDL_JoystickInstanceID(joystick);
		}

		auto getJoystickId() const noexcept {
			return buildJoystickId(m_gameController);
		}

	private:
		int m_index;
		SDL_GameController* m_gameController = nullptr;

		void open();
		void close() noexcept;

		SDL_Haptic* m_hapticController = nullptr;
		bool m_hapticRumbleSupported = false;

		void openHapticController();
		void closeHapticController() noexcept;
	};
}