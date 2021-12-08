#pragma once

#include <memory>

#include <SDL.h>

namespace Gaming {
	class GameController final {
	public:
		GameController(int index);
		virtual ~GameController();

		void startRumble() noexcept;
		void stopRumble() noexcept;

		[[nodiscard]] static auto buildJoystickId(SDL_GameController* controller) noexcept {
			auto joystick = ::SDL_GameControllerGetJoystick(controller);
			return ::SDL_JoystickInstanceID(joystick);
		}

		[[nodiscard]] auto getJoystickId() const noexcept {
			return buildJoystickId(m_gameController.get());
		}

	private:
		int m_index;
		std::shared_ptr<SDL_GameController> m_gameController;

		void open();
		void close() noexcept;

		std::shared_ptr<SDL_Haptic> m_hapticController;
		bool m_hapticRumbleSupported = false;

		void openHapticController();
		void closeHapticController() noexcept;
	};
}