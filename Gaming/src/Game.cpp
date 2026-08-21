#include "stdafx.h"
#include "../inc/Game.h"

#include "../inc/GameController.h"

namespace Gaming {

Game::Game(bool verbose /* = false */)
: m_wrapper(verbose) {}

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

	m_window.reset(::SDL_CreateWindow(
		title().c_str(),
		windowWidth(), windowHeight(),
		0), ::SDL_DestroyWindow);
	Wrapper::maybeThrowException(m_window.get(), "Unable to create window");

	const auto displayID = SDL_GetDisplayForWindow(m_window.get());
	const auto* mode = ::SDL_GetCurrentDisplayMode(displayID);
	Wrapper::maybeThrowException(mode, "Unable to obtain window information");

	m_renderer.reset(::SDL_CreateRenderer(
		m_window.get(),
		nullptr), ::SDL_DestroyRenderer);
	Wrapper::maybeThrowException(m_renderer.get(), "Unable to create renderer: ");

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

	m_pixelFormat = SDL_GetPixelFormatDetails(m_pixelType);
	Wrapper::maybeThrowException(m_pixelFormat, "Unable to obtain pixel format details");

	configureBackground();
	createBitmapTexture();

	m_performanceFrequency = ::SDL_GetPerformanceFrequency();
	m_targetFrameTime = 1.0 / fps();
}

void Game::configureBackground() const {
	const auto success = ::SDL_SetRenderDrawColor(m_renderer.get(), 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
	Wrapper::maybeThrowException(success, "Unable to set render draw colour");
}

void Game::createBitmapTexture() {
	m_bitmapTexture.reset(::SDL_CreateTexture(m_renderer.get(), m_pixelType, SDL_TEXTUREACCESS_STREAMING, rasterWidth(), rasterHeight()), ::SDL_DestroyTexture);
	Wrapper::maybeThrowException(m_bitmapTexture.get(), "Unable to create bitmap texture");
}

void Game::runLoop() {
	while (powered()) {
		update();
		draw();
		maybeSynchronise();
	}
}

void Game::update() {
	m_frameStartTime = ::SDL_GetPerformanceCounter();
	handleEvents();
	runVerticalBlank();
	runRasterLines();
}

void Game::handleEvents() {
	::SDL_Event e;
	while (::SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_EVENT_QUIT:
			lowerPOWER();
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
		case SDL_EVENT_JOYSTICK_ADDED:
			addJoystick(e);
			break;
		case SDL_EVENT_JOYSTICK_REMOVED:
			removeJoystick(e);
			break;
		}
	}
}

void Game::draw() {
	updateTexture();
	renderTexture();
	displayTexture();
}

bool Game::maybeSynchronise() {
	const bool synchronising = !m_vsync;
	if (synchronising)
		synchronise();
	return synchronising;
}

void Game::synchronise() {
	
	m_frameEndTime = ::SDL_GetPerformanceCounter();

	const auto frameTime = m_frameEndTime - m_frameStartTime;	// In performance frequency
	::SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Frame time (ticks): %ld", frameTime);

	const auto elapsedFrameTime = double(frameTime) / m_performanceFrequency;	// In seconds
	::SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Frame time (seconds): %f", elapsedFrameTime);

	const auto gap = m_targetFrameTime - elapsedFrameTime;	// in seconds
	::SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Timing gap (seconds): %f", gap);

	if (gap > 0) {
		const auto delay = Uint32(gap * 1000.0);
		::SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Delay (ticks): %d", delay);
		SDL_Delay(delay);
	}

	if (gap < 0) {
		::SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Running slowly");
	}
}

void Game::removeJoystick(SDL_Event& e) {
	const auto which = e.jdevice.which;
	const auto found = m_gameControllers.find(which);
	SDL_assert(found != m_gameControllers.end());
	auto& controller = found->second;
	const auto joystickId = controller->getJoystickId();
	m_mappedControllers.erase(joystickId);
	m_gameControllers.erase(which);
	SDL_Log("Joystick device %d removed (%zd controllers)", which, m_gameControllers.size());
}

void Game::addJoystick(SDL_Event& e) {
	const auto which = e.jdevice.which;
	SDL_assert(m_gameControllers.find(which) == m_gameControllers.end());
	auto controller = std::make_shared<GameController>(which);
	const auto joystickId = controller->getJoystickId();
	m_gameControllers[which] = controller;
	SDL_assert(m_mappedControllers.contains(joystickId));
	m_mappedControllers[joystickId] = which;
	SDL_Log("Joystick device %d added (%zd controllers)", which, m_gameControllers.size());
}

std::shared_ptr<GameController> Game::gamepad(const int which) const {
	const auto i = m_gameControllers.find(which);
	if (i == m_gameControllers.cend())
		throw std::runtime_error("Unknown controller");
	return i->second;
}

int Game::mappedController(const SDL_JoystickID which) const {
	const auto i = m_mappedControllers.find(which);
	if (i == m_mappedControllers.cend())
		throw std::runtime_error("Unknown joystick");
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
	SDL_assert(found != m_gameControllers.cend());
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