#include "stdafx.h"
#include "../inc/Game.h"

#include "../inc/GameController.h"

namespace Gaming {

Game::Game() {}

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
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		windowWidth(), windowHeight(),
		SDL_WINDOW_SHOWN), ::SDL_DestroyWindow);
	if (m_window == nullptr)
		SDLWrapper::throwSDLException("Unable to create window: ");

	::SDL_DisplayMode mode;
	SDLWrapper::verifySDLCall(
		::SDL_GetWindowDisplayMode(m_window.get(), &mode),
		"Unable to obtain window information: ");

	Uint32 rendererFlags = 0;
	m_vsync = useVsync();
	if (m_vsync) {
		const auto required = fps();
		if (std::abs(required - mode.refresh_rate) < 0.001) {
			rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
			::SDL_Log("Attempting to use SDL_RENDERER_PRESENTVSYNC");
		} else {
			m_vsync = false;
			::SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Display refresh rate is incompatible with required rate (%f)", required);
		}
	}
	m_renderer.reset(::SDL_CreateRenderer(m_window.get(), -1, rendererFlags), ::SDL_DestroyRenderer);
	if (m_renderer == nullptr)
		SDLWrapper::throwSDLException("Unable to create renderer: ");

	::SDL_RendererInfo info;
	SDLWrapper::verifySDLCall(
		::SDL_GetRendererInfo(m_renderer.get(), &info),
		"Unable to obtain renderer information");

	if (m_vsync) {
		if ((info.flags & SDL_RENDERER_PRESENTVSYNC) == 0) {
			::SDL_LogWarn(::SDL_LOG_CATEGORY_APPLICATION, "Renderer does not support VSYNC, reverting to timed delay loop.");
			m_vsync = false;
		}
	}

	m_pixelFormat.reset(::SDL_AllocFormat(m_pixelType), ::SDL_FreeFormat);
	if (m_pixelFormat == nullptr)
		SDLWrapper::throwSDLException("Unable to allocate pixel format: ");

	configureBackground();
	createBitmapTexture();

	m_frames = 0UL;
	m_startTicks = ::SDL_GetTicks();
}

void Game::configureBackground() const {
	SDLWrapper::verifySDLCall(
		::SDL_SetRenderDrawColor(m_renderer.get(), 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE),
		"Unable to set render draw colour");
}

void Game::createBitmapTexture() {
	m_bitmapTexture.reset(::SDL_CreateTexture(m_renderer.get(), m_pixelType, SDL_TEXTUREACCESS_STREAMING, rasterWidth(), rasterHeight()), ::SDL_DestroyTexture);
	if (m_bitmapTexture == nullptr)
		SDLWrapper::throwSDLException("Unable to create bitmap texture");
}

void Game::runLoop() {
	while (powered()) {
		update();
		draw();
		maybeSynchronise();
	}
}

void Game::update() {
	handleEvents();
	runVerticalBlank();
	runRasterLines();
}

void Game::handleEvents() {
	::SDL_Event e;
	while (::SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			lowerPOWER();
			break;
		case SDL_KEYDOWN:
			handleKeyDown(e.key.keysym.sym);
			break;
		case SDL_KEYUP:
			handleKeyUp(e.key.keysym.sym);
			break;
		case SDL_JOYBUTTONDOWN:
			handleJoyButtonDown(e.jbutton);
			break;
		case SDL_JOYBUTTONUP:
			handleJoyButtonUp(e.jbutton);
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			handleControllerButtonDown(e.cbutton);
			break;
		case SDL_CONTROLLERBUTTONUP:
			handleControllerButtonUp(e.cbutton);
			break;
		case SDL_JOYDEVICEADDED:
			addJoystick(e);
			break;
		case SDL_JOYDEVICEREMOVED:
			removeJoystick(e);
			break;
		}
	}
}

void Game::draw() {
	updateTexture();
	copyTexture();
	displayTexture();
}

bool Game::maybeSynchronise() {
	++m_frames;
	const bool synchronising = !m_vsync;
	if (synchronising)
		synchronise();
	return synchronising;
}

void Game::synchronise() {
	const auto elapsedTicks = ::SDL_GetTicks() - m_startTicks;
	const auto neededTicks = (m_frames / fps()) * 1000.0;
	const auto sleepNeeded = (int)(neededTicks - elapsedTicks);
	if (sleepNeeded > 0)
		::SDL_Delay(sleepNeeded);
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

std::shared_ptr<GameController> Game::gameController(const int which) const {
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
	SDLWrapper::verifySDLCall(
		::SDL_UpdateTexture(m_bitmapTexture.get(), nullptr, pixels(), displayWidth() * sizeof(Uint32)),
		"Unable to update texture: ");
}

void Game::copyTexture() {
	SDLWrapper::verifySDLCall(
		::SDL_RenderCopy(m_renderer.get(), m_bitmapTexture.get(), nullptr, nullptr),
		"Unable to copy texture to renderer: ");
}

void Game::displayTexture() {
	::SDL_RenderPresent(m_renderer.get());
}

void Game::toggleFullscreen() {
	auto wasFullscreen = ::SDL_GetWindowFlags(m_window.get()) & SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDLWrapper::verifySDLCall(
		::SDL_SetWindowFullscreen(m_window.get(), wasFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP),
		"Failed to modify the window full screen setting: ");
	::SDL_ShowCursor(wasFullscreen ? 1 : 0);
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

bool Game::handleControllerButtonDown(SDL_ControllerButtonEvent event) {
	return false;
}

bool Game::handleControllerButtonUp(SDL_ControllerButtonEvent event) {
	return false;
}

}