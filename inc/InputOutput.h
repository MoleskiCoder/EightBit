#pragma once

#include <cstdint>
#include <array>

#include "Signal.h"
#include "Register.h"

namespace EightBit {

	class InputOutput final {
	private:
		std::array<uint8_t, 0x10000> _input = {};
		std::array<uint8_t, 0x10000> _output = {};

	public:
		Signal<register16_t> ReadingPort;
		Signal<register16_t> ReadPort;

		Signal<register16_t> WritingPort;
		Signal<register16_t> WrittenPort;

		[[nodiscard]] uint8_t read(register16_t port);
		[[nodiscard]] uint8_t readInputPort(register16_t port);
		void writeInputPort(register16_t port, uint8_t value);

		void write(register16_t port, uint8_t value);
		void writeOutputPort(register16_t port, uint8_t value);
		[[nodiscard]] uint8_t readOutputPort(register16_t port);
	};
}
