#pragma once

#include <string>
#include <stdexcept>
#include <sstream>

namespace Gaming {
	class Wrapper final : public EightBit::Device {

		using base = Device;

	public:
		Wrapper(SDL_LogPriority logging = SDL_LOG_PRIORITY_WARN);
		~Wrapper() = default;

		void raisePOWER() noexcept final;
		void lowerPOWER() noexcept final;

		void initialise();
		void terminate();

		static void throwException(const std::string& failure) {
			std::ostringstream output;
			output << "SDL: " << failure << ": " << ::SDL_GetError();
			throw std::runtime_error(output.str());
		}

		static void maybeThrowException(bool success, const std::string& failure) {
			if (!success)
				throwException(failure);
		}

		static void maybeThrowException(void* handle, const std::string& failure) {
			maybeThrowException(handle != nullptr, failure);
		}

	private:
		SDL_LogPriority m_logging;
	};
}