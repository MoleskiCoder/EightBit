#include "stdafx.h"
#include "mos6502.h"

EightBit::MOS6502::MOS6502(Bus& bus)
: LittleEndianProcessor(bus) {}

void EightBit::MOS6502::powerOn() {

	LittleEndianProcessor::powerOn();

	X() = Bit7;
	Y() = 0;
	A() = 0;
	P() = RF;
	S() = Mask8;

	raise(NMI());
	raise(SO());
}

int EightBit::MOS6502::step() {
	resetCycles();
	ExecutingInstruction.fire(*this);
	if (LIKELY(powered())) {
		if (UNLIKELY(lowered(SO())))
			handleSO();
		if (UNLIKELY(lowered(HALT())))
			handleHALT();
		else if (UNLIKELY(lowered(RESET())))
			handleRESET();
		else if (UNLIKELY(lowered(NMI())))
			handleNMI();
		else if (UNLIKELY(lowered(IRQ()) && !interruptMasked()))
			handleIRQ();
		else
			execute(fetchByte());
	}
	ExecutedInstruction.fire(*this);
	return cycles();
}

// Interrupt (etc.) handlers

void EightBit::MOS6502::handleSO() {
	raise(SO());
	P() |= VF;
}

void EightBit::MOS6502::handleHALT() {
	execute(0xea);	// NOP
	addCycles(2);
}

void EightBit::MOS6502::handleRESET() {
	LittleEndianProcessor::handleRESET();
	jump(getWordPaged(0xff, RSTvector));
	addCycles(4);	// ?? TBC
}


void EightBit::MOS6502::handleNMI() {
	raise(NMI());
	interrupt(NMIvector);
	addCycles(4);	// ?? TBC
}

void EightBit::MOS6502::handleIRQ() {
	LittleEndianProcessor::handleIRQ();
	interrupt(IRQvector);
	addCycles(4);	// ?? TBC
}

void EightBit::MOS6502::interrupt(uint8_t vector) {
	raise(HALT());
	pushWord(PC());
	push(P());
	setFlag(P(), IF);	// Disable IRQ
	jump(getWordPaged(0xff, vector));
}

//

int EightBit::MOS6502::execute(const uint8_t opcode) { 

	switch (opcode) {

	case 0x00:	addCycles(7);	brk();													break;	// BRK
	case 0x01:	addCycles(6);	A() = orr(A(), AM_IndexedIndirectX());					break;	// ORA (indexed indirect X)
	case 0x02:																			break;
	case 0x03:																			break;
	case 0x04:																			break;
	case 0x05:	addCycles(3);	A() = orr(A(), AM_ZeroPage());							break;	// ORA (zero page)
	case 0x06:	addCycles(5);	BUS().write(asl(AM_ZeroPage()));						break;	// ASL (zero page)
	case 0x07:																			break;
	case 0x08:	addCycles(3);	php();													break;	// PHP
	case 0x09:	addCycles(2);	A() = orr(A(), AM_Immediate());							break;	// ORA (immediate)
	case 0x0a:	addCycles(2);	A() = asl(A());											break;	// ASL A
	case 0x0b:																			break;
	case 0x0c:																			break;
	case 0x0d:	addCycles(4);	A() = orr(A(), AM_Absolute());							break;	// ORA (absolute)
	case 0x0e:	addCycles(6);	BUS().write(asl(AM_Absolute()));						break;	// ASL (absolute)
	case 0x0f:																			break;

	case 0x10:	addCycles(2);	branch(!negative());									break;	// BPL (relative)
	case 0x11:	addCycles(5);	A() = orr(A(), AM_IndirectIndexedY());					break;	// ORA (indirect indexed Y)
	case 0x12:																			break;
	case 0x13:																			break;
	case 0x14:																			break;
	case 0x15:	addCycles(4);	A() = orr(A(), AM_ZeroPageX());							break;	// ORA (zero page, X)
	case 0x16:	addCycles(6);	BUS().write(asl(AM_ZeroPageX()));						break;	// ASL (zero page, X)
	case 0x17:																			break;
	case 0x18:	addCycles(2);	clearFlag(P(), CF);										break;	// CLC
	case 0x19:	addCycles(4);	A() = orr(A(), AM_AbsoluteY());							break;	// ORA (absolute, Y)
	case 0x1a:																			break;
	case 0x1b:																			break;
	case 0x1c:																			break;
	case 0x1d:	addCycles(4);	A() = orr(A(), AM_AbsoluteX());							break;	// ORA (absolute, X)
	case 0x1e:	addCycles(7);	BUS().write(asl(AM_AbsoluteX()));						break;	// ASL (absolute, X)
	case 0x1f:																			break;

	case 0x20:	addCycles(6);	jsr(Address_Absolute());								break;	// JSR (absolute)
	case 0x21:	addCycles(6);	A() = andr(A(), AM_IndexedIndirectX());					break;	// AND (indexed indirect X)
	case 0x22:																			break;
	case 0x23:																			break;
	case 0x24:	addCycles(3);	bit(A(), AM_ZeroPage());								break;	// BIT (zero page)
	case 0x25:	addCycles(3);	A() = andr(A(), AM_ZeroPage());							break;	// AND (zero page)
	case 0x26:	addCycles(5);	BUS().write(rol(AM_ZeroPage()));						break;	// ROL (zero page)
	case 0x27:																			break;
	case 0x28:	addCycles(4);	plp();													break;	// PLP
	case 0x29:	addCycles(2);	A() = andr(A(), AM_Immediate());						break;	// AND (immediate)
	case 0x2a:	addCycles(2);	A() = rol(A());											break;	// ROL A
	case 0x2b:																			break;
	case 0x2c:	addCycles(4);	bit(A(), AM_Absolute());								break;	// BIT (absolute)
	case 0x2d:	addCycles(4);	A() = andr(A(), AM_Absolute());							break;	// AND (absolute)
	case 0x2e:	addCycles(6);	BUS().write(rol(AM_Absolute()));						break;	// ROL (absolute)
	case 0x2f:																			break;

	case 0x30:	addCycles(2);	branch(negative());										break;	// BMI
	case 0x31:	addCycles(5);	A() = andr(A(), AM_IndirectIndexedY());					break;	// AND (indirect indexed Y)
	case 0x32:																			break;
	case 0x33:																			break;
	case 0x34:																			break;
	case 0x35:	addCycles(4);	A() = andr(A(), AM_ZeroPageX());						break;	// AND (zero page, X)
	case 0x36:	addCycles(6);	BUS().write(rol(AM_ZeroPageX()));						break;	// ROL (zero page, X)
	case 0x37:																			break;
	case 0x38:	addCycles(2);	setFlag(P(), CF);										break;	// SEC
	case 0x39:	addCycles(4);	A() = andr(A(), AM_AbsoluteY());						break;	// AND (absolute, Y)
	case 0x3a:																			break;
	case 0x3b:																			break;
	case 0x3c:																			break;
	case 0x3d:	addCycles(4);	A() = andr(A(), AM_AbsoluteX());						break;	// AND (absolute, X)
	case 0x3e:	addCycles(7);	BUS().write(rol(AM_AbsoluteX()));						break;	// ROL (absolute, X)
	case 0x3f:																			break;

	case 0x40:	addCycles(6);	rti();													break;	// RTI
	case 0x41:	addCycles(6);	A() = eorr(A(), AM_IndexedIndirectX());					break;	// EOR (indexed indirect X)
	case 0x42:																			break;
	case 0x43:																			break;
	case 0x44:																			break;
	case 0x45:	addCycles(3);	A() = eorr(A(), AM_ZeroPage());							break;	// EOR (zero page)
	case 0x46:	addCycles(5);	BUS().write(lsr(AM_ZeroPage()));						break;	// LSR (zero page)
	case 0x47:																			break;
	case 0x48:	addCycles(3);	push(A());												break;	// PHA
	case 0x49:	addCycles(2);	A() = eorr(A(), AM_Immediate());						break;	// EOR (immediate)
	case 0x4a:	addCycles(2);	A() = lsr(A());											break;	// LSR A
	case 0x4b:																			break;
	case 0x4c:	addCycles(3);	jump(Address_Absolute());								break;	// JMP (absolute)
	case 0x4d:	addCycles(4);	A() = eorr(A(), AM_Absolute());							break;	// EOR (absolute)
	case 0x4e:	addCycles(6);	BUS().write(lsr(AM_Absolute()));						break;	// LSR (absolute)
	case 0x4f:																			break;

	case 0x50:	addCycles(2);	branch(!overflow());									break;	// BVC (relative)
	case 0x51:	addCycles(5);	A() = eorr(A(), AM_IndirectIndexedY());					break;	// EOR (indirect indexed Y)
	case 0x52:																			break;
	case 0x53:																			break;
	case 0x54:																			break;
	case 0x55:	addCycles(4);	A() = eorr(A(), AM_ZeroPageX());						break;	// EOR (zero page, X)
	case 0x56:	addCycles(6);	BUS().write(lsr(AM_ZeroPageX()));						break;	// LSR (zero page, X)
	case 0x57:																			break;
	case 0x58:	addCycles(2);	clearFlag(P(), IF);										break;	// CLI
	case 0x59:	addCycles(4);	A() = eorr(A(), AM_AbsoluteY());						break;	// EOR (absolute, Y)
	case 0x5a:																			break;
	case 0x5b:																			break;
	case 0x5c:																			break;
	case 0x5d:	addCycles(4);	A() = eorr(A(), AM_AbsoluteX());						break;	// EOR (absolute, X)
	case 0x5e:	addCycles(7);	BUS().write(lsr(AM_AbsoluteX()));						break;	// LSR (absolute, X)
	case 0x5f:																			break;

	case 0x60:	addCycles(6);	rts();													break;	// RTS
	case 0x61:	addCycles(6);	A() = adc(A(), AM_IndexedIndirectX());					break;	// ADC (indexed indirect X)
	case 0x62:																			break;
	case 0x63:																			break;
	case 0x64:																			break;
	case 0x65:	addCycles(3);	A() = adc(A(), AM_ZeroPage());							break;	// ADC (zero page)
	case 0x66:	addCycles(5);	BUS().write(ror(AM_ZeroPage()));						break;	// ROR (zero page)
	case 0x67:																			break;
	case 0x68:	addCycles(4);	A() = through(pop());									break;	// PLA
	case 0x69:	addCycles(2);	A() = adc(A(), AM_Immediate());							break;	// ADC (immediate)
	case 0x6a:	addCycles(2);	A() = ror(A());											break;	// ROR A
	case 0x6b:																			break;
	case 0x6c:	addCycles(5);	jump(Address_Indirect());								break;	// JMP (indirect)
	case 0x6d:	addCycles(4);	A() = adc(A(), AM_Absolute());							break;	// ADC (absolute)
	case 0x6e:	addCycles(6);	BUS().write(ror(AM_Absolute()));						break;	// ROR (absolute)
	case 0x6f:																			break;

	case 0x70:	addCycles(2);	branch(overflow());										break;	// BVS (relative)
	case 0x71:	addCycles(5);	A() = adc(A(), AM_IndirectIndexedY());					break;	// ADC (indirect indexed Y)
	case 0x72:																			break;
	case 0x73:																			break;
	case 0x74:																			break;
	case 0x75:	addCycles(4);	A() = adc(A(), AM_ZeroPageX());							break;	// ADC (zero page, X)
	case 0x76:	addCycles(6);	BUS().write(ror(AM_ZeroPageX()));						break;	// ROR (zero page, X)
	case 0x77:																			break;
	case 0x78:	addCycles(2);	setFlag(P(), IF);										break;	// SEI
	case 0x79:	addCycles(4);	A() = adc(A(), AM_AbsoluteY());							break;	// ADC (absolute, Y)
	case 0x7a:																			break;
	case 0x7b:																			break;
	case 0x7c:																			break;
	case 0x7d:	addCycles(4);	A() = adc(A(), AM_AbsoluteX());							break;	// ADC (absolute, X)
	case 0x7e:	addCycles(7);	BUS().write(ror(AM_AbsoluteX()));						break;	// ROR (absolute, X)
	case 0x7f:																			break;

	case 0x80:																			break;
	case 0x81:	addCycles(6);	BUS().write(Address_IndexedIndirectX(), A());			break;	// STA (indexed indirect X)
	case 0x82:																			break;
	case 0x83:																			break;
	case 0x84:	addCycles(3);	BUS().write(Address_ZeroPage(), Y());					break;	// STY (zero page)
	case 0x85:	addCycles(3);	BUS().write(Address_ZeroPage(), A());					break;	// STA (zero page)
	case 0x86:	addCycles(3);	BUS().write(Address_ZeroPage(), X());					break;	// STX (zero page)
	case 0x87:																			break;
	case 0x88:	addCycles(2);	Y() = dec(Y());											break;	// DEY
	case 0x89:																			break;
	case 0x8a:	addCycles(2);	A() = through(X());										break;	// TXA
	case 0x8b:																			break;
	case 0x8c:	addCycles(4);	BUS().write(Address_Absolute(), Y());					break;	// STY (absolute)
	case 0x8d:	addCycles(4);	BUS().write(Address_Absolute(), A());					break;	// STA (absolute)
	case 0x8e:	addCycles(4);	BUS().write(Address_Absolute(), X());					break;	// STX (absolute)
	case 0x8f:																			break;

	case 0x90:	addCycles(2);	branch(!carry());										break;	// BCC
	case 0x91:	addCycles(6);	BUS().write(Address_IndirectIndexedY().first, A());		break;	// STA (indirect indexed Y)
	case 0x92:																			break;
	case 0x93:																			break;
	case 0x94:	addCycles(4);	BUS().write(Address_ZeroPageX(), Y());					break;	// STY (zero page, X)
	case 0x95:	addCycles(4);	BUS().write(Address_ZeroPageX(), A());					break;	// STA (zero page, X)
	case 0x96:	addCycles(4);	BUS().write(Address_ZeroPageY(), X());					break;	// STX (zero page, X)
	case 0x97:																			break;
	case 0x98:	addCycles(2);	A() = through(Y());										break;	// TYA
	case 0x99:	addCycles(5);	BUS().write(Address_AbsoluteY().first, A());			break;	// STA (absolute, Y)
	case 0x9a:	addCycles(2);	S() = X();												break;	// TXS
	case 0x9b:																			break;
	case 0x9c:																			break;
	case 0x9d:	addCycles(5);	BUS().write(Address_AbsoluteX().first, A());			break;	// STA (absolute, X)
	case 0x9e:																			break;
	case 0x9f:																			break;

	case 0xa0:	addCycles(2);	Y() = through(AM_Immediate());							break;	// LDY (immediate)
	case 0xa1:	addCycles(6);	A() = through(AM_IndexedIndirectX());					break;	// LDA (indexed indirect X)
	case 0xa2:	addCycles(2);	X() = through(AM_Immediate());							break;	// LDX (immediate)
	case 0xa3:																			break;
	case 0xa4:	addCycles(3);	Y() = through(AM_ZeroPage());							break;	// LDY (zero page)
	case 0xa5:	addCycles(3);	A() = through(AM_ZeroPage());							break;	// LDA (zero page)
	case 0xa6:	addCycles(3);	X() = through(AM_ZeroPage());							break;	// LDX (zero page)
	case 0xa7:																			break;
	case 0xa8:	addCycles(2);	Y() = through(A());										break;	// TAY
	case 0xa9:	addCycles(2);	A() = through(AM_Immediate());							break;	// LDA (immediate)
	case 0xaa:	addCycles(2);	X() = through(A());										break;	// TAX
	case 0xab:																			break;
	case 0xac:	addCycles(4);	Y() = through(AM_Absolute());							break;	// LDY (absolute)
	case 0xad:	addCycles(4);	A() = through(AM_Absolute());							break;	// LDA (absolute)
	case 0xae:	addCycles(4);	X() = through(AM_Absolute());							break;	// LDX (absolute)
	case 0xaf:																			break;

	case 0xb0:	addCycles(2);	branch(carry());										break;	// BCS
	case 0xb1:	addCycles(5);	A() = through(AM_IndirectIndexedY());					break;	// LDA (indirect indexed Y)
	case 0xb2:																			break;
	case 0xb3:																			break;
	case 0xb4:	addCycles(4);	Y() = through(AM_ZeroPageX());							break;	// LDY (zero page, X)
	case 0xb5:	addCycles(4);	A() = through(AM_ZeroPageX());							break;	// LDA (zero page, X)
	case 0xb6:	addCycles(4);	X() = through(AM_ZeroPageY());							break;	// LDX (zero page, Y)
	case 0xb7:																			break;
	case 0xb8:	addCycles(2);	clearFlag(P(), VF);										break;	// CLV
	case 0xb9:	addCycles(4);	A() = through(AM_AbsoluteY());							break;	// LDA (absolute, Y)
	case 0xba:	addCycles(2);	X() = through(S());										break;	// TSX
	case 0xbb:																			break;
	case 0xbc:	addCycles(4);	Y() = through(AM_AbsoluteX());							break;	// LDY (absolute, X)
	case 0xbd:	addCycles(4);	A() = through(AM_AbsoluteX());							break;	// LDA (absolute, X)
	case 0xbe:	addCycles(4);	X() = through(AM_AbsoluteY());							break;	// LDX (absolute, Y)
	case 0xbf:																			break;

	case 0xc0:	addCycles(2);	cmp(Y(), AM_Immediate());								break;	// CPY (immediate)
	case 0xc1:	addCycles(6);	cmp(A(), AM_IndexedIndirectX());						break;	// CMP (indexed indirect X)
	case 0xc2:																			break;
	case 0xc3:																			break;
	case 0xc4:	addCycles(3);	cmp(Y(), AM_ZeroPage());								break;	// CPY (zero page)
	case 0xc5:	addCycles(3);	cmp(A(), AM_ZeroPage());								break;	// CMP (zero page)
	case 0xc6:	addCycles(5);	BUS().write(dec(AM_ZeroPage()));						break;	// DEC (zero page)
	case 0xc7:																			break;
	case 0xc8:	addCycles(2);	Y() = inc(Y());											break;	// INY
	case 0xc9:	addCycles(2);	cmp(A(), AM_Immediate());								break;	// CMP (immediate)
	case 0xca:	addCycles(2);	X() = dec(X());											break;	// DEX
	case 0xcb:																			break;
	case 0xcc:	addCycles(4);	cmp(Y(), AM_Absolute());								break;	// CPY (absolute)
	case 0xcd:	addCycles(4);	cmp(A(), AM_Absolute());								break;	// CMP (absolute)
	case 0xce:	addCycles(6);	BUS().write(dec(AM_Absolute()));						break;	// DEC (absolute)
	case 0xcf:																			break;

	case 0xd0:	addCycles(2);	branch(!zero());										break;	// BNE
	case 0xd1:	addCycles(5);	cmp(A(), AM_IndirectIndexedY());						break;	// CMP (indirect indexed Y)
	case 0xd2:																			break;
	case 0xd3:																			break;
	case 0xd4:																			break;
	case 0xd5:	addCycles(4);	cmp(A(), AM_ZeroPageX());								break;	// CMP (zero page, X)
	case 0xd6:	addCycles(6);	BUS().write(dec(AM_ZeroPageX()));						break;	// DEC (zero page, X)
	case 0xd7:																			break;
	case 0xd8:	addCycles(2);	clearFlag(P(), DF);										break;	// CLD
	case 0xd9:	addCycles(4);	cmp(A(), AM_AbsoluteY());								break;	// CMP (absolute, Y)
	case 0xda:																			break;
	case 0xdb:																			break;
	case 0xdc:																			break;
	case 0xdd:	addCycles(4);	cmp(A(), AM_AbsoluteX());								break;	// CMP (absolute, X)
	case 0xde:	addCycles(7);	BUS().write(dec(AM_AbsoluteX()));						break;	// DEC (absolute, X)
	case 0xdf:																			break;

	case 0xe0:	addCycles(2);	cmp(X(), AM_Immediate());								break;	// CPX (immediate)
	case 0xe1:	addCycles(6);	A() = sbc(A(), AM_IndexedIndirectX());					break;	// SBC (indexed indirect X)
	case 0xe2:																			break;
	case 0xe3:																			break;
	case 0xe4:	addCycles(3);	cmp(X(), AM_ZeroPage());								break;	// CPX (zero page)
	case 0xe5:	addCycles(3);	A() = sbc(A(), AM_ZeroPage());							break;	// SBC (zero page)
	case 0xe6:	addCycles(5);	BUS().write(inc(AM_ZeroPage()));						break;	// INC (zero page)
	case 0xe7:																			break;
	case 0xe8:	addCycles(2);	X() = inc(X());											break;	// INX
	case 0xe9:	addCycles(2);	A() = sbc(A(), AM_Immediate());							break;	// SBC (immediate)
	case 0xea:																			break;
	case 0xeb:																			break;
	case 0xec:	addCycles(4);	cmp(X(), AM_Absolute());								break;	// CPX (absolute)
	case 0xed:	addCycles(4);	A() = sbc(A(), AM_Absolute());							break;	// SBC (absolute)
	case 0xee:	addCycles(6);	BUS().write(inc(AM_Absolute()));						break;	// INC (absolute)
	case 0xef:																			break;

	case 0xf0:	addCycles(2);	branch(zero());											break;	// BEQ
	case 0xf1:	addCycles(5);	A() = sbc(A(), AM_IndirectIndexedY());					break;	// SBC (indirect indexed Y)
	case 0xf2:																			break;
	case 0xf3:																			break;
	case 0xf4:																			break;
	case 0xf5:	addCycles(4);	A() = sbc(A(), AM_ZeroPageX());							break;	// SBC (zero page, X)
	case 0xf6:	addCycles(6);	BUS().write(inc(AM_ZeroPageX()));						break;	// INC (zero page, X)
	case 0xf7:																			break;
	case 0xf8:	addCycles(2);	setFlag(P(), DF);										break;	// SED
	case 0xf9:	addCycles(4);	A() = sbc(A(), AM_AbsoluteY());							break;	// SBC (absolute, Y)
	case 0xfa:																			break;
	case 0xfb:																			break;
	case 0xfc:																			break;
	case 0xfd:	addCycles(4);	A() = sbc(A(), AM_AbsoluteX());							break;	// SBC (absolute, X)
	case 0xfe:	addCycles(7);	BUS().write(inc(AM_AbsoluteX()));						break;	// INC (absolute, X)
	case 0xff:																			break;
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

EightBit::register16_t EightBit::MOS6502::Address_Absolute() {
	return fetchWord();
}

uint8_t EightBit::MOS6502::Address_ZeroPage() {
	return fetchByte();
}

EightBit::register16_t EightBit::MOS6502::Address_ZeroPageIndirect() {
	return getWordPaged(0, Address_ZeroPage());
}

EightBit::register16_t EightBit::MOS6502::Address_Indirect() {
	const auto address = Address_Absolute();
	return getWordPaged(address.high, address.low);
}

uint8_t EightBit::MOS6502::Address_ZeroPageX() {
	return Address_ZeroPage() + X();
}

uint8_t EightBit::MOS6502::Address_ZeroPageY() {
	return Address_ZeroPage() + Y();
}

std::pair<EightBit::register16_t, bool> EightBit::MOS6502::Address_AbsoluteX() {
	auto address = Address_Absolute();
	const auto page = address.high;
	address += X();
	return { address, address.high != page };
}

std::pair<EightBit::register16_t, bool> EightBit::MOS6502::Address_AbsoluteY() {
	auto address = Address_Absolute();
	const auto page = address.high;
	address += Y();
	return { address, address.high != page };
}

EightBit::register16_t EightBit::MOS6502::Address_IndexedIndirectX() {
	return getWordPaged(0, Address_ZeroPageX());
}

std::pair<EightBit::register16_t, bool> EightBit::MOS6502::Address_IndirectIndexedY() {
	auto address = Address_ZeroPageIndirect();
	const auto page = address.high;
	address += Y();
	return { address, address.high != page };
}

EightBit::register16_t EightBit::MOS6502::Address_relative_byte() {
	return PC() + (int8_t)fetchByte();
}

// Addressing modes, read

uint8_t EightBit::MOS6502::AM_Immediate() {
	return fetchByte();
}

uint8_t EightBit::MOS6502::AM_Absolute() {
	return BUS().read(Address_Absolute());
}

uint8_t EightBit::MOS6502::AM_ZeroPage() {
	return BUS().read(Address_ZeroPage());
}

uint8_t EightBit::MOS6502::AM_AbsoluteX() {
	const auto [address, paged] = Address_AbsoluteX();
	if (UNLIKELY(paged))
		addCycle();
	return BUS().read(address);
}

uint8_t EightBit::MOS6502::AM_AbsoluteY() {
	const auto [address, paged] = Address_AbsoluteY();
	if (UNLIKELY(paged))
		addCycle();
	return BUS().read(address);
}

uint8_t EightBit::MOS6502::AM_ZeroPageX() {
	return BUS().read(Address_ZeroPageX());
}

uint8_t EightBit::MOS6502::AM_ZeroPageY() {
	return BUS().read(Address_ZeroPageY());
}

uint8_t EightBit::MOS6502::AM_IndexedIndirectX() {
	return BUS().read(Address_IndexedIndirectX());
}

uint8_t EightBit::MOS6502::AM_IndirectIndexedY() {
	const auto [address, paged] = Address_IndirectIndexedY();
	if (UNLIKELY(paged))
		addCycle();
	return BUS().read(address);
}

////

uint8_t EightBit::MOS6502::sbc(const uint8_t operand, const uint8_t data) {

	const auto returned = sub(operand, data, ~P() & CF);

	const auto difference = m_intermediate;
	adjustNZ(difference.low);
	setFlag(P(), VF, (operand ^ data) & (operand ^ difference.low) & NF);
	clearFlag(P(), CF, difference.high);

	return returned;
}

uint8_t EightBit::MOS6502::sub(const uint8_t operand, const uint8_t data, const int borrow) {
	return decimal() ? sub_d(operand, data, borrow) : sub_b(operand, data, borrow);
}

uint8_t EightBit::MOS6502::sub_b(const uint8_t operand, const uint8_t data, const int borrow) {
	m_intermediate.word = operand - data - borrow;
	return m_intermediate.low;
}

uint8_t EightBit::MOS6502::sub_d(const uint8_t operand, const uint8_t data, const int borrow) {
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

uint8_t EightBit::MOS6502::adc(const uint8_t operand, const uint8_t data) {
	const auto returned = add(operand, data, carry());
	adjustNZ(m_intermediate.low);
	return returned;
}

uint8_t EightBit::MOS6502::add(uint8_t operand, uint8_t data, int carry) {
	return decimal() ? add_d(operand, data, carry) : add_b(operand, data, carry);
}

uint8_t EightBit::MOS6502::add_b(uint8_t operand, uint8_t data, int carry) {
	m_intermediate.word = operand + data + carry;

	setFlag(P(), VF, ~(operand ^ data) & (operand ^ m_intermediate.low) & NF);
	setFlag(P(), CF, m_intermediate.high & CF);

	return m_intermediate.low;
}

uint8_t EightBit::MOS6502::add_d(uint8_t operand, uint8_t data, int carry) {

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

uint8_t EightBit::MOS6502::andr(const uint8_t operand, const uint8_t data) {
	return through(operand & data);
}

uint8_t EightBit::MOS6502::asl(const uint8_t value) {
	setFlag(P(), CF, (value & Bit7) >> 7);
	return through(value << 1);
}

void EightBit::MOS6502::bit(const uint8_t operand, const uint8_t data) {
	setFlag(P(), VF, data & VF);
	adjustZero(operand & data);
	adjustNegative(data);
}

void EightBit::MOS6502::brk() {
	pushWord(++PC());
	php();
	setFlag(P(), IF);
	jump(getWordPaged(0xff, IRQvector));
}

void EightBit::MOS6502::cmp(const uint8_t first, const uint8_t second) {
	const register16_t result = first - second;
	adjustNZ(result.low);
	clearFlag(P(), CF, result.high);
}

uint8_t EightBit::MOS6502::dec(const uint8_t value) {
	return through(value - 1);
}

uint8_t EightBit::MOS6502::eorr(const uint8_t operand, const uint8_t data) {
	return through(operand ^ data);
}

uint8_t EightBit::MOS6502::inc(const uint8_t value) {
	return through(value + 1);
}

void EightBit::MOS6502::jsr(const register16_t destination) {
	--PC();
	call(destination);
}

uint8_t EightBit::MOS6502::lsr(const uint8_t value) {
	setFlag(P(), CF, value & Bit0);
	return through(value >> 1);
}

uint8_t EightBit::MOS6502::orr(const uint8_t operand, const uint8_t data) {
	return through(operand | data);
}

void EightBit::MOS6502::php() {
	push(P() | BF);
}

void EightBit::MOS6502::plp() {
	P() = (pop() | RF) & ~BF;
}

uint8_t EightBit::MOS6502::rol(const uint8_t operand) {
	const auto carryIn = carry();
	setFlag(P(), CF, operand & Bit7);
	const uint8_t result = (operand << 1) | carryIn;
	return through(result);
}

uint8_t EightBit::MOS6502::ror(const uint8_t operand) {
	const auto carryIn = carry();
	setFlag(P(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | (carryIn << 7);
	return through(result);
}

void EightBit::MOS6502::rti() {
	plp();
	ret();
}

void EightBit::MOS6502::rts() {
	ret();
	++PC();
}
