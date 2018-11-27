#pragma once

#include <cstdint>
#include <array>

#include "Signal.h"

namespace EightBit {
	class InputOutput final {
	public:
		InputOutput() = default;

		auto read(const uint8_t port) { return readInputPort(port); }
		void write(const uint8_t port, const uint8_t value) { return writeOutputPort(port, value); }

		uint8_t readInputPort(uint8_t port);
		void writeInputPort(const uint8_t port, const uint8_t value) noexcept { m_input[port] = value; }

		auto readOutputPort(const uint8_t port) noexcept { return m_output[port]; }
		void writeOutputPort(uint8_t port, uint8_t value);

		Signal<uint8_t> ReadingPort;
		Signal<uint8_t> ReadPort;

		Signal<uint8_t> WritingPort;
		Signal<uint8_t> WrittenPort;

	protected:
		void OnReadingPort(uint8_t port);
		void OnReadPort(uint8_t port);

		void OnWritingPort(uint8_t port);
		void OnWrittenPort(uint8_t port);

	private:
		std::array<uint8_t, 0x100> m_input = { 0 };
		std::array<uint8_t, 0x100> m_output = { 0 };
	};
}
