#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <SDL.h>

#include <Device.h>

#include "SDLWrapper.h"

class Configuration;

namespace Gaming {

	class GameController;

	class Game : public EightBit::Device {
	public:
		Game();
		virtual ~Game();

		virtual void runLoop();
		void raisePOWER() noexcept override;

	protected:
		[[nodiscard]] virtual float fps() const noexcept = 0;
		[[nodiscard]] virtual bool useVsync() const noexcept = 0;

		[[nodiscard]] virtual int windowWidth() const noexcept;
		[[nodiscard]] virtual int windowHeight() const noexcept;
		[[nodiscard]] virtual int displayWidth() const noexcept;
		[[nodiscard]] virtual int displayHeight() const noexcept;
		[[nodiscard]] virtual int displayScale() const noexcept = 0;
		[[nodiscard]] virtual int rasterWidth() const noexcept = 0;
		[[nodiscard]] virtual int rasterHeight() const noexcept = 0;

		[[nodiscard]] virtual std::string title() const noexcept = 0;

		virtual void handleEvents();
		virtual void update();
		virtual void draw();
		virtual bool maybeSynchronise();	// true, if manual synchronisation required
		virtual void synchronise();

		virtual void runRasterLines() {};
		virtual void runVerticalBlank() {}

		void addJoystick(SDL_Event& e);
		void removeJoystick(SDL_Event& e);

		virtual void updateTexture();
		virtual void copyTexture();
		virtual void displayTexture();

		[[nodiscard]] virtual const uint32_t* pixels() const = 0;

		virtual bool handleKeyDown(SDL_Keycode key);
		virtual bool handleKeyUp(SDL_Keycode key);

		virtual bool handleJoyButtonDown(SDL_JoyButtonEvent event);
		virtual bool handleJoyButtonUp(SDL_JoyButtonEvent event);

		virtual bool handleControllerButtonDown(SDL_ControllerButtonEvent event);
		virtual bool handleControllerButtonUp(SDL_ControllerButtonEvent event);

		void toggleFullscreen();

		[[nodiscard]] std::shared_ptr<GameController> gameController(int which) const;
		[[nodiscard]] int mappedController(const SDL_JoystickID which) const;

		[[nodiscard]] int chooseControllerIndex(int who) const;
		[[nodiscard]] std::shared_ptr<GameController> chooseController(int who) const;

		[[nodiscard]] std::shared_ptr<SDL_Renderer> renderer() const noexcept { return m_renderer; }
		[[nodiscard]] std::shared_ptr<SDL_Texture> bitmapTexture() const noexcept { return m_bitmapTexture; }
		[[nodiscard]] std::shared_ptr<SDL_PixelFormat> pixelFormat() const noexcept { return m_pixelFormat; }

	private:
		SDLWrapper m_wrapper;

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
	};
}
