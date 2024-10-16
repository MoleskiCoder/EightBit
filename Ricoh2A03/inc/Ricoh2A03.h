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
		uint8_t sub(uint8_t operand, int borrow) noexcept final;
		void adc() noexcept final;
	};
}