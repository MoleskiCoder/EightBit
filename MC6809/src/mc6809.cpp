#include "stdafx.h"
#include "mc6809.h"

EightBit::mc6809::mc6809(Bus& bus)
: BigEndianProcessor(bus) {}

void EightBit::mc6809::powerOn() {
	Processor::powerOn();
}

int EightBit::mc6809::step() {
	resetCycles();
	auto returned = 0;
	if (LIKELY(powered())) {
		ExecutingInstruction.fire(*this);
		returned = execute(fetchByte());
		ExecutedInstruction.fire(*this);
	}
	return returned;
}

void EightBit::mc6809::reset() {
	Processor::reset();
	DP() = 0;	// Reestablish zero page
	CC() |= (IF & FF);	// Disable interrupts
	jump(getWordPaged(0xff, RESETvector));
}

int EightBit::mc6809::execute(uint8_t cell) {

	switch (cell) {

	// ABX
	case 0x3a:	addCycles(3);	abx();								break;		// ABX (inherent)

	// ADC
	case 0x89:	addCycles(2);	A() = adc(A(), AM_immediate());		break;		// ADC (ADCA, immediate)
	case 0x99:	addCycles(4);	A() = adc(A(), AM_direct());		break;		// ADC (ADCA, direct)
	case 0xA9:	addCycles(4);	A() = adc(A(), AM_indexed());		break;		// ADC (ADCA, indexed)
	case 0xB9:	addCycles(4);	A() = adc(A(), AM_extended());		break;		// ADC (ADCA, extended)
	case 0xC9:	addCycles(2);	B() = adc(B(), AM_immediate());		break;		// ADC (ADCB, immediate)
	case 0xD9:	addCycles(4);	B() = adc(B(), AM_direct());		break;		// ADC (ADCB, direct)
	case 0xE9:	addCycles(4);	B() = adc(B(), AM_indexed());		break;		// ADC (ADCB, indexed)
	case 0xF9:	addCycles(4);	B() = adc(B(), AM_extended());		break;		// ADC (ADCB, extended)

	// NEG
	case 0x00:	addCycles(6);	BUS().write(neg(AM_direct()));		break;		// NEG (direct)
	case 0x40:	addCycles(2);	A() = neg(A());						break;		// NEG (NEGA, inherent)
	case 0x50:	addCycles(2);	B() = neg(B());						break;		// NEG (NEGB, inherent)
	case 0x60:	addCycles(6);	BUS().write(neg(AM_indexed()));		break;		// NEG (indexed)
	case 0x70:	addCycles(7);	BUS().write(neg(AM_extended()));	break;		// NEG (extended)

	default:
		ASSUME(false);
	}

	ASSUME(cycles() > 0);
	return cycles();
}

//

void EightBit::mc6809::abx() {
	X() += B();
}

uint8_t EightBit::mc6809::neg(uint8_t operand) {
	setFlag(CC(), VF, operand == Bit7);
	const register16_t result = 0 - operand;
	operand = result.low;
	setFlag(CC(), NF, operand & Bit7);
	setFlag(CC(), ZF, operand == 0);
	setFlag(CC(), CF, result.word & Bit8);
	return operand;
}

uint8_t EightBit::mc6809::adc(uint8_t operand, uint8_t data) {
	return add(operand, data, CC() & CF);
}

uint8_t EightBit::mc6809::add(uint8_t operand, uint8_t data, int carry) {
	const register16_t result = operand + data + carry;
	setFlag(CC(), NF, result.low & Bit7);
	setFlag(CC(), ZF, result.low == 0);
	setFlag(CC(), CF, result.word & Bit8);
	return result.low;
}