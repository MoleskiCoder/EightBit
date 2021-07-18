#pragma once

#include <cstdint>
#include <mos6502.h>
#include <Bus.h>

namespace EightBit {
	class Ricoh2A03 final : public MOS6502 {
	public:
		Ricoh2A03(Bus& bus);
		virtual ~Ricoh2A03() = default;

	protected:
		virtual uint8_t sub(uint8_t operand, uint8_t data, int borrow) noexcept final;
		virtual uint8_t add(uint8_t operand, uint8_t data, int carry) noexcept final;
	};
}