#pragma once

namespace Gaming {
	class SDLWrapper final {
	public:
		SDLWrapper();
		~SDLWrapper();

		static void throwSDLException(std::string failure) {
			throw std::runtime_error(failure + ::SDL_GetError());
		}

		static void verifySDLCall(int returned, std::string failure) {
			if (returned < 0)
				throwSDLException(failure);
		}
	};
}