#pragma once

#include <memory>

#include <SDL3/SDL.h>

namespace Gaming {
	class GameController final {
	public:
		GameController(int index);
		virtual ~GameController();

		void startRumble() noexcept;
		void stopRumble() noexcept;

		[[nodiscard]] static auto buildJoystickId(SDL_Gamepad* gamepad) noexcept {
			auto joystick = ::SDL_GetGamepadJoystick(gamepad);
			return ::SDL_GetJoystickID(joystick);
		}

		[[nodiscard]] auto getJoystickId() const noexcept {
			return buildJoystickId(m_gamepad.get());
		}

	private:
		int m_index;
		std::shared_ptr<SDL_Gamepad> m_gamepad;

		void open();
		void close() noexcept;

		std::shared_ptr<SDL_Haptic> m_haptic;
		bool m_hapticRumbleSupported = false;

		void openHaptic();
		void closeHaptic() noexcept;
	};
}