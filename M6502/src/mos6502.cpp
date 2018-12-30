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
			Processor::execute(fetchByte());
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
	Processor::execute(0xea);	// NOP
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

void EightBit::MOS6502::busWrite() {
	addCycle();
	Processor::busWrite();
}

uint8_t EightBit::MOS6502::busRead() {
	addCycle();
	return Processor::busRead();
}

//

int EightBit::MOS6502::execute() { 

	switch (opcode()) {

	case 0x00:	brk();																	break;	// BRK
	case 0x01:	addCycle(); A() = orr(A(), AM_IndexedIndirectX());						break;	// ORA (indexed indirect X)
	case 0x02:																			break;
	case 0x03:	addCycles(2); slo(AM_IndexedIndirectX());								break;	// *SLO (indexed indirect X)
	case 0x04:	AM_ZeroPage();															break;	// *NOP (zero page)
	case 0x05:	A() = orr(A(), AM_ZeroPage());											break;	// ORA (zero page)
	case 0x06:	addCycle(); Processor::busWrite(asl(AM_ZeroPage()));					break;	// ASL (zero page)
	case 0x07:	addCycle(); slo(AM_ZeroPage());											break;	// *SLO (zero page)
	case 0x08:	addCycle(); php();														break;	// PHP
	case 0x09:	A() = orr(A(), AM_Immediate());											break;	// ORA (immediate)
	case 0x0a:	addCycle(); A() = asl(A());												break;	// ASL A
	case 0x0b:	addCycle(); anc(AM_Immediate());										break;	// *ANC (immediate)
	case 0x0c:	AM_Absolute();															break;	// *NOP (absolute)
	case 0x0d:	A() = orr(A(), AM_Absolute());											break;	// ORA (absolute)
	case 0x0e:	addCycle(); Processor::busWrite(asl(AM_Absolute()));					break;	// ASL (absolute)
	case 0x0f:	addCycle(); slo(AM_Absolute());											break;	// *SLO (absolute)

	case 0x10:	branch(!negative());													break;	// BPL (relative)
	case 0x11:	A() = orr(A(), AM_IndirectIndexedY());									break;	// ORA (indirect indexed Y)
	case 0x12:																			break;
	case 0x13:	addCycle(); slo(AM_IndirectIndexedY());									break;	// *SLO (indirect indexed Y)
	case 0x14:	addCycle(); AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x15:	addCycle(); A() = orr(A(), AM_ZeroPageX());								break;	// ORA (zero page, X)
	case 0x16:	addCycles(2); Processor::busWrite(asl(AM_ZeroPageX()));					break;	// ASL (zero page, X)
	case 0x17:	addCycles(2); slo(AM_ZeroPageX());										break;	// *SLO (zero page, X)
	case 0x18:	addCycle(); clearFlag(P(), CF);											break;	// CLC
	case 0x19:	A() = orr(A(), AM_AbsoluteY());											break;	// ORA (absolute, Y)
	case 0x1a:	addCycle();																break;	// *NOP (implied)
	case 0x1b:	addCycle(); slo(AM_AbsoluteY());										break;	// *SLO (absolute, Y)
	case 0x1c:	AM_AbsoluteX();															break;	// *NOP (absolute, X)
	case 0x1d:	A() = orr(A(), AM_AbsoluteX());											break;	// ORA (absolute, X)
	case 0x1e:	addCycles(2); Processor::busWrite(asl(AM_AbsoluteX()));					break;	// ASL (absolute, X)
	case 0x1f:	addCycle(); slo(AM_AbsoluteX());										break;	// *SLO (absolute, X)

	case 0x20:	addCycle();  jsr(Address_Absolute());									break;	// JSR (absolute)
	case 0x21:	addCycle(); A() = andr(A(), AM_IndexedIndirectX());						break;	// AND (indexed indirect X)
	case 0x22:																			break;
	case 0x23:	addCycles(2); rla(AM_IndexedIndirectX());								break;	// *RLA (indexed indirect X)
	case 0x24:	bit(A(), AM_ZeroPage());												break;	// BIT (zero page)
	case 0x25:	A() = andr(A(), AM_ZeroPage());											break;	// AND (zero page)
	case 0x26:	addCycle(); Processor::busWrite(rol(AM_ZeroPage()));					break;	// ROL (zero page)
	case 0x27:	addCycle(); rla(AM_ZeroPage());											break;	// *RLA (zero page)
	case 0x28:	addCycles(2); plp();													break;	// PLP
	case 0x29:	A() = andr(A(), AM_Immediate());										break;	// AND (immediate)
	case 0x2a:	addCycle(); A() = rol(A());												break;	// ROL A
	case 0x2b:	addCycle(); anc(AM_Immediate());										break;	// *ANC (immediate)
	case 0x2c:	bit(A(), AM_Absolute());												break;	// BIT (absolute)
	case 0x2d:	A() = andr(A(), AM_Absolute());											break;	// AND (absolute)
	case 0x2e:	addCycle(); Processor::busWrite(rol(AM_Absolute()));					break;	// ROL (absolute)
	case 0x2f:	addCycle(); rla(AM_Absolute());											break;	// *RLA (absolute)

	case 0x30:	branch(negative());														break;	// BMI
	case 0x31:	A() = andr(A(), AM_IndirectIndexedY());									break;	// AND (indirect indexed Y)
	case 0x32:																			break;
	case 0x33:	addCycle(); rla(AM_IndirectIndexedY());									break;	// *RLA (indirect indexed Y)
	case 0x34:	addCycle(); AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x35:	addCycle(); A() = andr(A(), AM_ZeroPageX());							break;	// AND (zero page, X)
	case 0x36:	addCycles(2); Processor::busWrite(rol(AM_ZeroPageX()));					break;	// ROL (zero page, X)
	case 0x37:	addCycles(2); rla(AM_ZeroPageX());										break;	// *RLA (zero page, X)
	case 0x38:	addCycle(); setFlag(P(), CF);											break;	// SEC
	case 0x39:	A() = andr(A(), AM_AbsoluteY());										break;	// AND (absolute, Y)
	case 0x3a:	addCycle();																break;	// *NOP (implied)
	case 0x3b:	addCycle(); rla(AM_AbsoluteY());										break;	// *RLA (absolute, Y)
	case 0x3c:	AM_AbsoluteX();															break;	// *NOP (absolute, X)
	case 0x3d:	A() = andr(A(), AM_AbsoluteX());										break;	// AND (absolute, X)
	case 0x3e:	addCycles(2); Processor::busWrite(rol(AM_AbsoluteX()));					break;	// ROL (absolute, X)
	case 0x3f:	addCycle(); rla(AM_AbsoluteX());										break;	// *RLA (absolute, X)

	case 0x40:	addCycles(2); rti();													break;	// RTI
	case 0x41:	addCycle(); A() = eorr(A(), AM_IndexedIndirectX());						break;	// EOR (indexed indirect X)
	case 0x42:																			break;
	case 0x43:	addCycles(2); sre(AM_IndexedIndirectX());								break;	// *SRE (indexed indirect X)
	case 0x44:	AM_ZeroPage();															break;	// *NOP (zero page)
	case 0x45:	A() = eorr(A(), AM_ZeroPage());											break;	// EOR (zero page)
	case 0x46:	addCycle(); Processor::busWrite(lsr(AM_ZeroPage()));					break;	// LSR (zero page)
	case 0x47:	addCycle(); sre(AM_ZeroPage());											break;	// *SRE (zero page)
	case 0x48:	addCycle(); push(A());													break;	// PHA
	case 0x49:	A() = eorr(A(), AM_Immediate());										break;	// EOR (immediate)
	case 0x4a:	addCycle(); A() = lsr(A());												break;	// LSR A
	case 0x4b:	asr(AM_Immediate());													break;	// *ASR (immediate)
	case 0x4c:	jump(Address_Absolute());												break;	// JMP (absolute)
	case 0x4d:	A() = eorr(A(), AM_Absolute());											break;	// EOR (absolute)
	case 0x4e:	addCycle(); Processor::busWrite(lsr(AM_Absolute()));					break;	// LSR (absolute)
	case 0x4f:	addCycle(); sre(AM_Absolute());											break;	// *SRE (absolute)

	case 0x50:	branch(!overflow());													break;	// BVC (relative)
	case 0x51:	A() = eorr(A(), AM_IndirectIndexedY());									break;	// EOR (indirect indexed Y)
	case 0x52:																			break;
	case 0x53:	addCycle(); sre(AM_IndirectIndexedY());									break;	// *SRE (indirect indexed Y)
	case 0x54:	addCycle(); AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x55:	addCycle(); A() = eorr(A(), AM_ZeroPageX());							break;	// EOR (zero page, X)
	case 0x56:	addCycles(2); Processor::busWrite(lsr(AM_ZeroPageX()));					break;	// LSR (zero page, X)
	case 0x57:	addCycles(2); sre(AM_ZeroPageX());										break;	// *SRE (zero page, X)
	case 0x58:	addCycle(); clearFlag(P(), IF);											break;	// CLI
	case 0x59:	A() = eorr(A(), AM_AbsoluteY());										break;	// EOR (absolute, Y)
	case 0x5a:	addCycle();																break;	// *NOP (implied)
	case 0x5b:	addCycle(); sre(AM_AbsoluteY());										break;	// *SRE (absolute, Y)
	case 0x5c:	AM_AbsoluteX();															break;	// *NOP (absolute, X)
	case 0x5d:	A() = eorr(A(), AM_AbsoluteX());										break;	// EOR (absolute, X)
	case 0x5e:	addCycles(2); Processor::busWrite(lsr(AM_AbsoluteX()));					break;	// LSR (absolute, X)
	case 0x5f:	addCycle(); sre(AM_AbsoluteX());										break;	// *SRE (absolute, X)

	case 0x60:	addCycles(3); rts();													break;	// RTS
	case 0x61:	addCycle(); A() = adc(A(), AM_IndexedIndirectX());						break;	// ADC (indexed indirect X)
	case 0x62:																			break;
	case 0x63:	addCycles(2); rra(AM_IndexedIndirectX());								break;	// *RRA (indexed indirect X)
	case 0x64:	AM_ZeroPage();															break;	// *NOP (zero page)
	case 0x65:	A() = adc(A(), AM_ZeroPage());											break;	// ADC (zero page)
	case 0x66:	addCycle(); Processor::busWrite(ror(AM_ZeroPage()));					break;	// ROR (zero page)
	case 0x67:	addCycle(); rra(AM_ZeroPage());											break;	// *RRA (zero page)
	case 0x68:	addCycles(2); A() = through(pop());										break;	// PLA
	case 0x69:	A() = adc(A(), AM_Immediate());											break;	// ADC (immediate)
	case 0x6a:	addCycle(); A() = ror(A());												break;	// ROR A
	case 0x6b:	addCycle(); arr(AM_Immediate());										break;	// *ARR (immediate)
	case 0x6c:	jump(Address_Indirect());												break;	// JMP (indirect)
	case 0x6d:	A() = adc(A(), AM_Absolute());											break;	// ADC (absolute)
	case 0x6e:	addCycle(); Processor::busWrite(ror(AM_Absolute()));					break;	// ROR (absolute)
	case 0x6f:	addCycle(); rra(AM_Absolute());											break;	// *RRA (absolute)

	case 0x70:	branch(overflow());														break;	// BVS (relative)
	case 0x71:	A() = adc(A(), AM_IndirectIndexedY());									break;	// ADC (indirect indexed Y)
	case 0x72:																			break;
	case 0x73:	addCycle(); rra(AM_IndirectIndexedY());									break;	// *RRA (indirect indexed Y)
	case 0x74:	addCycle(); AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x75:	addCycle(); A() = adc(A(), AM_ZeroPageX());								break;	// ADC (zero page, X)
	case 0x76:	addCycles(2); Processor::busWrite(ror(AM_ZeroPageX()));					break;	// ROR (zero page, X)
	case 0x77:	addCycles(2); rra(AM_ZeroPageX());										break;	// *RRA (zero page, X)
	case 0x78:	addCycle(); setFlag(P(), IF);											break;	// SEI
	case 0x79:	A() = adc(A(), AM_AbsoluteY());											break;	// ADC (absolute, Y)
	case 0x7a:	addCycle();																break;	// *NOP (implied)
	case 0x7b:	addCycle(); rra(AM_AbsoluteY());										break;	// *RRA (absolute, Y)
	case 0x7c:	AM_AbsoluteX();															break;	// *NOP (absolute, X)
	case 0x7d:	A() = adc(A(), AM_AbsoluteX());											break;	// ADC (absolute, X)
	case 0x7e:	addCycles(2); Processor::busWrite(ror(AM_AbsoluteX()));					break;	// ROR (absolute, X)
	case 0x7f:	addCycle(); rra(AM_AbsoluteX());										break;	// *RRA (absolute, X)

	case 0x80:	AM_Immediate();															break;	// *NOP (immediate)
	case 0x81:	addCycle(); Processor::busWrite(Address_IndexedIndirectX(), A());		break;	// STA (indexed indirect X)
	case 0x82:	AM_Immediate();															break;	// *NOP (immediate)
	case 0x83:	addCycle(); Processor::busWrite(Address_IndexedIndirectX(), A() & X());	break;	// *SAX (indexed indirect X)
	case 0x84:	Processor::busWrite(Address_ZeroPage(), Y());							break;	// STY (zero page)
	case 0x85:	Processor::busWrite(Address_ZeroPage(), A());							break;	// STA (zero page)
	case 0x86:	Processor::busWrite(Address_ZeroPage(), X());							break;	// STX (zero page)
	case 0x87:	Processor::busWrite(Address_ZeroPage(), A() & X());						break;	// *SAX (zero page)
	case 0x88:	addCycle(); Y() = dec(Y());												break;	// DEY
	case 0x89:	AM_Immediate();															break;	// *NOP (immediate)
	case 0x8a:	addCycle(); A() = through(X());											break;	// TXA
	case 0x8b:																			break;
	case 0x8c:	Processor::busWrite(Address_Absolute(), Y());							break;	// STY (absolute)
	case 0x8d:	Processor::busWrite(Address_Absolute(), A());							break;	// STA (absolute)
	case 0x8e:	Processor::busWrite(Address_Absolute(), X());							break;	// STX (absolute)
	case 0x8f:	Processor::busWrite(Address_Absolute(), A() & X());						break;	// *SAX (absolute)

	case 0x90:	branch(!carry());														break;	// BCC
	case 0x91:	addCycle(); Processor::busWrite(Address_IndirectIndexedY().first, A());	break;	// STA (indirect indexed Y)
	case 0x92:																			break;
	case 0x93:																			break;
	case 0x94:	addCycle(); Processor::busWrite(Address_ZeroPageX(), Y());				break;	// STY (zero page, X)
	case 0x95:	addCycle(); Processor::busWrite(Address_ZeroPageX(), A());				break;	// STA (zero page, X)
	case 0x96:	addCycle(); Processor::busWrite(Address_ZeroPageY(), X());				break;	// STX (zero page, Y)
	case 0x97:	addCycle(); Processor::busWrite(Address_ZeroPageY(), A() & X());		break;	// *SAX (zero page, Y)
	case 0x98:	addCycle(); A() = through(Y());											break;	// TYA
	case 0x99:	addCycle(); Processor::busWrite(Address_AbsoluteY().first, A());		break;	// STA (absolute, Y)
	case 0x9a:	addCycle(); S() = X();													break;	// TXS
	case 0x9b:																			break;
	case 0x9c:																			break;
	case 0x9d:	addCycle(); Processor::busWrite(Address_AbsoluteX().first, A());		break;	// STA (absolute, X)
	case 0x9e:																			break;
	case 0x9f:																			break;

	case 0xa0:	Y() = through(AM_Immediate());											break;	// LDY (immediate)
	case 0xa1:	addCycle(); A() = through(AM_IndexedIndirectX());						break;	// LDA (indexed indirect X)
	case 0xa2:	X() = through(AM_Immediate());											break;	// LDX (immediate)
	case 0xa3:	addCycle(); A() = X() = through(AM_IndexedIndirectX());					break;	// *LAX (indexed indirect X)
	case 0xa4:	Y() = through(AM_ZeroPage());											break;	// LDY (zero page)
	case 0xa5:	A() = through(AM_ZeroPage());											break;	// LDA (zero page)
	case 0xa6:	X() = through(AM_ZeroPage());											break;	// LDX (zero page)
	case 0xa7:	A() = X() = through(AM_ZeroPage());										break;	// *LAX (zero page)
	case 0xa8:	addCycle(); Y() = through(A());											break;	// TAY
	case 0xa9:	A() = through(AM_Immediate());											break;	// LDA (immediate)
	case 0xaa:	addCycle(); X() = through(A());											break;	// TAX
	case 0xab:																			break;	// *ATX (immediate)
	case 0xac:	Y() = through(AM_Absolute());											break;	// LDY (absolute)
	case 0xad:	A() = through(AM_Absolute());											break;	// LDA (absolute)
	case 0xae:	X() = through(AM_Absolute());											break;	// LDX (absolute)
	case 0xaf:	A() = X() = through(AM_Absolute());										break;	// *LAX (absolute)

	case 0xb0:	branch(carry());														break;	// BCS
	case 0xb1:	A() = through(AM_IndirectIndexedY());									break;	// LDA (indirect indexed Y)
	case 0xb2:																			break;
	case 0xb3:	A() = X() = through(AM_IndirectIndexedY());								break;	// *LAX (indirect indexed Y)
	case 0xb4:	addCycle(); Y() = through(AM_ZeroPageX());								break;	// LDY (zero page, X)
	case 0xb5:	addCycle(); A() = through(AM_ZeroPageX());								break;	// LDA (zero page, X)
	case 0xb6:	addCycle(); X() = through(AM_ZeroPageY());								break;	// LDX (zero page, Y)
	case 0xb7:	addCycle(); A() = X() = through(AM_ZeroPageY());						break;	// *LAX (zero page, Y)
	case 0xb8:	addCycle(); clearFlag(P(), VF);											break;	// CLV
	case 0xb9:	A() = through(AM_AbsoluteY());											break;	// LDA (absolute, Y)
	case 0xba:	addCycle(); X() = through(S());											break;	// TSX
	case 0xbb:																			break;
	case 0xbc:	Y() = through(AM_AbsoluteX());											break;	// LDY (absolute, X)
	case 0xbd:	A() = through(AM_AbsoluteX());											break;	// LDA (absolute, X)
	case 0xbe:	X() = through(AM_AbsoluteY());											break;	// LDX (absolute, Y)
	case 0xbf:	A() = X() = through(AM_AbsoluteY());									break;	// *LAX (absolute, Y)

	case 0xc0:	cmp(Y(), AM_Immediate());												break;	// CPY (immediate)
	case 0xc1:	addCycle(); cmp(A(), AM_IndexedIndirectX());							break;	// CMP (indexed indirect X)
	case 0xc2:	AM_Immediate();															break;	// *NOP (immediate)
	case 0xc3:	addCycles(2); dcp(AM_IndexedIndirectX());								break;	// *DCP (indexed indirect X)
	case 0xc4:	cmp(Y(), AM_ZeroPage());												break;	// CPY (zero page)
	case 0xc5:	cmp(A(), AM_ZeroPage());												break;	// CMP (zero page)
	case 0xc6:	addCycle(); Processor::busWrite(dec(AM_ZeroPage()));					break;	// DEC (zero page)
	case 0xc7:	addCycle(); dcp(AM_ZeroPage());											break;	// *DCP (zero page)
	case 0xc8:	addCycle(); Y() = inc(Y());												break;	// INY
	case 0xc9:	cmp(A(), AM_Immediate());												break;	// CMP (immediate)
	case 0xca:	addCycle(); X() = dec(X());												break;	// DEX
	case 0xcb:	axs(AM_Immediate());													break;	// *AXS (immediate)
	case 0xcc:	cmp(Y(), AM_Absolute());												break;	// CPY (absolute)
	case 0xcd:	cmp(A(), AM_Absolute());												break;	// CMP (absolute)
	case 0xce:	addCycle(); Processor::busWrite(dec(AM_Absolute()));					break;	// DEC (absolute)
	case 0xcf:	addCycle(); dcp(AM_Absolute());											break;	// *DCP (absolute)

	case 0xd0:	branch(!zero());														break;	// BNE
	case 0xd1:	cmp(A(), AM_IndirectIndexedY());										break;	// CMP (indirect indexed Y)
	case 0xd2:																			break;
	case 0xd3:	addCycle(); dcp(AM_IndirectIndexedY());									break;	// *DCP (indirect indexed Y)
	case 0xd4:	addCycle(); AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0xd5:	addCycle(); cmp(A(), AM_ZeroPageX());									break;	// CMP (zero page, X)
	case 0xd6:	addCycles(2); Processor::busWrite(dec(AM_ZeroPageX()));					break;	// DEC (zero page, X)
	case 0xd7:	addCycles(2); dcp(AM_ZeroPageX());										break;	// *DCP (zero page, X)
	case 0xd8:	addCycle(); clearFlag(P(), DF);											break;	// CLD
	case 0xd9:	cmp(A(), AM_AbsoluteY());												break;	// CMP (absolute, Y)
	case 0xda:	addCycle();																break;	// *NOP (implied)
	case 0xdb:	addCycle(); dcp(AM_AbsoluteY());										break;	// *DCP (absolute, Y)
	case 0xdc:	AM_AbsoluteX();															break;	// *NOP (absolute, X)
	case 0xdd:	cmp(A(), AM_AbsoluteX());												break;	// CMP (absolute, X)
	case 0xde:	addCycles(2); Processor::busWrite(dec(AM_AbsoluteX()));					break;	// DEC (absolute, X)
	case 0xdf:	addCycle(); dcp(AM_AbsoluteX());										break;	// *DCP (absolute, X)

	case 0xe0:	cmp(X(), AM_Immediate());												break;	// CPX (immediate)
	case 0xe1:	addCycle(); A() = sbc(A(), AM_IndexedIndirectX());						break;	// SBC (indexed indirect X)
	case 0xe2:	AM_Immediate();															break;	// *NOP (immediate)
	case 0xe3:	addCycles(2); isb(AM_IndexedIndirectX());								break;	// *ISB (indexed indirect X)
	case 0xe4:	cmp(X(), AM_ZeroPage());												break;	// CPX (zero page)
	case 0xe5:	A() = sbc(A(), AM_ZeroPage());											break;	// SBC (zero page)
	case 0xe6:	addCycle(); Processor::busWrite(inc(AM_ZeroPage()));					break;	// INC (zero page)
	case 0xe7:	addCycle(); isb(AM_ZeroPage());											break;	// *ISB (zero page)
	case 0xe8:	addCycle(); X() = inc(X());												break;	// INX
	case 0xe9:	A() = sbc(A(), AM_Immediate());											break;	// SBC (immediate)
	case 0xea:	addCycle();																break;	// NOP
	case 0xeb:	A() = sbc(A(), AM_Immediate());											break;	// *SBC (immediate)
	case 0xec:	cmp(X(), AM_Absolute());												break;	// CPX (absolute)
	case 0xed:	A() = sbc(A(), AM_Absolute());											break;	// SBC (absolute)
	case 0xee:	addCycle(); Processor::busWrite(inc(AM_Absolute()));					break;	// INC (absolute)
	case 0xef:	addCycle(); isb(AM_Absolute());											break;	// *ISB (absolute)

	case 0xf0:	branch(zero());															break;	// BEQ
	case 0xf1:	A() = sbc(A(), AM_IndirectIndexedY());									break;	// SBC (indirect indexed Y)
	case 0xf2:																			break;
	case 0xf3:	addCycle(); isb(AM_IndirectIndexedY());									break;	// *ISB (indirect indexed Y)
	case 0xf4:	addCycle(); AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0xf5:	addCycle(); A() = sbc(A(), AM_ZeroPageX());								break;	// SBC (zero page, X)
	case 0xf6:	addCycles(2); Processor::busWrite(inc(AM_ZeroPageX()));					break;	// INC (zero page, X)
	case 0xf7:	addCycles(2); isb(AM_ZeroPageX());										break;	// *ISB (zero page, X)
	case 0xf8:	addCycle(); setFlag(P(), DF);											break;	// SED
	case 0xf9:	A() = sbc(A(), AM_AbsoluteY());											break;	// SBC (absolute, Y)
	case 0xfa:	addCycle();																break;	// *NOP (implied)
	case 0xfb:	addCycle(); isb(AM_AbsoluteY());										break;	// *ISB (absolute, Y)
	case 0xfc:	AM_AbsoluteX();															break;	// *NOP (absolute, X)
	case 0xfd:	A() = sbc(A(), AM_AbsoluteX());											break;	// SBC (absolute, X)
	case 0xfe:	addCycles(2); Processor::busWrite(inc(AM_AbsoluteX()));					break;	// INC (absolute, X)
	case 0xff:	addCycle(); isb(AM_AbsoluteX());										break;	// *ISB (absolute, X)
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
	return Processor::busRead(Address_Absolute());
}

uint8_t EightBit::MOS6502::AM_ZeroPage() {
	return Processor::busRead(Address_ZeroPage());
}

uint8_t EightBit::MOS6502::AM_AbsoluteX() {
	const auto [address, paged] = Address_AbsoluteX();
	if (UNLIKELY(paged))
		addCycle();
	return Processor::busRead(address);
}

uint8_t EightBit::MOS6502::AM_AbsoluteY() {
	const auto [address, paged] = Address_AbsoluteY();
	if (UNLIKELY(paged))
		addCycle();
	return Processor::busRead(address);
}

uint8_t EightBit::MOS6502::AM_ZeroPageX() {
	return Processor::busRead(Address_ZeroPageX());
}

uint8_t EightBit::MOS6502::AM_ZeroPageY() {
	return Processor::busRead(Address_ZeroPageY());
}

uint8_t EightBit::MOS6502::AM_IndexedIndirectX() {
	return Processor::busRead(Address_IndexedIndirectX());
}

uint8_t EightBit::MOS6502::AM_IndirectIndexedY() {
	const auto [address, paged] = Address_IndirectIndexedY();
	if (UNLIKELY(paged))
		addCycle();
	return Processor::busRead(address);
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
	setFlag(P(), CF, value & Bit7);
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

// Undocumented compound instructions

void EightBit::MOS6502::anc(const uint8_t value) {
	A() = andr(A(), value);
	setFlag(P(), CF, A() & Bit7);
}

void EightBit::MOS6502::arr(const uint8_t value) {
	A() = andr(A(), value);
	A() = ror(A());
	setFlag(P(), CF, A() & Bit6);
	setFlag(P(), VF, ((A() & Bit6) >> 6) ^((A() & Bit5) >> 5));
}

void EightBit::MOS6502::asr(const uint8_t value) {
	A() = andr(A(), value);
	A() = lsr(A());
}

void EightBit::MOS6502::axs(const uint8_t value) {
	X() = through(sub(A() & X(), value));
	clearFlag(P(), CF, m_intermediate.high);
}

void EightBit::MOS6502::dcp(const uint8_t value) {
	Processor::busWrite(dec(value));
	cmp(A(), BUS().DATA());
}

void EightBit::MOS6502::isb(const uint8_t value) {
	Processor::busWrite(inc(value));
	A() = sbc(A(), BUS().DATA());
}

void EightBit::MOS6502::rla(const uint8_t value) {
	Processor::busWrite(rol(value));
	A() = andr(A(), BUS().DATA());
}

void EightBit::MOS6502::rra(const uint8_t value) {
	Processor::busWrite(ror(value));
	A() = adc(A(), BUS().DATA());
}

void EightBit::MOS6502::slo(const uint8_t value) {
	Processor::busWrite(asl(value));
	A() = orr(A(), BUS().DATA());
}

void EightBit::MOS6502::sre(const uint8_t value) {
	Processor::busWrite(lsr(value));
	A() = eorr(A(), BUS().DATA());
}
