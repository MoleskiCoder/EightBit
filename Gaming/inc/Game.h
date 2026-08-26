#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <SDL3/SDL.h>

#include <Device.h>

#include "Wrapper.h"

class Configuration;

namespace Gaming {

	class GameController;

	class Game : public EightBit::Device {
	public:
		Game(SDL_LogPriority logging = SDL_LOG_PRIORITY_WARN);
		virtual ~Game();

		virtual SDL_AppResult runFrame();
		virtual SDL_AppResult handleEvent(SDL_Event& e);

		void raisePOWER() noexcept override;
		void lowerPOWER() noexcept override;

		virtual void initialise();
		virtual void terminate();

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

		virtual void update();
		virtual void draw();

		virtual void runRasterLines() {};
		virtual void runVerticalBlank() {}

		void addGamepad(SDL_Event& e);
		void removeGamepad(SDL_Event& e);

		virtual void updateTexture();
		virtual void renderTexture();
		virtual void displayTexture();

		[[nodiscard]] virtual const uint32_t* pixels() const = 0;

		virtual bool handleKeyDown(SDL_Keycode key);
		virtual bool handleKeyUp(SDL_Keycode key);

		virtual bool handleJoyButtonDown(SDL_JoyButtonEvent event);
		virtual bool handleJoyButtonUp(SDL_JoyButtonEvent event);

		virtual bool handleGamepadButtonDown(SDL_GamepadButtonEvent event);
		virtual bool handleGamepadButtonUp(SDL_GamepadButtonEvent event);

		void toggleFullscreen();

		[[nodiscard]] std::shared_ptr<GameController> gamepad(int which) const;

		[[nodiscard]] int chooseControllerIndex(int who) const;
		[[nodiscard]] std::shared_ptr<GameController> chooseController(int who) const;

		[[nodiscard]] std::shared_ptr<SDL_Renderer> renderer() const noexcept { return m_renderer; }
		[[nodiscard]] std::shared_ptr<SDL_Texture> bitmapTexture() const noexcept { return m_bitmapTexture; }
		[[nodiscard]] const SDL_PixelFormatDetails* pixelFormat() const noexcept { return m_pixelFormat; }

	private:
		Wrapper m_wrapper;

		std::shared_ptr<SDL_Window> m_window;
		std::shared_ptr<SDL_Renderer> m_renderer;
		std::shared_ptr<SDL_Texture> m_bitmapTexture;
		const SDL_PixelFormatDetails* m_pixelFormat = nullptr;

		SDL_PixelFormat m_pixelType = SDL_PIXELFORMAT_ARGB8888;

		bool m_vsync = false;

		std::map<int, std::shared_ptr<GameController>> m_gameControllers;

		void configureBackground() const;
		void createBitmapTexture();
	};
}
