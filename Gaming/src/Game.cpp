#include "stdafx.h"
#include "../inc/Game.h"

#include "../inc/GameController.h"

#include <format>

namespace Gaming {

Game::Game(SDL_LogPriority logging /* = SDL_LOG_PRIORITY_WARN */)
: m_wrapper(logging) {}

Game::~Game() {}

int Game::windowWidth() const noexcept {
	return displayWidth() * displayScale();
}

int Game::windowHeight() const noexcept {
	return displayHeight() * displayScale();
}

int Game::displayWidth() const noexcept {
	return rasterWidth();
}

int Game::displayHeight() const noexcept {
	return rasterHeight();
}

void Game::raisePOWER() noexcept {
	Device::raisePOWER();
	initialise();
}

void Game::lowerPOWER() noexcept {
	terminate();
	Device::lowerPOWER();
}

void Game::initialise() {

	m_wrapper.raisePOWER();

	m_window.reset(::SDL_CreateWindow(title().c_str(), windowWidth(), windowHeight(), 0), ::SDL_DestroyWindow);
	Wrapper::maybeThrowException(m_window.get(), "Unable to create window");

	const auto displayID = SDL_GetDisplayForWindow(m_window.get());
	Wrapper::maybeThrowException(displayID != 0, "Unable to get display for window");

	const auto* mode = ::SDL_GetCurrentDisplayMode(displayID);
	Wrapper::maybeThrowException(mode, "Unable to obtain window display mode");

	m_renderer.reset(::SDL_CreateRenderer(m_window.get(), nullptr), ::SDL_DestroyRenderer);
	Wrapper::maybeThrowException(m_renderer.get(), "Unable to create renderer");

	m_vsync = useVsync();
	if (m_vsync) {
		assert(mode != nullptr);
		const auto difference = fps() - mode->refresh_rate;
		m_vsync = std::abs(difference) < 0.001;
		if (m_vsync) {
			SDL_Log("Attempting to configure renderer VSYNC");
			m_vsync = SDL_SetRenderVSync(m_renderer.get(), 1);
			if (!m_vsync) {
				const auto error = SDL_GetError();
				SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to set render VSYNC (%s)", error);
			}
		} else {
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Display refresh rate is incompatible with required rate (%f)", fps());
		}
	}

	if (!m_vsync) {
		SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Setting callback rate hint");
		const auto success = SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, std::format("{}", fps()).c_str());
		Wrapper::maybeThrowException(success, "Unable to set event loop callback rate hint");
	}

	m_pixelFormat = SDL_GetPixelFormatDetails(m_pixelType);
	Wrapper::maybeThrowException(m_pixelFormat, "Unable to obtain pixel format details");

	configureBackground();
	createBitmapTexture();
}

void Game::terminate() {
	m_bitmapTexture.reset();
	m_renderer.reset();
	m_window.reset();
	m_wrapper.lowerPOWER();
}

void Game::configureBackground() const {
	const auto success = ::SDL_SetRenderDrawColor(m_renderer.get(), 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
	Wrapper::maybeThrowException(success, "Unable to set render draw colour");
}

void Game::createBitmapTexture() {
	m_bitmapTexture.reset(::SDL_CreateTexture(m_renderer.get(), m_pixelType, SDL_TEXTUREACCESS_STREAMING, rasterWidth(), rasterHeight()), ::SDL_DestroyTexture);
	Wrapper::maybeThrowException(m_bitmapTexture.get(), "Unable to create bitmap texture");
}

SDL_AppResult Game::runFrame() {
	update();
	draw();
	return SDL_APP_CONTINUE;
}

void Game::update() {
	runVerticalBlank();
	runRasterLines();
}

SDL_AppResult Game::handleEvent(SDL_Event& e) {
	switch (e.type) {
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
		break;
	case SDL_EVENT_KEY_DOWN:
		handleKeyDown(e.key.key);
		break;
	case SDL_EVENT_KEY_UP:
		handleKeyUp(e.key.key);
		break;
	case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
		handleJoyButtonDown(e.jbutton);
		break;
	case SDL_EVENT_JOYSTICK_BUTTON_UP:
		handleJoyButtonUp(e.jbutton);
		break;
	case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		handleGamepadButtonDown(e.gbutton);
		break;
	case SDL_EVENT_GAMEPAD_BUTTON_UP:
		handleGamepadButtonUp(e.gbutton);
		break;
	case SDL_EVENT_GAMEPAD_ADDED:
		addGamepad(e);
		break;
	case SDL_EVENT_GAMEPAD_REMOVED:
		removeGamepad(e);
		break;
	}
	return SDL_APP_CONTINUE;
}

void Game::draw() {
	updateTexture();
	renderTexture();
	displayTexture();
}

void Game::removeGamepad(SDL_Event& e) {
	const auto which = e.gdevice.which;
	const auto found = m_gameControllers.find(which);
	assert(found != m_gameControllers.end());
	m_gameControllers.erase(which);
	SDL_Log("Joystick device %d removed (%zd controllers)", which, m_gameControllers.size());
}

void Game::addGamepad(SDL_Event& e) {
	const auto which = e.gdevice.which;
	assert(m_gameControllers.find(which) == m_gameControllers.end());
	auto controller = std::make_shared<GameController>(which);
	m_gameControllers[which] = controller;
}

std::shared_ptr<GameController> Game::gamepad(const int which) const {
	const auto i = m_gameControllers.find(which);
	if (i == m_gameControllers.cend())
		throw std::runtime_error("Unknown controller");
	return i->second;
}

// -1 if no controllers, otherwise index
int Game::chooseControllerIndex(const int who) const {
	const auto count = m_gameControllers.size();
	if (count == 0)
		return -1;
	auto firstController = m_gameControllers.cbegin();
	if (count == 1 || (who == 1))
		return firstController->first;
	auto secondController = (++firstController)->first;
	return secondController;
}

std::shared_ptr<GameController> Game::chooseController(const int who) const {
	const auto which = chooseControllerIndex(who);
	if (which == -1)
		return nullptr;
	const auto found = m_gameControllers.find(which);
	assert(found != m_gameControllers.cend());
	return found->second;
}

void Game::updateTexture() {
	const auto success = ::SDL_UpdateTexture(m_bitmapTexture.get(), nullptr, pixels(), displayWidth() * sizeof(Uint32));
	Wrapper::maybeThrowException(success, "Unable to update texture");
}

void Game::renderTexture() {
	const auto success = ::SDL_RenderTexture(m_renderer.get(), m_bitmapTexture.get(), nullptr, nullptr);
	Wrapper::maybeThrowException(success, "Unable to render texture");
}

void Game::displayTexture() {
	const auto success = ::SDL_RenderPresent(m_renderer.get());
	Wrapper::maybeThrowException(success, "Unable to present render to screen");

}

void Game::toggleFullscreen() {
	const auto wasFullscreen = (SDL_GetWindowFlags(m_window.get()) & SDL_WINDOW_FULLSCREEN) != 0;
	auto success = SDL_SetWindowFullscreen(m_window.get(), !wasFullscreen);
	Wrapper::maybeThrowException(success, "Failed to toggle window full screen setting");
	success = wasFullscreen ? SDL_ShowCursor() : SDL_HideCursor();
	Wrapper::maybeThrowException(success, "Failed to toggle cursor show/hide");
}

bool Game::handleKeyDown(SDL_Keycode key) {
	switch (key) {
	case SDLK_F12:
		// Don't let it get poked.
		return true;
		break;
	default:
		return false;
	}
}

bool Game::handleKeyUp(SDL_Keycode key) {
	switch (key) {
	case SDLK_F12:
		toggleFullscreen();
		return true;
		break;
	default:
		return false;
	}
}

bool Game::handleJoyButtonDown(SDL_JoyButtonEvent event) {
	return false;
}

bool Game::handleJoyButtonUp(SDL_JoyButtonEvent event) {
	return false;
}

bool Game::handleGamepadButtonDown(SDL_GamepadButtonEvent event) {
	return false;
}

bool Game::handleGamepadButtonUp(SDL_GamepadButtonEvent event) {
	return false;
}

}