#pragma once

#include <memory>

#include <SDL3/SDL.h>

#include "Wrapper.h"

namespace Gaming {
	class GameController final {
	public:
		GameController(int index);
		virtual ~GameController();

		void startRumble() noexcept;
		void stopRumble() noexcept;

	private:
		int m_index;
		std::shared_ptr<SDL_Gamepad> m_gamepad;

		void open();
		void close() noexcept;
	};
}