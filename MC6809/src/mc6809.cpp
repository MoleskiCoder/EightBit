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
	case 0x3a:	addCycles(3);	abx();				break;		// ABX (inherent)

	// NEG
	case 0x00:	addCycles(6);	neg(AM_direct());	break;		// NEG (direct)
	case 0x40:	addCycles(2);	neg(A());			break;		// NEG (NEGA, inherent)
	case 0x50:	addCycles(2);	neg(B());			break;		// NEG (NEGB, inherent)
	case 0x60:	addCycles(6);	neg(AM_indexed());	break;		// NEG (indexed)
	case 0x70:	addCycles(7);	neg(AM_extended());	break;		// NEG (extended)

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

void EightBit::mc6809::neg(uint8_t& operand) {
	setFlag(CC(), VF, operand == Bit7);
	const register16_t result = 0 - operand;
	operand = result.low;
	setFlag(CC(), NF, operand & Bit7);
	setFlag(CC(), ZF, operand == 0);
	setFlag(CC(), CF, result.word & Bit8);
}