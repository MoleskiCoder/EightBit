#include "stdafx.h"
#include "mos6502.h"

EightBit::MOS6502::MOS6502(Bus& bus)
: LittleEndianProcessor(bus) {}

void EightBit::MOS6502::powerOn() {

	Processor::powerOn();

	X() = Bit7;
	Y() = 0;
	A() = 0;
	P() = RF;
	S() = Mask8;

	raise(SO());
}

void EightBit::MOS6502::handleSO() {
	raise(SO());
	P() |= VF;
}

void EightBit::MOS6502::handleRESET() {
	Processor::handleRESET();
	jump(getWordPaged(0xff, RSTvector));
	addCycles(4);	// ?? TBC
}


void EightBit::MOS6502::handleNMI() {
	raise(NMI());
	interrupt(NMIvector);
	addCycles(4);	// ?? TBC
}

void EightBit::MOS6502::handleIRQ() {
	Processor::handleIRQ();
	interrupt(IRQvector);
	addCycles(4);	// ?? TBC
}

void EightBit::MOS6502::handleHALT() {
	execute(0xea);	// NOP
	addCycles(2);
}

int EightBit::MOS6502::step() {
	resetCycles();
	if (LIKELY(powered())) {
		ExecutingInstruction.fire(*this);
		if (UNLIKELY(lowered(SO()))) {
			handleSO();
		}
		if (lowered(RESET())) {
			handleRESET();
		} else if (UNLIKELY(lowered(NMI()))) {
			handleNMI();
		} else if (UNLIKELY(lowered(IRQ()))) {
			handleIRQ();
		} else if (UNLIKELY(lowered(HALT()))) {
			handleHALT();
		} else {
			execute(fetchByte());
		}
		ExecutedInstruction.fire(*this);
	}
	return cycles();
}

void EightBit::MOS6502::interrupt(uint8_t vector) {
	raise(HALT());
	pushWord(PC());
	push(P());
	setFlag(P(), IF);
	jump(getWordPaged(0xff, vector));
}

int EightBit::MOS6502::execute(uint8_t cell) { 

	switch (cell) {

	case 0x00:	addCycles(7);	BRK();									break;
	case 0x01:	addCycles(6);	ORA(AM_IndexedIndirectX());				break;
	case 0x02:	addCycles(0);											break;
	case 0x03:	addCycles(8);	SLO(AM_IndexedIndirectX());				break;
	case 0x04:	addCycles(3);	AM_ZeroPage();							break;
	case 0x05:	addCycles(3);	ORA(AM_ZeroPage());						break;
	case 0x06:	addCycles(5);	BUS().write(ASL(AM_ZeroPage()));		break;
	case 0x07:	addCycles(5);	SLO(AM_ZeroPage());						break;
	case 0x08:	addCycles(3);	PHP();									break;
	case 0x09:	addCycles(2);	ORA(AM_Immediate());					break;
	case 0x0a:	addCycles(2);	A() = ASL(A());							break;
	case 0x0b:	addCycles(2);	AAC(AM_Immediate());					break;
	case 0x0c:	addCycles(4);	AM_Absolute();							break;
	case 0x0d:	addCycles(4);	ORA(AM_Absolute());						break;
	case 0x0e:	addCycles(6);	BUS().write(ASL(AM_Absolute()));		break;
	case 0x0f:	addCycles(6);	SLO(AM_Absolute());						break;

	case 0x10:	addCycles(2);	Branch(!(P() & NF));					break;
	case 0x11:	addCycles(5);	ORA(AM_IndirectIndexedY());				break;
	case 0x12:	addCycles(0);											break;
	case 0x13:	addCycles(7);	SLO(AM_IndirectIndexedY());				break;
	case 0x14:	addCycles(4);	AM_ZeroPageX();							break;
	case 0x15:	addCycles(4);	ORA(AM_ZeroPageX());					break;
	case 0x16:	addCycles(6);	BUS().write(ASL(AM_ZeroPageX()));		break;
	case 0x17:	addCycles(6);	SLO(AM_ZeroPageX());					break;
	case 0x18:	addCycles(2);	clearFlag(P(), CF);						break;
	case 0x19:	addCycles(4);	ORA(AM_AbsoluteY());					break;
	case 0x1a:	addCycles(2);											break;
	case 0x1b:	addCycles(6);	SLO(AM_AbsoluteY());					break;
	case 0x1c:	addCycles(4);	AM_AbsoluteX();							break;
	case 0x1d:	addCycles(4);	ORA(AM_AbsoluteX());					break;
	case 0x1e:	addCycles(7);	BUS().write(ASL(AM_AbsoluteX()));		break;
	case 0x1f:	addCycles(6);	SLO(AM_AbsoluteX());					break;

	case 0x20:	addCycles(6);	JSR_abs();								break;
	case 0x21:	addCycles(6);	ANDA(AM_IndexedIndirectX());			break;
	case 0x22:	addCycles(0);											break;
	case 0x23:	addCycles(8);	RLA(AM_IndexedIndirectX());				break;
	case 0x24:	addCycles(3);	BIT(AM_ZeroPage());						break;
	case 0x25:	addCycles(3);	ANDA(AM_ZeroPage());					break;
	case 0x26:	addCycles(5);	BUS().write(ROL(AM_ZeroPage()));		break;
	case 0x27:	addCycles(5);	RLA(AM_ZeroPage());						break;
	case 0x28:	addCycles(4);	PLP();									break;
	case 0x29:	addCycles(2);	ANDA(AM_Immediate());					break;
	case 0x2a:	addCycles(2);	A() = ROL(A());							break;
	case 0x2b:	addCycles(2);	AAC(AM_Immediate());					break;
	case 0x2c:	addCycles(4);	BIT(AM_Absolute());						break;
	case 0x2d:	addCycles(4);	ANDA(AM_Absolute());					break;
	case 0x2e:	addCycles(6);	BUS().write(ROL(AM_Absolute()));		break;
	case 0x2f:	addCycles(6);	RLA(AM_Absolute());						break;

	case 0x30:	addCycles(2);	Branch(!!(P() & NF));					break;
	case 0x31:	addCycles(5);	ANDA(AM_IndirectIndexedY());			break;
	case 0x32:	addCycles(0);											break;
	case 0x33:	addCycles(7);	RLA(AM_IndirectIndexedY());				break;
	case 0x34:	addCycles(4);	AM_ZeroPageX();							break;
	case 0x35:	addCycles(4);	ANDA(AM_ZeroPageX());					break;
	case 0x36:	addCycles(6);	BUS().write(ROL(AM_ZeroPageX()));		break;
	case 0x37:	addCycles(6);	RLA(AM_ZeroPageX());					break;
	case 0x38:	addCycles(2);	setFlag(P(), CF);						break;
	case 0x39:	addCycles(4);	ANDA(AM_AbsoluteY());					break;
	case 0x3a:	addCycles(2);											break;
	case 0x3b:	addCycles(6);	RLA(AM_AbsoluteY());					break;
	case 0x3c:	addCycles(4);	AM_AbsoluteX();							break;
	case 0x3d:	addCycles(4);	ANDA(AM_AbsoluteX());					break;
	case 0x3e:	addCycles(7);	BUS().write(ROL(AM_AbsoluteX()));		break;
	case 0x3f:	addCycles(6);	RLA(AM_AbsoluteX());					break;

	case 0x40:	addCycles(6);	RTI();									break;
	case 0x41:	addCycles(6);	EORA(AM_IndexedIndirectX());			break;
	case 0x42:	addCycles(0);											break;
	case 0x43:	addCycles(8);	SRE(AM_IndexedIndirectX());				break;
	case 0x44:	addCycles(3);	AM_ZeroPage();							break;
	case 0x45:	addCycles(3);	EORA(AM_ZeroPage());					break;
	case 0x46:	addCycles(5);	BUS().write(LSR(AM_ZeroPage()));		break;
	case 0x47:	addCycles(5);	SRE(AM_ZeroPage());						break;
	case 0x48:	addCycles(3);	push(A());								break;
	case 0x49:	addCycles(2);	EORA(AM_Immediate());					break;
	case 0x4a:	addCycles(2);	A() = LSR(A());							break;
	case 0x4b:	addCycles(2);	ASR(AM_Immediate());					break;
	case 0x4c:	addCycles(3);	JMP_abs();								break;
	case 0x4d:	addCycles(4);	EORA(AM_Absolute());					break;
	case 0x4e:	addCycles(6);	BUS().write(LSR(AM_Absolute()));		break;
	case 0x4f:	addCycles(6);	SRE(AM_Absolute());						break;

	case 0x50:	addCycles(2);	Branch(!(P() & VF));					break;
	case 0x51:	addCycles(5);	EORA(AM_IndirectIndexedY());			break;
	case 0x52:	addCycles(0);											break;
	case 0x53:	addCycles(7);	SRE(AM_IndirectIndexedY());				break;
	case 0x54:	addCycles(4);	AM_ZeroPage();							break;
	case 0x55:	addCycles(4);	EORA(AM_ZeroPageX());					break;
	case 0x56:	addCycles(6);	BUS().write(LSR(AM_ZeroPageX()));		break;
	case 0x57:	addCycles(6);	SRE(AM_ZeroPageX());					break;
	case 0x58:	addCycles(2);	clearFlag(P(), IF);						break;
	case 0x59:	addCycles(4);	EORA(AM_AbsoluteY());					break;
	case 0x5a:	addCycles(2);											break;
	case 0x5b:	addCycles(6);	SRE(AM_AbsoluteY());					break;
	case 0x5c:	addCycles(4);	AM_AbsoluteX();							break;
	case 0x5d:	addCycles(4);	EORA(AM_AbsoluteX());					break;
	case 0x5e:	addCycles(7);	BUS().write(LSR(AM_AbsoluteX()));		break;
	case 0x5f:	addCycles(6);	SRE(AM_AbsoluteX());					break;

	case 0x60:	addCycles(6);	RTS();									break;
	case 0x61:	addCycles(6);	A() = ADC(A(), AM_IndexedIndirectX());	break;
	case 0x62:	addCycles(0);											break;
	case 0x63:	addCycles(8);	RRA(AM_IndexedIndirectX());				break;
	case 0x64:	addCycles(3);	AM_ZeroPage();							break;
	case 0x65:	addCycles(3);	A() = ADC(A(), AM_ZeroPage());			break;
	case 0x66:	addCycles(5);	BUS().write(ROR(AM_ZeroPage()));		break;
	case 0x67:	addCycles(5);	RRA(AM_ZeroPage());						break;
	case 0x68:	addCycles(4);	adjustNZ(A() = pop());					break;
	case 0x69:	addCycles(2);	A() = ADC(A(), AM_Immediate());			break;
	case 0x6a:	addCycles(2);	A() = ROR(A());							break;
	case 0x6b:	addCycles(2);	ARR(AM_Immediate());					break;
	case 0x6c:	addCycles(5);	JMP_ind();								break;
	case 0x6d:	addCycles(4);	A() = ADC(A(), AM_Absolute());			break;
	case 0x6e:	addCycles(6);	BUS().write(ROR(AM_Absolute()));		break;
	case 0x6f:	addCycles(6);	RRA(AM_Absolute());						break;

	case 0x70:	addCycles(2);	Branch(!!(P() & VF));					break;
	case 0x71:	addCycles(5);	A() = ADC(A(), AM_IndirectIndexedY());	break;
	case 0x72:	addCycles(0);											break;
	case 0x73:	addCycles(7);	RRA(AM_IndirectIndexedY());				break;
	case 0x74:	addCycles(4);	AM_ZeroPageX();							break;
	case 0x75:	addCycles(4);	A() = ADC(A(), AM_ZeroPageX());			break;
	case 0x76:	addCycles(6);	BUS().write(ROR(AM_ZeroPageX()));		break;
	case 0x77:	addCycles(6);	RRA(AM_ZeroPageX());					break;
	case 0x78:	addCycles(2);	setFlag(P(), IF);						break;
	case 0x79:	addCycles(4);	A() = ADC(A(), AM_AbsoluteY());			break;
	case 0x7a:	addCycles(2);											break;
	case 0x7b:	addCycles(6);	RRA(AM_AbsoluteY());					break;
	case 0x7c:	addCycles(4);	AM_AbsoluteX();							break;
	case 0x7d:	addCycles(4);	A() = ADC(A(), AM_AbsoluteX());			break;
	case 0x7e:	addCycles(7);	BUS().write(ROR(AM_AbsoluteX()));		break;
	case 0x7f:	addCycles(6);	RRA(AM_AbsoluteX());					break;

	case 0x80:	addCycles(2);	AM_Immediate();							break;
	case 0x81:	addCycles(6);	AM_IndexedIndirectX(A());				break;
	case 0x82:	addCycles(2);	AM_Immediate();							break;
	case 0x83:	addCycles(6);	AM_IndexedIndirectX(A() & X());			break;
	case 0x84:	addCycles(3);	AM_ZeroPage(Y());						break;
	case 0x85:	addCycles(3);	AM_ZeroPage(A());						break;
	case 0x86:	addCycles(3);	AM_ZeroPage(X());						break;
	case 0x87:	addCycles(3);	AM_ZeroPage(A() & X());					break;
	case 0x88:	addCycles(2);	adjustNZ(--Y());						break;
	case 0x89:	addCycles(2);	AM_Immediate();							break;
	case 0x8a:	addCycles(2);	adjustNZ(A() = X());					break;
	case 0x8b:	addCycles(0);											break;
	case 0x8c:	addCycles(4);	AM_Absolute(Y());						break;
	case 0x8d:	addCycles(4);	AM_Absolute(A());						break;
	case 0x8e:	addCycles(4);	AM_Absolute(X());						break;
	case 0x8f:	addCycles(4);	AM_Absolute(A() & X());					break;

	case 0x90:	addCycles(2);	Branch(!(P() & CF));					break;
	case 0x91:	addCycles(6);	AM_IndirectIndexedY(A());				break;
	case 0x92:	addCycles(0);											break;
	case 0x93:	addCycles(0);											break;
	case 0x94:	addCycles(4);	AM_ZeroPageX(Y());						break;
	case 0x95:	addCycles(4);	AM_ZeroPageX(A());						break;
	case 0x96:	addCycles(4);	AM_ZeroPageY(X());						break;
	case 0x97:	addCycles(4);	AM_ZeroPageY(A() & X());				break;
	case 0x98:	addCycles(2);	adjustNZ(A() = Y());					break;
	case 0x99:	addCycles(5);	AM_AbsoluteY(A());						break;
	case 0x9a:	addCycles(2);	S() = X();								break;
	case 0x9b:	addCycles(0);											break;
	case 0x9c:	addCycles(0);											break;
	case 0x9d:	addCycles(5);	AM_AbsoluteX(A());						break;
	case 0x9e:	addCycles(0);											break;
	case 0x9f:	addCycles(0);											break;

	case 0xa0:	addCycles(2);	adjustNZ(Y() = AM_Immediate());			break;
	case 0xa1:	addCycles(6);	adjustNZ(A() = AM_IndexedIndirectX());	break;
	case 0xa2:	addCycles(2);	adjustNZ(X() = AM_Immediate());			break;
	case 0xa3:	addCycles(6);	LAX(AM_IndexedIndirectX());				break;
	case 0xa4:	addCycles(3);	adjustNZ(Y() = AM_ZeroPage());			break;
	case 0xa5:	addCycles(3);	adjustNZ(A() = AM_ZeroPage());			break;
	case 0xa6:	addCycles(3);	adjustNZ(X() = AM_ZeroPage());			break;
	case 0xa7:	addCycles(3);	LAX(AM_ZeroPage());						break;
	case 0xa8:	addCycles(2);	adjustNZ(Y() = A());					break;
	case 0xa9:	addCycles(2);	adjustNZ(A() = AM_Immediate());			break;
	case 0xaa:	addCycles(2);	adjustNZ(X() = A());					break;
	case 0xab:	addCycles(2);	ATX(AM_Immediate());					break;
	case 0xac:	addCycles(4);	adjustNZ(Y() = AM_Absolute());			break;
	case 0xad:	addCycles(4);	adjustNZ(A() = AM_Absolute());			break;
	case 0xae:	addCycles(4);	adjustNZ(X() = AM_Absolute());			break;
	case 0xaf:	addCycles(4);	LAX(AM_Absolute());						break;

	case 0xb0:	addCycles(2);	Branch(!!(P() & CF));					break;
	case 0xb1:	addCycles(5);	adjustNZ(A() = AM_IndirectIndexedY());	break;
	case 0xb2:	addCycles(0);											break;
	case 0xb3:	addCycles(5);	LAX(AM_IndirectIndexedY());				break;
	case 0xb4:	addCycles(4);	adjustNZ(Y() = AM_ZeroPageX());			break;
	case 0xb5:	addCycles(4);	adjustNZ(A() = AM_ZeroPageX());			break;
	case 0xb6:	addCycles(4);	adjustNZ(X() = AM_ZeroPageY());			break;
	case 0xb7:	addCycles(4);	LAX(AM_ZeroPageY());					break;
	case 0xb8:	addCycles(2);	clearFlag(P(), VF);						break;
	case 0xb9:	addCycles(4);	adjustNZ(A() = AM_AbsoluteY());			break;
	case 0xba:	addCycles(2);	adjustNZ(X() = S());					break;
	case 0xbb:	addCycles(0);											break;
	case 0xbc:	addCycles(4);	adjustNZ(Y() = AM_AbsoluteX());			break;
	case 0xbd:	addCycles(4);	adjustNZ(A() = AM_AbsoluteX());			break;
	case 0xbe:	addCycles(4);	adjustNZ(X() = AM_AbsoluteY());			break;
	case 0xbf:	addCycles(4);	LAX(AM_AbsoluteY());					break;

	case 0xc0:	addCycles(2);	CMP(Y(), AM_Immediate());				break;
	case 0xc1:	addCycles(6);	CMP(A(), AM_IndexedIndirectX());		break;
	case 0xc2:	addCycles(2);	AM_Immediate();							break;
	case 0xc3:	addCycles(8);	DCP(AM_IndexedIndirectX());				break;
	case 0xc4:	addCycles(3);	CMP(Y(), AM_ZeroPage());				break;
	case 0xc5:	addCycles(3);	CMP(A(), AM_ZeroPage());				break;
	case 0xc6:	addCycles(5);	BUS().write(DEC(AM_ZeroPage()));		break;
	case 0xc7:	addCycles(5);	DCP(AM_ZeroPage());						break;
	case 0xc8:	addCycles(2);	adjustNZ(++Y());						break;
	case 0xc9:	addCycles(2);	CMP(A(), AM_Immediate());				break;
	case 0xca:	addCycles(2);	adjustNZ(--X());						break;
	case 0xcb:	addCycles(2);	AXS(AM_Immediate());					break;
	case 0xcc:	addCycles(4);	CMP(Y(), AM_Absolute());				break;
	case 0xcd:	addCycles(4);	CMP(A(), AM_Absolute());				break;
	case 0xce:	addCycles(6);	BUS().write(DEC(AM_Absolute()));		break;
	case 0xcf:	addCycles(6);	DCP(AM_Absolute());						break;

	case 0xd0:	addCycles(2);	Branch(!(P() & ZF));					break;
	case 0xd1:	addCycles(5);	CMP(A(), AM_IndirectIndexedY());		break;
	case 0xd2:	addCycles(0);											break;
	case 0xd3:	addCycles(7);	DCP(AM_IndirectIndexedY());				break;
	case 0xd4:	addCycles(4);	AM_ZeroPageX();							break;
	case 0xd5:	addCycles(4);	CMP(A(), AM_ZeroPageX());				break;
	case 0xd6:	addCycles(6);	BUS().write(DEC(AM_ZeroPageX()));		break;
	case 0xd7:	addCycles(6);	DCP(AM_ZeroPageX());					break;
	case 0xd8:	addCycles(2);	clearFlag(P(), DF);						break;
	case 0xd9:	addCycles(4);	CMP(A(), AM_AbsoluteY());				break;
	case 0xda:	addCycles(2);											break;
	case 0xdb:	addCycles(6);	DCP(AM_AbsoluteY());					break;
	case 0xdc:	addCycles(4);	AM_AbsoluteX();							break;
	case 0xdd:	addCycles(4);	CMP(A(), AM_AbsoluteX());				break;
	case 0xde:	addCycles(7);	BUS().write(DEC(AM_AbsoluteX()));		break;
	case 0xdf:	addCycles(6);	DCP(AM_AbsoluteX());					break;

	case 0xe0:	addCycles(2);	CMP(X(), AM_Immediate());				break;
	case 0xe1:	addCycles(6);	A() = SBC(A(), AM_IndexedIndirectX());	break;
	case 0xe2:	addCycles(2);	AM_Immediate();							break;
	case 0xe3:	addCycles(8);	ISB(AM_IndexedIndirectX());				break;
	case 0xe4:	addCycles(3);	CMP(X(), AM_ZeroPage());				break;
	case 0xe5:	addCycles(3);	A() = SBC(A(), AM_ZeroPage());			break;
	case 0xe6:	addCycles(5);	BUS().write(INC(AM_ZeroPage()));		break;
	case 0xe7:	addCycles(5);	ISB(AM_ZeroPage());						break;
	case 0xe8:	addCycles(2);	adjustNZ(++X());						break;
	case 0xe9:	addCycles(2);	A() = SBC(A(), AM_Immediate());			break;
	case 0xea:	addCycles(2);											break;
	case 0xeb:	addCycles(2);	A() = SBC(A(), AM_Immediate());			break;
	case 0xec:	addCycles(4);	CMP(X(), AM_Absolute());				break;
	case 0xed:	addCycles(4);	A() = SBC(A(), AM_Absolute());			break;
	case 0xee:	addCycles(6);	BUS().write(INC(AM_Absolute()));		break;
	case 0xef:	addCycles(6);	ISB(AM_Absolute());						break;

	case 0xf0:	addCycles(2);	Branch(!!(P() & ZF));					break;
	case 0xf1:	addCycles(5);	A() = SBC(A(), AM_IndirectIndexedY());	break;
	case 0xf2:	addCycles(0);											break;
	case 0xf3:	addCycles(7);	ISB(AM_IndirectIndexedY());				break;
	case 0xf4:	addCycles(4);	AM_ZeroPageX();							break;
	case 0xf5:	addCycles(4);	A() = SBC(A(), AM_ZeroPageX());			break;
	case 0xf6:	addCycles(6);	BUS().write(INC(AM_ZeroPageX()));		break;
	case 0xf7:	addCycles(6);	ISB(AM_ZeroPageX());					break;
	case 0xf8:	addCycles(2);	setFlag(P(), DF);						break;
	case 0xf9:	addCycles(4);	A() = SBC(A(), AM_AbsoluteY());			break;
	case 0xfa:	addCycles(2);											break;
	case 0xfb:	addCycles(6);	ISB(AM_AbsoluteY());					break;
	case 0xfc:	addCycles(4);	AM_AbsoluteX();							break;
	case 0xfd:	addCycles(4);	A() = SBC(A(), AM_AbsoluteX());			break;
	case 0xfe:	addCycles(7);	BUS().write(INC(AM_AbsoluteX()));		break;
	case 0xff:	addCycles(6);	ISB(AM_AbsoluteX());					break;
	}

	ASSUME(cycles() > 0);
	return cycles();
}

////

void EightBit::MOS6502::push(uint8_t value) {
	setBytePaged(1, S()--, value);
}

uint8_t EightBit::MOS6502::pop() {
	return getBytePaged(1, ++S());
}

////

uint8_t EightBit::MOS6502::ROR(uint8_t value) {
	const auto carry = P() & CF;
	setFlag(P(), CF, value & CF);
	value = (value >> 1) | (carry << 7);
	adjustNZ(value);
	return value;
}

uint8_t EightBit::MOS6502::LSR(uint8_t value) {
	setFlag(P(), CF, value & CF);
	adjustNZ(value >>= 1);
	return value;
}

void EightBit::MOS6502::BIT(uint8_t data) {
	adjustZero(A() & data);
	adjustNegative(data);
	setFlag(P(), VF, data & VF);
}

uint8_t EightBit::MOS6502::ROL(uint8_t value) {
	const uint8_t result = (value << 1) | (P() & CF);
	setFlag(P(), CF, value & Bit7);
	adjustNZ(result);
	return result;
}

uint8_t EightBit::MOS6502::ASL(uint8_t value) {
	setFlag(P(), CF, (value & Bit7) >> 7);
	adjustNZ(value <<= 1);
	return value;
}

uint8_t EightBit::MOS6502::SBC(const uint8_t operand, const uint8_t data) {

	const auto returned = SUB(operand, data, ~P() & CF);

	const register16_t& difference = m_intermediate;
	adjustNZ(difference.low);
	setFlag(P(), VF, (operand ^ data) & (operand ^ difference.low) & NF);
	clearFlag(P(), CF, difference.high);

	return returned;
}

uint8_t EightBit::MOS6502::SUB(const uint8_t operand, const uint8_t data, const int borrow) {
	return P() & DF ? SUB_d(operand, data, borrow) : SUB_b(operand, data, borrow);
}

uint8_t EightBit::MOS6502::SUB_b(const uint8_t operand, const uint8_t data, const int borrow) {
	m_intermediate.word = operand - data - borrow;
	return m_intermediate.low;
}

uint8_t EightBit::MOS6502::SUB_d(const uint8_t operand, const uint8_t data, const int borrow) {
	m_intermediate.word = operand - data - borrow;

	uint8_t low = lowNibble(operand) - lowNibble(data) - borrow;
	const auto lowNegative = low & NF;
	if (lowNegative)
		low -= 6;

	uint8_t high = highNibble(operand) - highNibble(data) - (lowNegative >> 7);
	const auto highNegative = high & NF;
	if (highNegative)
		high -= 6;

	return promoteNibble(high) | lowNibble(low);
}

void EightBit::MOS6502::CMP(uint8_t first, uint8_t second) {
	const register16_t result = first - second;
	adjustNZ(result.low);
	clearFlag(P(), CF, result.high);
}

uint8_t EightBit::MOS6502::ADC(const uint8_t operand, const uint8_t data) {
	const auto returned = ADD(operand, data, P() & CF);
	adjustNZ(m_intermediate.low);
	return returned;
}

uint8_t EightBit::MOS6502::ADD(uint8_t operand, uint8_t data, int carry) {
	return P() & DF ? ADD_d(operand, data, carry) : ADD_b(operand, data, carry);
}

uint8_t EightBit::MOS6502::ADD_b(uint8_t operand, uint8_t data, int carry) {
	m_intermediate.word = operand + data + carry;

	setFlag(P(), VF, ~(operand ^ data) & (operand ^ m_intermediate.low) & NF);
	setFlag(P(), CF, m_intermediate.high & CF);

	return m_intermediate.low;
}

uint8_t EightBit::MOS6502::ADD_d(uint8_t operand, uint8_t data, int carry) {

	m_intermediate.word = operand + data + carry;

	uint8_t low = lowNibble(operand) + lowNibble(data) + carry;
	if (low > 9)
		low += 6;

	uint8_t high = highNibble(operand) + highNibble(data) + (low > 0xf ? 1 : 0);
	setFlag(P(), VF, ~(operand ^ data) & (operand ^ promoteNibble(high)) & NF);

	if (high > 9)
		high += 6;

	setFlag(P(), CF, high > 0xf);

	return promoteNibble(high) | lowNibble(low);
}

////

void EightBit::MOS6502::Branch(int8_t displacement) {
	const auto page = PC().high;
	PC() += displacement;
	if (UNLIKELY(PC().high != page))
		addCycle();
	addCycle();
}

void EightBit::MOS6502::Branch(bool flag) {
	const int8_t displacement = AM_Immediate();
	if (UNLIKELY(flag))
		Branch(displacement);
}

//

void EightBit::MOS6502::PHP() {
	push(P() | BF);
}

void EightBit::MOS6502::PLP() {
	P() = (pop() | RF) & ~BF;
}

//

void EightBit::MOS6502::JSR_abs() {
	const auto address = Address_Absolute();
	--PC();
	call(address);
}

void EightBit::MOS6502::RTI() {
	PLP();
	ret();
}

void EightBit::MOS6502::RTS() {
	ret();
	++PC();
}

void EightBit::MOS6502::JMP_abs() {
	jump(Address_Absolute());
}

void EightBit::MOS6502::JMP_ind() {
	jump(Address_Indirect());
}

void EightBit::MOS6502::BRK() {
	pushWord(++PC());
	PHP();
	setFlag(P(), IF);
	jump(getWordPaged(0xff, IRQvector));
}
