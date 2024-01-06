#include "stdafx.h"
#include "../inc/mos6502.h"

EightBit::MOS6502::MOS6502(Bus& bus) noexcept
	: LittleEndianProcessor(bus) {
	RaisedPOWER.connect([this](EventArgs) {
		X() = Bit7;
		Y() = 0;
		A() = 0;
		P() = RF;
		S() = Mask8;
		lowerSYNC();
		lowerRW();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(NMI, MOS6502)
DEFINE_PIN_LEVEL_CHANGERS(SO, MOS6502)
DEFINE_PIN_LEVEL_CHANGERS(SYNC, MOS6502)
DEFINE_PIN_LEVEL_CHANGERS(RDY, MOS6502)
DEFINE_PIN_LEVEL_CHANGERS(RW, MOS6502)

int EightBit::MOS6502::step() noexcept {
	resetCycles();
	ExecutingInstruction.fire(*this);
	if (LIKELY(powered())) {
		if (UNLIKELY(lowered(SO())))
			handleSO();
		if (LIKELY(raised(RDY()))) {
			lowerSYNC();	// Instruction fetch beginning
			raiseRW();
			opcode() = BUS().read(PC()++);	// can't use fetchByte
			if (UNLIKELY(lowered(RESET())))
				handleRESET();
			else if (UNLIKELY(lowered(NMI())))
				handleNMI();
			else if (UNLIKELY(lowered(INT()) && !interruptMasked()))
				handleINT();
			execute();
		}
	}
	ExecutedInstruction.fire(*this);
	return cycles();
}

// Interrupt (etc.) handlers

void EightBit::MOS6502::handleSO() noexcept {
	raiseSO();
	P() |= VF;
}

void EightBit::MOS6502::handleRESET() noexcept {
	raiseRESET();
	m_handlingRESET = true;
	opcode() = 0x00;	// BRK
}


void EightBit::MOS6502::handleNMI() noexcept {
	raiseNMI();
	m_handlingNMI = true;
	opcode() = 0x00;	// BRK
}

void EightBit::MOS6502::handleINT() noexcept {
	raiseINT();
	m_handlingINT = true;
	opcode() = 0x00;	// BRK
}

void EightBit::MOS6502::interrupt() noexcept {
	const bool reset = m_handlingRESET;
	const bool nmi = m_handlingNMI;
	if (reset) {
		dummyPush(PC().high);
		dummyPush(PC().low);
		dummyPush(P());
	} else {
		const bool irq = m_handlingINT;
		const bool hardware = nmi || irq || reset;
		const bool software = !hardware;
		pushWord(PC());
		push(P() | (software ? BF : 0));
	}
	P() = setBit(P(), IF);	// Disable IRQ
	const uint8_t vector = reset ? RSTvector : (nmi ? NMIvector : IRQvector);
	jump(getWordPaged(0xff, vector));
	m_handlingRESET = m_handlingNMI = m_handlingINT = false;
}

//

void EightBit::MOS6502::busWrite() noexcept {
	tick();
	lowerRW();
	Processor::busWrite();
}

uint8_t EightBit::MOS6502::busRead() noexcept {
	tick();
	raiseRW();
	return Processor::busRead();
}

//

int EightBit::MOS6502::execute() noexcept {

	raiseSYNC();	// Instruction fetch has now completed

	switch (opcode()) {

	case 0x00:	fetchByte(); interrupt();									break;	// BRK (implied)
	case 0x01:	A() = orr(A(), AM_IndexedIndirectX());						break;	// ORA (indexed indirect X)
	case 0x02:	jam();														break;	// *JAM
	case 0x03:	slo(AM_IndexedIndirectX());									break;	// *SLO (indexed indirect X)
	case 0x04:	AM_ZeroPage();												break;	// *NOP (zero page)
	case 0x05:	A() = orr(A(), AM_ZeroPage());								break;	// ORA (zero page)
	case 0x06:	memoryReadModifyWrite(asl(AM_ZeroPage()));					break;	// ASL (zero page)
	case 0x07:	slo(AM_ZeroPage());											break;	// *SLO (zero page)
	case 0x08:	memoryRead(PC()); php();									break;	// PHP (implied)
	case 0x09:	A() = orr(A(), AM_Immediate());								break;	// ORA (immediate)
	case 0x0a:	memoryRead(PC()); A() = asl(A());							break;	// ASL A (implied)
	case 0x0b:	anc(AM_Immediate());										break;	// *ANC (immediate)
	case 0x0c:  { auto ignored = Address_Absolute(); }						break;	// *NOP (absolute)
	case 0x0d:	A() = orr(A(), AM_Absolute());								break;	// ORA (absolute)
	case 0x0e:	memoryReadModifyWrite(asl(AM_Absolute()));					break;	// ASL (absolute)
	case 0x0f:	slo(AM_Absolute());											break;	// *SLO (absolute)

	case 0x10:	branch(negative() == 0);									break;	// BPL (relative)
	case 0x11:	A() = orr(A(), AM_IndirectIndexedY());						break;	// ORA (indirect indexed Y)
	case 0x12:	jam();														break;	// *JAM
	case 0x13:	slo_IndirectIndexedY();										break;	// *SLO (indirect indexed Y)
	case 0x14:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x15:	A() = orr(A(), AM_ZeroPageX());								break;	// ORA (zero page, X)
	case 0x16:	memoryReadModifyWrite(asl(AM_ZeroPageX()));					break;	// ASL (zero page, X)
	case 0x17:	slo(AM_ZeroPageX());										break;	// *SLO (zero page, X)
	case 0x18:	memoryRead(PC()); P() = clearBit(P(), CF);					break;	// CLC (implied)
	case 0x19:	A() = orr(A(), AM_AbsoluteY());								break;	// ORA (absolute, Y)
	case 0x1a:	memoryRead(PC());											break;	// *NOP (implied)
	case 0x1b:	slo_AbsoluteY();											break;	// *SLO (absolute, Y)
	case 0x1c:	nop_AbsoluteX();											break;	// *NOP (absolute, X)
	case 0x1d:	A() = orr(A(), AM_AbsoluteX());								break;	// ORA (absolute, X)
	case 0x1e:	memoryReadModifyWrite(asl(AM_AbsoluteX(PageCrossingBehavior::AlwaysReadTwice)));		break;	// ASL (absolute, X)
	case 0x1f:	slo_AbsoluteX();											break;	// *SLO (absolute, X)

	case 0x20:	jsr();														break;	// JSR (absolute)
	case 0x21:	A() = andr(A(), AM_IndexedIndirectX());						break;	// AND (indexed indirect X)
	case 0x22:	jam();														break;	// *JAM
	case 0x23:	rla(AM_IndexedIndirectX());									break;	// *RLA (indexed indirect X)
	case 0x24:	bit(A(), AM_ZeroPage());									break;	// BIT (zero page)
	case 0x25:	A() = andr(A(), AM_ZeroPage());								break;	// AND (zero page)
	case 0x26:	memoryReadModifyWrite(rol(AM_ZeroPage()));					break;	// ROL (zero page)
	case 0x27:	rla(AM_ZeroPage());											break;	// *RLA (zero page)
	case 0x28:	memoryRead(PC()); getBytePaged(1, S());  plp();				break;	// PLP (implied)
	case 0x29:	A() = andr(A(), AM_Immediate());							break;	// AND (immediate)
	case 0x2a:	memoryRead(PC()); A() = rol(A());							break;	// ROL A (implied)
	case 0x2b:	anc(AM_Immediate());										break;	// *ANC (immediate)
	case 0x2c:	bit(A(), AM_Absolute());									break;	// BIT (absolute)
	case 0x2d:	A() = andr(A(), AM_Absolute());								break;	// AND (absolute)
	case 0x2e:	memoryReadModifyWrite(rol(AM_Absolute()));					break;	// ROL (absolute)
	case 0x2f:	rla(AM_Absolute());											break;	// *RLA (absolute)

	case 0x30:	branch(negative());											break;	// BMI (relative)
	case 0x31:	A() = andr(A(), AM_IndirectIndexedY());						break;	// AND (indirect indexed Y)
	case 0x32:	jam();														break;	// *JAM
	case 0x33:	rla_IndirectIndexedY();										break;	// *RLA (indirect indexed Y)
	case 0x34:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x35:	A() = andr(A(), AM_ZeroPageX());							break;	// AND (zero page, X)
	case 0x36:	memoryReadModifyWrite(rol(AM_ZeroPageX()));					break;	// ROL (zero page, X)
	case 0x37:	rla(AM_ZeroPageX());										break;	// *RLA (zero page, X)
	case 0x38:	memoryRead(PC()); P() = setBit(P(), CF);					break;	// SEC (implied)
	case 0x39:	A() = andr(A(), AM_AbsoluteY());							break;	// AND (absolute, Y)
	case 0x3a:	memoryRead(PC());											break;	// *NOP (implied)
	case 0x3b:	rla_AbsoluteY();											break;	// *RLA (absolute, Y)
	case 0x3c:	nop_AbsoluteX();											break;	// *NOP (absolute, X)
	case 0x3d:	A() = andr(A(), AM_AbsoluteX());							break;	// AND (absolute, X)
	case 0x3e:	memoryReadModifyWrite(rol(AM_AbsoluteX(PageCrossingBehavior::AlwaysReadTwice)));		break;	// ROL (absolute, X)
	case 0x3f:	rla_AbsoluteX();											break;	// *RLA (absolute, X)

	case 0x40:	memoryRead(PC()); rti();									break;	// RTI (implied)
	case 0x41:	A() = eorr(A(), AM_IndexedIndirectX());						break;	// EOR (indexed indirect X)
	case 0x42:	jam();														break;	// *JAM
	case 0x43:	sre(AM_IndexedIndirectX());									break;	// *SRE (indexed indirect X)
	case 0x44:	AM_ZeroPage();												break;	// *NOP (zero page)
	case 0x45:	A() = eorr(A(), AM_ZeroPage());								break;	// EOR (zero page)
	case 0x46:	memoryReadModifyWrite(lsr(AM_ZeroPage()));					break;	// LSR (zero page)
	case 0x47:	sre(AM_ZeroPage());											break;	// *SRE (zero page)
	case 0x48:	memoryRead(PC()); push(A());								break;	// PHA (implied)
	case 0x49:	A() = eorr(A(), AM_Immediate());							break;	// EOR (immediate)
	case 0x4a:	memoryRead(PC()); A() = lsr(A());							break;	// LSR A (implied)
	case 0x4b:	asr(AM_Immediate());										break;	// *ASR (immediate)
	case 0x4c:	jump(Address_Absolute());									break;	// JMP (absolute)
	case 0x4d:	A() = eorr(A(), AM_Absolute());								break;	// EOR (absolute)
	case 0x4e:	memoryReadModifyWrite(lsr(AM_Absolute()));					break;	// LSR (absolute)
	case 0x4f:	sre(AM_Absolute());											break;	// *SRE (absolute)

	case 0x50:	branch(overflow() == 0);									break;	// BVC (relative)
	case 0x51:	A() = eorr(A(), AM_IndirectIndexedY());						break;	// EOR (indirect indexed Y)
	case 0x52:	jam();														break;	// *JAM
	case 0x53:	sre_IndirectIndexedY();										break;	// *SRE (indirect indexed Y)
	case 0x54:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x55:	A() = eorr(A(), AM_ZeroPageX());							break;	// EOR (zero page, X)
	case 0x56:	memoryReadModifyWrite(lsr(AM_ZeroPageX()));					break;	// LSR (zero page, X)
	case 0x57:	sre(AM_ZeroPageX());										break;	// *SRE (zero page, X)
	case 0x58:	memoryRead(PC()); P() = clearBit(P(), IF);					break;	// CLI (implied)
	case 0x59:	A() = eorr(A(), AM_AbsoluteY());							break;	// EOR (absolute, Y)
	case 0x5a:	memoryRead(PC());											break;	// *NOP (implied)
	case 0x5b:	sre_AbsoluteY();											break;	// *SRE (absolute, Y)
	case 0x5c:	nop_AbsoluteX();											break;	// *NOP (absolute, X)
	case 0x5d:	A() = eorr(A(), AM_AbsoluteX());							break;	// EOR (absolute, X)
	case 0x5e:	memoryReadModifyWrite(lsr(AM_AbsoluteX(PageCrossingBehavior::AlwaysReadTwice)));		break;	// LSR (absolute, X)
	case 0x5f:	sre_AbsoluteX();											break;	// *SRE (absolute, X)

	case 0x60:	memoryRead(PC()); rts();									break;	// RTS (implied)
	case 0x61:	A() = adc(A(), AM_IndexedIndirectX());						break;	// ADC (indexed indirect X)
	case 0x62:	jam();														break;	// *JAM
	case 0x63:	rra(AM_IndexedIndirectX());									break;	// *RRA (indexed indirect X)
	case 0x64:	AM_ZeroPage();												break;	// *NOP (zero page)
	case 0x65:	A() = adc(A(), AM_ZeroPage());								break;	// ADC (zero page)
	case 0x66:	memoryReadModifyWrite(ror(AM_ZeroPage()));					break;	// ROR (zero page)
	case 0x67:	rra(AM_ZeroPage());											break;	// *RRA (zero page)
	case 0x68:	memoryRead(PC()); getBytePaged(1, S()); A() = through(pop());	break;	// PLA (implied)
	case 0x69:	A() = adc(A(), AM_Immediate());								break;	// ADC (immediate)
	case 0x6a:	memoryRead(PC()); A() = ror(A());							break;	// ROR A (implied)
	case 0x6b:	arr(AM_Immediate());										break;	// *ARR (immediate)
	case 0x6c:	jump(Address_Indirect());									break;	// JMP (indirect)
	case 0x6d:	A() = adc(A(), AM_Absolute());								break;	// ADC (absolute)
	case 0x6e:	memoryReadModifyWrite(ror(AM_Absolute()));					break;	// ROR (absolute)
	case 0x6f:	rra(AM_Absolute());											break;	// *RRA (absolute)

	case 0x70:	branch(overflow());											break;	// BVS (relative)
	case 0x71:	A() = adc(A(), AM_IndirectIndexedY());						break;	// ADC (indirect indexed Y)
	case 0x72:	jam();														break;	// *JAM
	case 0x73:	rra_IndirectIndexedY();										break;	// *RRA (indirect indexed Y)
	case 0x74:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x75:	A() = adc(A(), AM_ZeroPageX());								break;	// ADC (zero page, X)
	case 0x76:	memoryReadModifyWrite(ror(AM_ZeroPageX()));					break;	// ROR (zero page, X)
	case 0x77:	rra(AM_ZeroPageX());										break;	// *RRA (zero page, X)
	case 0x78:	memoryRead(PC()); P() = setBit(P(), IF);					break;	// SEI (implied)
	case 0x79:	A() = adc(A(), AM_AbsoluteY());								break;	// ADC (absolute, Y)
	case 0x7a:	memoryRead(PC());											break;	// *NOP (implied)
	case 0x7b:	rra_AbsoluteY();											break;	// *RRA (absolute, Y)
	case 0x7c:	nop_AbsoluteX();											break;	// *NOP (absolute, X)
	case 0x7d:	A() = adc(A(), AM_AbsoluteX());								break;	// ADC (absolute, X)
	case 0x7e:	memoryReadModifyWrite(ror(AM_AbsoluteX(PageCrossingBehavior::AlwaysReadTwice)));		break;	// ROR (absolute, X)
	case 0x7f:	rra_AbsoluteX();											break;	// *RRA (absolute, X)

	case 0x80:	AM_Immediate();												break;	// *NOP (immediate)
	case 0x81:	memoryWrite(Address_IndexedIndirectX(), A());				break;	// STA (indexed indirect X)
	case 0x82:	AM_Immediate();												break;	// *NOP (immediate)
	case 0x83:	memoryWrite(Address_IndexedIndirectX(), A() & X());			break;	// *SAX (indexed indirect X)
	case 0x84:	memoryWrite(Address_ZeroPage(), Y());						break;	// STY (zero page)
	case 0x85:	memoryWrite(Address_ZeroPage(), A());						break;	// STA (zero page)
	case 0x86:	memoryWrite(Address_ZeroPage(), X());						break;	// STX (zero page)
	case 0x87:	memoryWrite(Address_ZeroPage(), A() & X());					break;	// *SAX (zero page)
	case 0x88:	memoryRead(PC()); Y() = dec(Y());							break;	// DEY (implied)
	case 0x89:	AM_Immediate();												break;	// *NOP (immediate)
	case 0x8a:	memoryRead(PC()); A() = through(X());						break;	// TXA (implied)
	case 0x8b:																break;
	case 0x8c:	memoryWrite(Address_Absolute(), Y());						break;	// STY (absolute)
	case 0x8d:	memoryWrite(Address_Absolute(), A());						break;	// STA (absolute)
	case 0x8e:	memoryWrite(Address_Absolute(), X());						break;	// STX (absolute)
	case 0x8f:	memoryWrite(Address_Absolute(), A() & X());					break;	// *SAX (absolute)

	case 0x90:	branch(carry() == 0);										break;	// BCC (relative)
	case 0x91:	sta_IndirectIndexedY();										break;	// STA (indirect indexed Y)
	case 0x92:	jam();														break;	// *JAM
	case 0x93:																break;
	case 0x94:	memoryWrite(Address_ZeroPageX(), Y());						break;	// STY (zero page, X)
	case 0x95:	memoryWrite(Address_ZeroPageX(), A());						break;	// STA (zero page, X)
	case 0x96:	memoryWrite(Address_ZeroPageY(), X());						break;	// STX (zero page, Y)
	case 0x97:	memoryWrite(Address_ZeroPageY(), A() & X());				break;	// *SAX (zero page, Y)
	case 0x98:	memoryRead(PC()); A() = through(Y());						break;	// TYA (implied)
	case 0x99:	sta_AbsoluteY();											break;	// STA (absolute, Y)
	case 0x9a:	memoryRead(PC()); S() = X();								break;	// TXS (implied)
	case 0x9b:																break;
	case 0x9c:	sya_AbsoluteX();											break;  // *SYA (absolute, X)
	case 0x9d:	sta_AbsoluteX();											break;	// STA (absolute, X)
	case 0x9e:	sxa_AbsoluteY();											break;  // *SXA (absolute, Y)
	case 0x9f:																break;

	case 0xa0:	Y() = through(AM_Immediate());								break;	// LDY (immediate)
	case 0xa1:	A() = through(AM_IndexedIndirectX());						break;	// LDA (indexed indirect X)
	case 0xa2:	X() = through(AM_Immediate());								break;	// LDX (immediate)
	case 0xa3:	A() = X() = through(AM_IndexedIndirectX());					break;	// *LAX (indexed indirect X)
	case 0xa4:	Y() = through(AM_ZeroPage());								break;	// LDY (zero page)
	case 0xa5:	A() = through(AM_ZeroPage());								break;	// LDA (zero page)
	case 0xa6:	X() = through(AM_ZeroPage());								break;	// LDX (zero page)
	case 0xa7:	A() = X() = through(AM_ZeroPage());							break;	// *LAX (zero page)
	case 0xa8:	memoryRead(PC()); Y() = through(A());						break;	// TAY (implied)
	case 0xa9:	A() = through(AM_Immediate());								break;	// LDA (immediate)
	case 0xaa:	memoryRead(PC()); X() = through(A());						break;	// TAX (implied)
	case 0xab:	A() = X() = through((A() | 0xee) & AM_Immediate());			break;	// *ATX (immediate)
	case 0xac:	Y() = through(AM_Absolute());								break;	// LDY (absolute)
	case 0xad:	A() = through(AM_Absolute());								break;	// LDA (absolute)
	case 0xae:	X() = through(AM_Absolute());								break;	// LDX (absolute)
	case 0xaf:	A() = X() = through(AM_Absolute());							break;	// *LAX (absolute)

	case 0xb0:	branch(carry());											break;	// BCS (relative)
	case 0xb1:	A() = through(AM_IndirectIndexedY());						break;	// LDA (indirect indexed Y)
	case 0xb2:	jam();														break;	// *JAM
	case 0xb3:	A() = X() = through(AM_IndirectIndexedY());					break;	// *LAX (indirect indexed Y)
	case 0xb4:	Y() = through(AM_ZeroPageX());								break;	// LDY (zero page, X)
	case 0xb5:	A() = through(AM_ZeroPageX());								break;	// LDA (zero page, X)
	case 0xb6:	X() = through(AM_ZeroPageY());								break;	// LDX (zero page, Y)
	case 0xb7:	A() = X() = through(AM_ZeroPageY());						break;	// *LAX (zero page, Y)
	case 0xb8:	memoryRead(PC()); P() = clearBit(P(), VF);					break;	// CLV (implied)
	case 0xb9:	A() = through(AM_AbsoluteY());								break;	// LDA (absolute, Y)
	case 0xba:	memoryRead(PC()); X() = through(S());						break;	// TSX (implied)
	case 0xbb:																break;
	case 0xbc:	Y() = through(AM_AbsoluteX());								break;	// LDY (absolute, X)
	case 0xbd:	A() = through(AM_AbsoluteX());								break;	// LDA (absolute, X)
	case 0xbe:	X() = through(AM_AbsoluteY());								break;	// LDX (absolute, Y)
	case 0xbf:	A() = X() = through(AM_AbsoluteY());						break;	// *LAX (absolute, Y)

	case 0xc0:	cmp(Y(), AM_Immediate());									break;	// CPY (immediate)
	case 0xc1:	cmp(A(), AM_IndexedIndirectX());							break;	// CMP (indexed indirect X)
	case 0xc2:	AM_Immediate();												break;	// *NOP (immediate)
	case 0xc3:	dcp(AM_IndexedIndirectX());									break;	// *DCP (indexed indirect X)
	case 0xc4:	cmp(Y(), AM_ZeroPage());									break;	// CPY (zero page)
	case 0xc5:	cmp(A(), AM_ZeroPage());									break;	// CMP (zero page)
	case 0xc6:	memoryReadModifyWrite(dec(AM_ZeroPage()));					break;	// DEC (zero page)
	case 0xc7:	dcp(AM_ZeroPage());											break;	// *DCP (zero page)
	case 0xc8:	memoryRead(PC()); Y() = inc(Y());							break;	// INY (implied)
	case 0xc9:	cmp(A(), AM_Immediate());									break;	// CMP (immediate)
	case 0xca:	memoryRead(PC()); X() = dec(X());							break;	// DEX (implied)
	case 0xcb:	axs(AM_Immediate());										break;	// *AXS (immediate)
	case 0xcc:	cmp(Y(), AM_Absolute());									break;	// CPY (absolute)
	case 0xcd:	cmp(A(), AM_Absolute());									break;	// CMP (absolute)
	case 0xce:	memoryReadModifyWrite(dec(AM_Absolute()));					break;	// DEC (absolute)
	case 0xcf:	dcp(AM_Absolute());											break;	// *DCP (absolute)

	case 0xd0:	branch(zero() == 0);										break;	// BNE (relative)
	case 0xd1:	cmp(A(), AM_IndirectIndexedY());							break;	// CMP (indirect indexed Y)
	case 0xd2:	jam();														break;	// *JAM
	case 0xd3:	dcp_IndirectIndexedY();										break;	// *DCP (indirect indexed Y)
	case 0xd4:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0xd5:	cmp(A(), AM_ZeroPageX());									break;	// CMP (zero page, X)
	case 0xd6:	memoryReadModifyWrite(dec(AM_ZeroPageX()));					break;	// DEC (zero page, X)
	case 0xd7:	dcp(AM_ZeroPageX());										break;	// *DCP (zero page, X)
	case 0xd8:	memoryRead(PC()); P() = clearBit(P(), DF);					break;	// CLD (implied)
	case 0xd9:	cmp(A(), AM_AbsoluteY());									break;	// CMP (absolute, Y)
	case 0xda:	memoryRead(PC());											break;	// *NOP (implied)
	case 0xdb:	dcp_AbsoluteY();											break;	// *DCP (absolute, Y)
	case 0xdc:	nop_AbsoluteX();											break;	// *NOP (absolute, X)
	case 0xdd:	cmp(A(), AM_AbsoluteX());									break;	// CMP (absolute, X)
	case 0xde:	memoryReadModifyWrite(dec(AM_AbsoluteX(PageCrossingBehavior::AlwaysReadTwice)));		break;	// DEC (absolute, X)
	case 0xdf:	dcp_AbsoluteX();											break;	// *DCP (absolute, X)

	case 0xe0:	cmp(X(), AM_Immediate());									break;	// CPX (immediate)
	case 0xe1:	A() = sbc(A(), AM_IndexedIndirectX());						break;	// SBC (indexed indirect X)
	case 0xe2:	AM_Immediate();												break;	// *NOP (immediate)
	case 0xe3:	isb(AM_IndexedIndirectX());									break;	// *ISB (indexed indirect X)
	case 0xe4:	cmp(X(), AM_ZeroPage());									break;	// CPX (zero page)
	case 0xe5:	A() = sbc(A(), AM_ZeroPage());								break;	// SBC (zero page)
	case 0xe6:	memoryReadModifyWrite(inc(AM_ZeroPage()));					break;	// INC (zero page)
	case 0xe7:	isb(AM_ZeroPage());											break;	// *ISB (zero page)
	case 0xe8:	memoryRead(PC()); X() = inc(X());							break;	// INX (implied)
	case 0xe9:	A() = sbc(A(), AM_Immediate());								break;	// SBC (immediate)
	case 0xea:	memoryRead(PC());											break;	// NOP (implied)
	case 0xeb:	A() = sbc(A(), AM_Immediate());								break;	// *SBC (immediate)
	case 0xec:	cmp(X(), AM_Absolute());									break;	// CPX (absolute)
	case 0xed:	A() = sbc(A(), AM_Absolute());								break;	// SBC (absolute)
	case 0xee:	memoryReadModifyWrite(inc(AM_Absolute()));					break;	// INC (absolute)
	case 0xef:	isb(AM_Absolute());											break;	// *ISB (absolute)

	case 0xf0:	branch(zero());												break;	// BEQ (relative)
	case 0xf1:	A() = sbc(A(), AM_IndirectIndexedY());						break;	// SBC (indirect indexed Y)
	case 0xf2:	jam();														break;	// *JAM
	case 0xf3:	isb_IndirectIndexedY();										break;	// *ISB (indirect indexed Y)
	case 0xf4:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0xf5:	A() = sbc(A(), AM_ZeroPageX());								break;	// SBC (zero page, X)
	case 0xf6:	memoryReadModifyWrite(inc(AM_ZeroPageX()));					break;	// INC (zero page, X)
	case 0xf7:	isb(AM_ZeroPageX());										break;	// *ISB (zero page, X)
	case 0xf8:	memoryRead(PC()); P() = setBit(P(), DF);					break;	// SED (implied)
	case 0xf9:	A() = sbc(A(), AM_AbsoluteY());								break;	// SBC (absolute, Y)
	case 0xfa:	memoryRead(PC());											break;	// *NOP (implied)
	case 0xfb:	isb_AbsoluteY();											break;	// *ISB (absolute, Y)
	case 0xfc:	nop_AbsoluteX();											break;	// *NOP (absolute, X)
	case 0xfd:	A() = sbc(A(), AM_AbsoluteX());								break;	// SBC (absolute, X)
	case 0xfe:	memoryReadModifyWrite(inc(AM_AbsoluteX(PageCrossingBehavior::AlwaysReadTwice)));		break;	// INC (absolute, X)
	case 0xff:	isb_AbsoluteX();											break;	// *ISB (absolute, X)
	}

	ASSUME(cycles() > 0);
	return cycles();
}

////

void EightBit::MOS6502::push(uint8_t value) noexcept {
	setBytePaged(1, S()--, value);
}

uint8_t EightBit::MOS6502::pop() noexcept {
	return getBytePaged(1, ++S());
}

void EightBit::MOS6502::dummyPush(const uint8_t value) noexcept {
	tick();
	BUS().DATA() = value;
	BUS().ADDRESS() = { S()--, 1 };
}

////

EightBit::register16_t EightBit::MOS6502::Address_Absolute() noexcept {
	return fetchWord();
}

uint8_t EightBit::MOS6502::Address_ZeroPage() noexcept {
	return fetchByte();
}

EightBit::register16_t EightBit::MOS6502::Address_ZeroPageIndirect() noexcept {
	return getWordPaged(0, Address_ZeroPage());
}

EightBit::register16_t EightBit::MOS6502::Address_Indirect() noexcept {
	const auto address = Address_Absolute();
	return getWordPaged(address.high, address.low);
}

uint8_t EightBit::MOS6502::Address_ZeroPageX() noexcept {
	const auto address = Address_ZeroPage();
	memoryRead(address);
	return address + X();
}

uint8_t EightBit::MOS6502::Address_ZeroPageY() noexcept {
	const auto address = Address_ZeroPage();
	memoryRead(address);
	return address + Y();
}

std::pair<EightBit::register16_t, uint8_t> EightBit::MOS6502::Address_AbsoluteX() noexcept {
	const auto address = Address_Absolute();
	const auto page = address.high;
	return { address + X(), page };
}

std::pair<EightBit::register16_t, uint8_t> EightBit::MOS6502::Address_AbsoluteY() noexcept {
	const auto address = Address_Absolute();
	const auto page = address.high;
	return { address + Y(), page };
}

EightBit::register16_t EightBit::MOS6502::Address_IndexedIndirectX() noexcept {
	return getWordPaged(0, Address_ZeroPageX());
}

std::pair<EightBit::register16_t, uint8_t> EightBit::MOS6502::Address_IndirectIndexedY() noexcept {
	const auto address = Address_ZeroPageIndirect();
	const auto page = address.high;
	return { address + Y(), page };
}

EightBit::register16_t EightBit::MOS6502::Address_relative_byte() noexcept {
	return PC() + (int8_t)fetchByte();
}

// Addressing modes, read

uint8_t EightBit::MOS6502::AM_Immediate() noexcept {
	return fetchByte();
}

uint8_t EightBit::MOS6502::AM_Absolute() noexcept {
	return memoryRead(Address_Absolute());
}

uint8_t EightBit::MOS6502::AM_ZeroPage() noexcept {
	return memoryRead(Address_ZeroPage());
}

uint8_t EightBit::MOS6502::AM_AbsoluteX(const PageCrossingBehavior behaviour) noexcept {
	const auto [address, page] = Address_AbsoluteX();
	auto possible = getBytePaged(page, address.low);
	if ((behaviour == PageCrossingBehavior::AlwaysReadTwice) || UNLIKELY(page != address.high))
		possible = memoryRead(address);
	return possible;
}

uint8_t EightBit::MOS6502::AM_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	auto possible = getBytePaged(page, address.low);
	if (UNLIKELY(page != address.high))
		possible = memoryRead(address);
	return possible;
}

uint8_t EightBit::MOS6502::AM_ZeroPageX() noexcept {
	return memoryRead(Address_ZeroPageX());
}

uint8_t EightBit::MOS6502::AM_ZeroPageY() noexcept {
	return memoryRead(Address_ZeroPageY());
}

uint8_t EightBit::MOS6502::AM_IndexedIndirectX() noexcept {
	return memoryRead(Address_IndexedIndirectX());
}

uint8_t EightBit::MOS6502::AM_IndirectIndexedY() noexcept {
	const auto [address, page] = Address_IndirectIndexedY();
	auto possible = getBytePaged(page, address.low);
	if (page != address.high)
		possible = memoryRead(address);
	return possible;
}

////

void EightBit::MOS6502::branch(const int condition) noexcept {
	const auto destination = Address_relative_byte();
	if (condition) {
		memoryRead(PC());
		const auto page = PC().high;
		jump(destination);
		if (UNLIKELY(PC().high != page))
			getBytePaged(page, PC().low);
	}
}

////

uint8_t EightBit::MOS6502::sbc(const uint8_t operand, const uint8_t data) noexcept {

	const auto returned = sub(operand, data, carry(~P()));

	const auto difference = m_intermediate;
	adjustNZ(difference.low);
	P() = setBit(P(), VF, (operand ^ data) & (operand ^ difference.low) & NF);
	P() = clearBit(P(), CF, difference.high);

	return returned;
}

uint8_t EightBit::MOS6502::sub(const uint8_t operand, const uint8_t data, const int borrow) noexcept {
	return decimal() ? sub_d(operand, data, borrow) : sub_b(operand, data, borrow);
}

uint8_t EightBit::MOS6502::sub_b(const uint8_t operand, const uint8_t data, const int borrow) noexcept {
	m_intermediate.word = operand - data - borrow;
	return m_intermediate.low;
}

uint8_t EightBit::MOS6502::sub_d(const uint8_t operand, const uint8_t data, const int borrow) noexcept {
	m_intermediate.word = operand - data - borrow;

	uint8_t low = lowNibble(operand) - lowNibble(data) - borrow;
	const auto lowNegative = negative(low);
	if (lowNegative)
		low -= 6;

	uint8_t high = highNibble(operand) - highNibble(data) - (lowNegative >> 7);
	const auto highNegative = negative(high);
	if (highNegative)
		high -= 6;

	return promoteNibble(high) | lowNibble(low);
}

uint8_t EightBit::MOS6502::adc(const uint8_t operand, const uint8_t data) noexcept {
	return add(operand, data, carry());
}

uint8_t EightBit::MOS6502::add(uint8_t operand, uint8_t data, int carrying) noexcept {
	return decimal() ? add_d(operand, data, carrying) : add_b(operand, data, carrying);
}

uint8_t EightBit::MOS6502::add_b(uint8_t operand, uint8_t data, int carrying) noexcept {
	m_intermediate.word = operand + data + carrying;

	P() = setBit(P(), VF, ~(operand ^ data) & (operand ^ m_intermediate.low) & NF);
	P() = setBit(P(), CF, carry(m_intermediate.high));

	adjustNZ(m_intermediate.low);

	return m_intermediate.low;
}

uint8_t EightBit::MOS6502::add_d(uint8_t operand, uint8_t data, int carry) noexcept {

	register16_t low = lowerNibble(operand) + lowerNibble(data) + carry;
	register16_t high = higherNibble(operand) + higherNibble(data);

	P() = clearBit(P(), ZF, (low + high).low);

	if (low.word > 0x09) {
		high += 0x10;
		low += 0x06;
	}

	adjustNegative(high.low);
	P() = setBit(P(), VF, ~(operand ^ data) & (operand ^ high.low) & NF);

	if (high.word > 0x90)
		high += 0x60;

	P() = setBit(P(), CF, high.high);

	return lowerNibble(low.low) | higherNibble(high.low);
}

uint8_t EightBit::MOS6502::andr(const uint8_t operand, const uint8_t data) noexcept {
	return through(operand & data);
}

uint8_t EightBit::MOS6502::asl(const uint8_t value) noexcept {
	P() = setBit(P(), CF, value & Bit7);
	return through(value << 1);
}

void EightBit::MOS6502::bit(const uint8_t operand, const uint8_t data) noexcept {
	P() = setBit(P(), VF, overflow(data));
	adjustZero(operand & data);
	adjustNegative(data);
}

void EightBit::MOS6502::cmp(const uint8_t first, const uint8_t second) noexcept {
	const register16_t result = first - second;
	adjustNZ(result.low);
	P() = clearBit(P(), CF, result.high);
}

uint8_t EightBit::MOS6502::dec(const uint8_t value) noexcept {
	return through(value - 1);
}

uint8_t EightBit::MOS6502::eorr(const uint8_t operand, const uint8_t data) noexcept {
	return through(operand ^ data);
}

uint8_t EightBit::MOS6502::inc(const uint8_t value) noexcept {
	return through(value + 1);
}

void EightBit::MOS6502::jsr() noexcept {
	const auto low = fetchByte();
	getBytePaged(1, S()); // dummy read
	pushWord(PC());
	PC().high = fetchByte();
	PC().low = low;
}

uint8_t EightBit::MOS6502::lsr(const uint8_t value) noexcept {
	P() = setBit(P(), CF, value & Bit0);
	return through(value >> 1);
}

uint8_t EightBit::MOS6502::orr(const uint8_t operand, const uint8_t data) noexcept {
	return through(operand | data);
}

void EightBit::MOS6502::php() noexcept {
	push(P() | BF);
}

void EightBit::MOS6502::plp() noexcept {
	P() = (pop() | RF) & ~BF;
}

uint8_t EightBit::MOS6502::rol(const uint8_t operand) noexcept {
	const auto carryIn = carry();
	P() = setBit(P(), CF, operand & Bit7);
	const uint8_t result = (operand << 1) | carryIn;
	return through(result);
}

uint8_t EightBit::MOS6502::ror(const uint8_t operand) noexcept {
	const auto carryIn = carry();
	P() = setBit(P(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | (carryIn << 7);
	return through(result);
}

void EightBit::MOS6502::rti() noexcept {
	getBytePaged(1, S()); // dummy read
	plp();
	ret();
}

void EightBit::MOS6502::rts() noexcept {
	getBytePaged(1, S()); // dummy read
	ret();
	fetchByte();
}

// Undocumented compound instructions

void EightBit::MOS6502::anc(const uint8_t value) noexcept {
	A() = andr(A(), value);
	P() = setBit(P(), CF, A() & Bit7);
}

void EightBit::MOS6502::arr(const uint8_t value) noexcept {
	decimal() ? arr_d(value) : arr_b(value);
}

void EightBit::MOS6502::arr_d(const uint8_t value) noexcept {

	// With thanks to https://github.com/TomHarte/CLK
	// What a very strange instruction ARR is...

	A() &= value;
	auto unshiftedA = A();
	A() = through((A() >> 1) | (carry() << 7));
	P() = setBit(P(), VF, (A() ^ (A() << 1)) & VF);

	if (lowerNibble(unshiftedA) + (unshiftedA & 0x1) > 5)
		A() = lowerNibble(A() + 6) | higherNibble(A());

	P() = setBit(P(), CF, higherNibble(unshiftedA) + (unshiftedA & 0x10) > 0x50);
	
	if (carry())
		A() += 0x60;
}

void EightBit::MOS6502::arr_b(const uint8_t value) noexcept {
	A() &= value;
	A() = through((A() >> 1) | (carry() << 7));
	P() = setBit(P(), CF, A() & Bit6);
	P() = setBit(P(), VF, (A() ^ (A() << 1)) & VF);
}

void EightBit::MOS6502::asr(const uint8_t value) noexcept {
	A() = andr(A(), value);
	A() = lsr(A());
}

void EightBit::MOS6502::axs(const uint8_t value) noexcept {
	X() = through(sub_b(A() & X(), value));
	P() = clearBit(P(), CF, m_intermediate.high);
}

void EightBit::MOS6502::dcp(const uint8_t value) noexcept {
	memoryReadModifyWrite(dec(value));
	cmp(A(), BUS().DATA());
}

void EightBit::MOS6502::isb(const uint8_t value) noexcept {
	memoryReadModifyWrite(inc(value));
	A() = sbc(A(), BUS().DATA());
}

void EightBit::MOS6502::rla(const uint8_t value) noexcept {
	memoryReadModifyWrite(rol(value));
	A() = andr(A(), BUS().DATA());
}

void EightBit::MOS6502::rra(const uint8_t value) noexcept {
	memoryReadModifyWrite(ror(value));
	A() = adc(A(), BUS().DATA());
}

void EightBit::MOS6502::slo(const uint8_t value) noexcept {
	memoryReadModifyWrite(asl(value));
	A() = orr(A(), BUS().DATA());
}

void EightBit::MOS6502::sre(const uint8_t value) noexcept {
	memoryReadModifyWrite(lsr(value));
	A() = eorr(A(), BUS().DATA());
}

void EightBit::MOS6502::jam() noexcept {
	memoryRead(PC());
	memoryRead(PC()--);
}

//

void EightBit::MOS6502::sta_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	sta_with_fixup(address, page);
}

void EightBit::MOS6502::sta_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	sta_with_fixup(address, page);
}

void EightBit::MOS6502::sta_IndirectIndexedY() noexcept {
	const auto [address, page] = Address_IndirectIndexedY();
	sta_with_fixup(address, page);
}

//

void EightBit::MOS6502::slo_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	slo_with_fixup(address, page);
}

void EightBit::MOS6502::slo_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	slo_with_fixup(address, page);
}

void EightBit::MOS6502::slo_IndirectIndexedY() noexcept {
	const auto [address, page] = Address_IndirectIndexedY();
	slo_with_fixup(address, page);
}

void EightBit::MOS6502::isb_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	isb_with_fixup(address, page);
}

void EightBit::MOS6502::isb_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	isb_with_fixup(address, page);
}

void EightBit::MOS6502::isb_IndirectIndexedY() noexcept {
	const auto [address, page] = Address_IndirectIndexedY();
	isb_with_fixup(address, page);
}

void EightBit::MOS6502::rla_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	rla_with_fixup(address, page);
}

void EightBit::MOS6502::rla_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	rla_with_fixup(address, page);
}

void EightBit::MOS6502::rla_IndirectIndexedY() noexcept {
	const auto [address, page] = Address_IndirectIndexedY();
	rla_with_fixup(address, page);
}

void EightBit::MOS6502::rra_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	rra_with_fixup(address, page);
}

void EightBit::MOS6502::rra_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	rra_with_fixup(address, page);
}

void EightBit::MOS6502::rra_IndirectIndexedY() noexcept {
	const auto [address, page] = Address_IndirectIndexedY();
	rra_with_fixup(address, page);
}

void EightBit::MOS6502::dcp_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	dcp_with_fixup(address, page);
}

void EightBit::MOS6502::dcp_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	dcp_with_fixup(address, page);
}

void EightBit::MOS6502::dcp_IndirectIndexedY() noexcept {
	const auto [address, page] = Address_IndirectIndexedY();
	dcp_with_fixup(address, page);
}

void EightBit::MOS6502::sre_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	sre_with_fixup(address, page);
}

void EightBit::MOS6502::sre_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	sre_with_fixup(address, page);
}

void EightBit::MOS6502::sre_IndirectIndexedY() noexcept {
	const auto [address, page] = Address_IndirectIndexedY();
	sre_with_fixup(address, page);
}

void EightBit::MOS6502::sya_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	fixup(address, page);
	memoryWrite(Y() & (address.high + 1));
}

void EightBit::MOS6502::sxa_AbsoluteY() noexcept {
	const auto [address, page] = Address_AbsoluteY();
	fixup(address, page);
	memoryWrite(X() & (address.high + 1));
}

void EightBit::MOS6502::nop_AbsoluteX() noexcept {
	const auto [address, page] = Address_AbsoluteX();
	fixup(address, page);
}
