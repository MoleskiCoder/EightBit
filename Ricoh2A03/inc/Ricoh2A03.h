#pragma once

#include <cstdint>
#include <mos6502.h>
#include <Bus.h>

namespace EightBit {
	class Ricoh2A03 final : public MOS6502 {
	public:
		Ricoh2A03(Bus& bus);
		virtual ~Ricoh2A03() = default;

		int clockCycles() const { return cycles();	}

	protected:
		virtual uint8_t SUB(uint8_t operand, uint8_t data, int borrow) final;
		virtual uint8_t ADD(uint8_t operand, uint8_t data, int carry) final;
	};
}