#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <SDL.h>

#include <Device.h>

#include "GameController.h"

class Configuration;

namespace Gaming {
	class Game : public EightBit::Device {
	public:

		static void throwSDLException(std::string failure) {
			throw std::runtime_error(failure + ::SDL_GetError());
		}

		static void verifySDLCall(int returned, std::string failure) {
			if (returned < 0)
				throwSDLException(failure);
		}

		Game();
		virtual ~Game();

		virtual void runLoop();
		virtual void raisePOWER() override;

	protected:
		virtual int fps() const = 0;
		virtual bool useVsync() const = 0;

		virtual int windowWidth() const noexcept { return displayWidth() * displayScale(); }
		virtual int windowHeight() const noexcept { return displayHeight() * displayScale(); }
		virtual int displayWidth() const noexcept { return rasterWidth(); }
		virtual int displayHeight() const noexcept { return rasterHeight(); }
		virtual int displayScale() const noexcept = 0;
		virtual int rasterWidth() const noexcept = 0;
		virtual int rasterHeight() const noexcept = 0;

		virtual std::string title() const = 0;

		virtual void runRasterLines() {};
		virtual void runVerticalBlank() {}

		void addJoystick(SDL_Event& e);
		void removeJoystick(SDL_Event& e);

		virtual void updateTexture();
		virtual void copyTexture();
		virtual void displayTexture();

		virtual const uint32_t* pixels() const = 0;

		std::shared_ptr<GameController> gameController(const int which) const {
			const auto i = m_gameControllers.find(which);
			if (i == m_gameControllers.cend())
				throw std::runtime_error("Unknown controller");
			return i->second;
		}

		int mappedController(const SDL_JoystickID which) const {
			const auto i = m_mappedControllers.find(which);
			if (i == m_mappedControllers.cend())
				throw std::runtime_error("Unknown joystick");
			return i->second;
		}

		int chooseControllerIndex(int who) const;
		std::shared_ptr<GameController> chooseController(int who) const;

		std::shared_ptr<SDL_Renderer> renderer() const noexcept { return m_renderer; }
		std::shared_ptr<SDL_Texture> bitmapTexture() const noexcept { return m_bitmapTexture; }
		std::shared_ptr<SDL_PixelFormat> pixelFormat() const noexcept { return m_pixelFormat; }

	private:
		std::shared_ptr<SDL_Window> m_window;
		std::shared_ptr<SDL_Renderer> m_renderer;
		std::shared_ptr<SDL_Texture> m_bitmapTexture;
		std::shared_ptr<SDL_PixelFormat> m_pixelFormat;

		Uint32 m_pixelType = SDL_PIXELFORMAT_ARGB8888;

		bool m_vsync = false;
		Uint32 m_startTicks = 0;
		Uint32 m_frames = 0;

		std::map<int, std::shared_ptr<GameController>> m_gameControllers;
		std::map<SDL_JoystickID, int> m_mappedControllers;

		void configureBackground() const;
		void createBitmapTexture();

		virtual void handleKeyDown(SDL_Keycode key) {}
		virtual void handleKeyUp(SDL_Keycode key) {}

		virtual void handleJoyButtonDown(SDL_JoyButtonEvent event) {}
		virtual void handleJoyButtonUp(SDL_JoyButtonEvent event) {}
	};
}
