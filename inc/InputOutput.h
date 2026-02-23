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
		Signal<uint16_t> ReadingPort;
		Signal<uint16_t> ReadPort;

		Signal<uint16_t> WritingPort;
		Signal<uint16_t> WrittenPort;

		[[nodiscard]] uint8_t read(register16_t port);
		[[nodiscard]] uint8_t readInputPort(register16_t port);
		[[nodiscard]] uint8_t readInputPort(uint16_t port);
		void writeInputPort(uint16_t port, uint8_t value) noexcept;
		void writeInputPort(register16_t port, uint8_t value);

		void write(register16_t port, uint8_t value);
		void writeOutputPort(register16_t port, uint8_t value);
		void writeOutputPort(uint16_t port, uint8_t value);
		[[nodiscard]] uint8_t readOutputPort(uint16_t port) noexcept;
		[[nodiscard]] uint8_t readOutputPort(register16_t port);
	};
}
