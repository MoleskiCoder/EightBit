#pragma once

#include <mos6502.h>

#include <Bus.h>

namespace EightBit {
	class Ricoh2A03 final : public MOS6502 {
	public:
		Ricoh2A03(Bus& bus);
		virtual ~Ricoh2A03() = default;

	protected:
		virtual void SBC(uint8_t data) final;
		virtual void ADC(uint8_t data) final;
	};
}