#pragma once

#include <string>

namespace EightBit {
	class Intel8080;

	class Disassembler {
	public:
		Disassembler();

		static std::string state(Intel8080& cpu);
		static std::string disassemble(Intel8080& cpu);

		static std::string hex(uint8_t value);
		static std::string hex(uint16_t value);
		static std::string binary(uint8_t value);

		static std::string invalid(uint8_t value);
	};
}