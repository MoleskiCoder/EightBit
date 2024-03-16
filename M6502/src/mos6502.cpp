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

		tick();	// A cycle is used, whether RDY is high or not

		if (UNLIKELY(lowered(SO())))
			handleSO();

		if (LIKELY(raised(RDY()))) {

			lowerSYNC();	// Instruction fetch beginning

			// Read the opcode within the existing cycle
			assert(cycles() == 1 && "An extra cycle has occurred");
			// Can't use fetchByte, since that would add an extra tick.
			raiseRW();
			opcode() = BUS().read(PC()++);
			assert(cycles() == 1 && "BUS read has introduced stray cycles");

			// Priority: RESET > NMI > INT
			if (UNLIKELY(lowered(RESET())))
				handleRESET();
			else if (UNLIKELY(lowered(NMI())))
				handleNMI();
			else if (UNLIKELY(lowered(INT()) && !interruptMasked()))
				handleINT();

			// Instruction fetch has now completed
			raiseSYNC();

			// Whatever opcode is available, execute it.
			execute();
		}
	}
	ExecutedInstruction.fire(*this);

	ASSUME(cycles() > 0);
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
	set_flag(IF);	// Disable IRQ
	const uint8_t vector = reset ? RSTvector : (nmi ? NMIvector : IRQvector);
	jump(Processor::getWordPaged(0xff, vector));
	m_handlingRESET = m_handlingNMI = m_handlingINT = false;
}

//

void EightBit::MOS6502::busWrite() noexcept {
	tick();
	lowerRW();
	base::busWrite();
}

uint8_t EightBit::MOS6502::busRead() noexcept {
	tick();
	raiseRW();
	return base::busRead();
}

//

void EightBit::MOS6502::execute() noexcept {

	switch (opcode()) {

	case 0x00:	swallow_fetch(); interrupt();								break;	// BRK (implied)
	case 0x01:	AM_IndexedIndirectX(); orr();								break;	// ORA (indexed indirect X)
	case 0x02:	jam();														break;	// *JAM
	case 0x03:	AM_IndexedIndirectX(); slo();								break;	// *SLO (indexed indirect X)
	case 0x04:	AM_ZeroPage();												break;	// *NOP (zero page)
	case 0x05:	AM_ZeroPage(); orr();										break;	// ORA (zero page)
	case 0x06:	AM_ZeroPage(); MW(asl);										break;	// ASL (zero page)
	case 0x07:	AM_ZeroPage(); slo();										break;	// *SLO (zero page)
	case 0x08:	swallow(); php();											break;	// PHP (implied)
	case 0x09:	AM_Immediate(); orr();										break;	// ORA (immediate)
	case 0x0a:	swallow(); A() = asl(A());									break;	// ASL A (implied)
	case 0x0b:	AM_Immediate(); anc();										break;	// *ANC (immediate)
	case 0x0c:	Address_Absolute(); 										break;	// *NOP (absolute)
	case 0x0d:	AM_Absolute(); orr();										break;	// ORA (absolute)
	case 0x0e:	AM_Absolute(); MW(asl);										break;	// ASL (absolute)
	case 0x0f:	AM_Absolute(); slo();										break;	// *SLO (absolute)

	case 0x10:	branch(negative() == 0);									break;	// BPL (relative)
	case 0x11:	AM_IndirectIndexedY(); orr();								break;	// ORA (indirect indexed Y)
	case 0x12:	jam();														break;	// *JAM
	case 0x13:	Address_IndirectIndexedY(); fixupR(); slo();				break;	// *SLO (indirect indexed Y)
	case 0x14:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x15:	AM_ZeroPageX(); orr();										break;	// ORA (zero page, X)
	case 0x16:	AM_ZeroPageX(); MW(asl);									break;	// ASL (zero page, X)
	case 0x17:	AM_ZeroPageX(); slo();										break;	// *SLO (zero page, X)
	case 0x18:	swallow(); reset_flag(CF);									break;	// CLC (implied)
	case 0x19:	AM_AbsoluteY(); orr();										break;	// ORA (absolute, Y)
	case 0x1a:	swallow();													break;	// *NOP (implied)
	case 0x1b:	Address_AbsoluteY(); fixupR(); slo();						break;	// *SLO (absolute, Y)
	case 0x1c:	Address_AbsoluteX(); fixup();								break;	// *NOP (absolute, X)
	case 0x1d:	AM_AbsoluteX(); orr();										break;	// ORA (absolute, X)
	case 0x1e:	Address_AbsoluteX(); fixupR(); MW(asl);						break;	// ASL (absolute, X)
	case 0x1f:	Address_AbsoluteX(); fixupR(); slo();						break;	// *SLO (absolute, X)

	case 0x20:	jsr();														break;	// JSR (absolute)
	case 0x21:	AM_IndexedIndirectX(); andr();								break;	// AND (indexed indirect X)
	case 0x22:	jam();														break;	// *JAM
	case 0x23:	AM_IndexedIndirectX(); rla();;								break;	// *RLA (indexed indirect X)
	case 0x24:	AM_ZeroPage(); bit();										break;	// BIT (zero page)
	case 0x25:	AM_ZeroPage(); andr();										break;	// AND (zero page)
	case 0x26:	AM_ZeroPage(); MW(rol);										break;	// ROL (zero page)
	case 0x27:	AM_ZeroPage(); rla();;										break;	// *RLA (zero page)
	case 0x28:	swallow(); plp();											break;	// PLP (implied)
	case 0x29:	AM_Immediate(); andr();										break;	// AND (immediate)
	case 0x2a:	swallow(); A() = rol(A());									break;	// ROL A (implied)
	case 0x2b:	AM_Immediate(); anc();										break;	// *ANC (immediate)
	case 0x2c:	AM_Absolute(); bit();										break;	// BIT (absolute)
	case 0x2d:	AM_Absolute(); andr();										break;	// AND (absolute)
	case 0x2e:	AM_Absolute(); MW(rol);										break;	// ROL (absolute)
	case 0x2f:	AM_Absolute(); rla();;										break;	// *RLA (absolute)

	case 0x30:	branch(negative());											break;	// BMI (relative)
	case 0x31:	AM_IndirectIndexedY(); andr();								break;	// AND (indirect indexed Y)
	case 0x32:	jam();														break;	// *JAM
	case 0x33:	Address_IndirectIndexedY(); fixupR(); rla();;				break;	// *RLA (indirect indexed Y)
	case 0x34:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x35:	AM_ZeroPageX(); andr();										break;	// AND (zero page, X)
	case 0x36:	AM_ZeroPageX(); MW(rol);									break;	// ROL (zero page, X)
	case 0x37:	AM_ZeroPageX(); rla();;										break;	// *RLA (zero page, X)
	case 0x38:	swallow(); set_flag(CF);									break;	// SEC (implied)
	case 0x39:	AM_AbsoluteY(); andr();										break;	// AND (absolute, Y)
	case 0x3a:	swallow();													break;	// *NOP (implied)
	case 0x3b:	Address_AbsoluteY(); fixupR(); rla();;						break;	// *RLA (absolute, Y)
	case 0x3c:	Address_AbsoluteX(); fixup();								break;	// *NOP (absolute, X)
	case 0x3d:	AM_AbsoluteX(); andr();										break;	// AND (absolute, X)
	case 0x3e:	Address_AbsoluteX(); fixupR(); MW(rol);						break;	// ROL (absolute, X)
	case 0x3f:	Address_AbsoluteX(); fixupR(); rla();;						break;	// *RLA (absolute, X)

	case 0x40:	swallow(); rti();											break;	// RTI (implied)
	case 0x41:	AM_IndexedIndirectX(); eorr();								break;	// EOR (indexed indirect X)
	case 0x42:	jam();														break;	// *JAM
	case 0x43:	AM_IndexedIndirectX(); sre();								break;	// *SRE (indexed indirect X)
	case 0x44:	AM_ZeroPage();												break;	// *NOP (zero page)
	case 0x45:	AM_ZeroPage(); eorr();										break;	// EOR (zero page)
	case 0x46:	AM_ZeroPage(); MW(lsr);										break;	// LSR (zero page)
	case 0x47:	AM_ZeroPage(); sre();										break;	// *SRE (zero page)
	case 0x48:	swallow(); push(A());										break;	// PHA (implied)
	case 0x49:	AM_Immediate(); eorr();										break;	// EOR (immediate)
	case 0x4a:	swallow(); A() = lsr(A());									break;	// LSR A (implied)
	case 0x4b:	AM_Immediate(); asr();										break;	// *ASR (immediate)
	case 0x4c:	Address_Absolute(); jump(BUS().ADDRESS());					break;	// JMP (absolute)
	case 0x4d:	AM_Absolute(); eorr();										break;	// EOR (absolute)
	case 0x4e:	AM_Absolute(); MW(lsr); 									break;	// LSR (absolute)
	case 0x4f:	AM_Absolute(); sre();										break;	// *SRE (absolute)

	case 0x50:	branch(overflow() == 0);									break;	// BVC (relative)
	case 0x51:	AM_IndirectIndexedY(); eorr();								break;	// EOR (indirect indexed Y)
	case 0x52:	jam();														break;	// *JAM
	case 0x53:	Address_IndirectIndexedY(); fixupR(); sre();				break;	// *SRE (indirect indexed Y)
	case 0x54:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x55:	AM_ZeroPageX(); eorr();										break;	// EOR (zero page, X)
	case 0x56:	AM_ZeroPageX(); MW(lsr); 									break;	// LSR (zero page, X)
	case 0x57:	AM_ZeroPageX(); sre();										break;	// *SRE (zero page, X)
	case 0x58:	swallow(); reset_flag(IF);									break;	// CLI (implied)
	case 0x59:	AM_AbsoluteY(); eorr();										break;	// EOR (absolute, Y)
	case 0x5a:	swallow();													break;	// *NOP (implied)
	case 0x5b:	Address_AbsoluteY(); fixupR(); sre();						break;	// *SRE (absolute, Y)
	case 0x5c:	Address_AbsoluteX(); fixup();								break;	// *NOP (absolute, X)
	case 0x5d:	AM_AbsoluteX(); eorr();										break;	// EOR (absolute, X)
	case 0x5e:	Address_AbsoluteX(); fixupR(); MW(lsr);						break;	// LSR (absolute, X)
	case 0x5f:	Address_AbsoluteX(); fixupR(); sre();						break;	// *SRE (absolute, X)

	case 0x60:	swallow(); rts();											break;	// RTS (implied)
	case 0x61:	AM_IndexedIndirectX(); adc();								break;	// ADC (indexed indirect X)
	case 0x62:	jam();														break;	// *JAM
	case 0x63:	AM_IndexedIndirectX(); rra();								break;	// *RRA (indexed indirect X)
	case 0x64:	AM_ZeroPage();												break;	// *NOP (zero page)
	case 0x65:	AM_ZeroPage(); adc();										break;	// ADC (zero page)
	case 0x66:	AM_ZeroPage(); MW(ror);										break;	// ROR (zero page)
	case 0x67:	AM_ZeroPage(); rra();										break;	// *RRA (zero page)
	case 0x68:	swallow(); swallow_stack(); A() = through(pop());			break;	// PLA (implied)
	case 0x69:	AM_Immediate(); adc();										break;	// ADC (immediate)
	case 0x6a:	swallow(); A() = ror(A());									break;	// ROR A (implied)
	case 0x6b:	AM_Immediate(); arr();										break;	// *ARR (immediate)
	case 0x6c:	Address_Indirect(); jump(BUS().ADDRESS());					break;	// JMP (indirect)
	case 0x6d:	AM_Absolute(); adc();										break;	// ADC (absolute)
	case 0x6e:	AM_Absolute(); MW(ror); 									break;	// ROR (absolute)
	case 0x6f:	AM_Absolute(); rra();										break;	// *RRA (absolute)

	case 0x70:	branch(overflow());											break;	// BVS (relative)
	case 0x71:	AM_IndirectIndexedY(); adc();								break;	// ADC (indirect indexed Y)
	case 0x72:	jam();														break;	// *JAM
	case 0x73:	Address_IndirectIndexedY(); fixupR(); rra();				break;	// *RRA (indirect indexed Y)
	case 0x74:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0x75:	AM_ZeroPageX(); adc();										break;	// ADC (zero page, X)
	case 0x76:	AM_ZeroPageX(); MW(ror); 									break;	// ROR (zero page, X)
	case 0x77:	AM_ZeroPageX(); rra();										break;	// *RRA (zero page, X)
	case 0x78:	swallow(); set_flag(IF);									break;	// SEI (implied)
	case 0x79:	AM_AbsoluteY(); adc();										break;	// ADC (absolute, Y)
	case 0x7a:	swallow();													break;	// *NOP (implied)
	case 0x7b:	Address_AbsoluteY(); fixupR(); rra();						break;	// *RRA (absolute, Y)
	case 0x7c:	Address_AbsoluteX(); fixup();								break;	// *NOP (absolute, X)
	case 0x7d:	AM_AbsoluteX(); adc();										break;	// ADC (absolute, X)
	case 0x7e:	Address_AbsoluteX(); fixupR(); MW(ror);						break;	// ROR (absolute, X)
	case 0x7f:	Address_AbsoluteX(); fixupR(); rra();						break;	// *RRA (absolute, X)

	case 0x80:	AM_Immediate();												break;	// *NOP (immediate)
	case 0x81:	Address_IndexedIndirectX(); memoryWrite(A());				break;	// STA (indexed indirect X)
	case 0x82:	AM_Immediate();												break;	// *NOP (immediate)
	case 0x83:	Address_IndexedIndirectX(); memoryWrite(A() & X());			break;	// *SAX (indexed indirect X)
	case 0x84:	Address_ZeroPage(); memoryWrite(Y());						break;	// STY (zero page)
	case 0x85:	Address_ZeroPage(); memoryWrite(A());						break;	// STA (zero page)
	case 0x86:	Address_ZeroPage(); memoryWrite(X());						break;	// STX (zero page)
	case 0x87:	Address_ZeroPage(); memoryWrite(A() & X());					break;	// *SAX (zero page)
	case 0x88:	swallow(); Y() = dec(Y());									break;	// DEY (implied)
	case 0x89:	AM_Immediate();												break;	// *NOP (immediate)
	case 0x8a:	swallow(); A() = through(X());								break;	// TXA (implied)
	case 0x8b:	AM_Immediate(); ane();										break;	// *ANE (immediate)
	case 0x8c:	Address_Absolute(); memoryWrite(Y());						break;	// STY (absolute)
	case 0x8d:	Address_Absolute(); memoryWrite(A());						break;	// STA (absolute)
	case 0x8e:	Address_Absolute(); memoryWrite(X());						break;	// STX (absolute)
	case 0x8f:	Address_Absolute(); memoryWrite(A() & X());					break;	// *SAX (absolute)

	case 0x90:	branch(carry() == 0);										break;	// BCC (relative)
	case 0x91:	Address_IndirectIndexedY(); fixup(); memoryWrite(A());		break;	// STA (indirect indexed Y)
	case 0x92:	jam();														break;	// *JAM
	case 0x93:	Address_IndirectIndexedY(); fixup(); sha();					break;	// *SHA (indirect indexed, Y)
	case 0x94:	Address_ZeroPageX(); memoryWrite(Y());						break;	// STY (zero page, X)
	case 0x95:	Address_ZeroPageX(); memoryWrite(A());						break;	// STA (zero page, X)
	case 0x96:	Address_ZeroPageY(); memoryWrite(X());						break;	// STX (zero page, Y)
	case 0x97:	Address_ZeroPageY(); memoryWrite(A() & X());				break;	// *SAX (zero page, Y)
	case 0x98:	swallow(); A() = through(Y());								break;	// TYA (implied)
	case 0x99:	Address_AbsoluteY(); fixup(); memoryWrite(A());				break;	// STA (absolute, Y)
	case 0x9a:	swallow(); S() = X();										break;	// TXS (implied)
	case 0x9b:	Address_AbsoluteY(); fixup(); tas();						break;	// *TAS (absolute, Y)
	case 0x9c:	Address_AbsoluteX(); fixup(); sya();						break;	// *SYA (absolute, X)
	case 0x9d:	Address_AbsoluteX(); fixup(); memoryWrite(A());				break;	// STA (absolute, X)
	case 0x9e:	Address_AbsoluteY(); fixup(); sxa();						break;	// *SXA (absolute, Y)
	case 0x9f:	Address_AbsoluteY(); fixup(); sha();						break;	// *SHA (absolute, Y)

	case 0xa0:	AM_Immediate(); Y() = through(BUS().DATA());				break;	// LDY (immediate)
	case 0xa1:	AM_IndexedIndirectX(); A() = through(BUS().DATA());			break;	// LDA (indexed indirect X)
	case 0xa2:	AM_Immediate(); X() = through(BUS().DATA());				break;	// LDX (immediate)
	case 0xa3:	AM_IndexedIndirectX(); A() = X() = through(BUS().DATA());	break;	// *LAX (indexed indirect X)
	case 0xa4:	AM_ZeroPage(); Y() = through(BUS().DATA());					break;	// LDY (zero page)
	case 0xa5:	AM_ZeroPage(); A() = through(BUS().DATA());					break;	// LDA (zero page)
	case 0xa6:	AM_ZeroPage(); X() = through(BUS().DATA());					break;	// LDX (zero page)
	case 0xa7:	AM_ZeroPage(); A() = X() = through(BUS().DATA());			break;	// *LAX (zero page)
	case 0xa8:	swallow(); Y() = through(A());								break;	// TAY (implied)
	case 0xa9:	AM_Immediate(); A() = through(BUS().DATA());				break;	// LDA (immediate)
	case 0xaa:	swallow(); X() = through(A());								break;	// TAX (implied)
	case 0xab:	AM_Immediate(); atx();										break;	// *ATX (immediate)
	case 0xac:	AM_Absolute(); Y() = through(BUS().DATA());					break;	// LDY (absolute)
	case 0xad:	AM_Absolute(); A() = through(BUS().DATA());					break;	// LDA (absolute)
	case 0xae:	AM_Absolute(); X() = through(BUS().DATA());					break;	// LDX (absolute)
	case 0xaf:	AM_Absolute(); A() = X() = through(BUS().DATA());			break;	// *LAX (absolute)

	case 0xb0:	branch(carry());											break;	// BCS (relative)
	case 0xb1:	AM_IndirectIndexedY(); A() = through(BUS().DATA());			break;	// LDA (indirect indexed Y)
	case 0xb2:	jam();														break;	// *JAM
	case 0xb3:	AM_IndirectIndexedY(); A() = X() = through(BUS().DATA());	break;	// *LAX (indirect indexed Y)
	case 0xb4:	AM_ZeroPageX(); Y() = through(BUS().DATA());				break;	// LDY (zero page, X)
	case 0xb5:	AM_ZeroPageX(); A() = through(BUS().DATA());				break;	// LDA (zero page, X)
	case 0xb6:	AM_ZeroPageY(); X() = through(BUS().DATA());				break;	// LDX (zero page, Y)
	case 0xb7:	AM_ZeroPageY(); A() = X() = through(BUS().DATA());			break;	// *LAX (zero page, Y)
	case 0xb8:	swallow(); reset_flag(VF);									break;	// CLV (implied)
	case 0xb9:	AM_AbsoluteY(); A() = through(BUS().DATA());				break;	// LDA (absolute, Y)
	case 0xba:	swallow(); X() = through(S());								break;	// TSX (implied)
	case 0xbb:	Address_AbsoluteY(); maybe_fixup(); las();					break;	// *LAS (absolute, Y)
	case 0xbc:	AM_AbsoluteX(); Y() = through(BUS().DATA());				break;	// LDY (absolute, X)
	case 0xbd:	AM_AbsoluteX(); A() = through(BUS().DATA());				break;	// LDA (absolute, X)
	case 0xbe:	AM_AbsoluteY(); X() = through(BUS().DATA());				break;	// LDX (absolute, Y)
	case 0xbf:	AM_AbsoluteY(); A() = X() = through(BUS().DATA());			break;	// *LAX (absolute, Y)

	case 0xc0:	AM_Immediate(); cmp(Y());									break;	// CPY (immediate)
	case 0xc1:	AM_IndexedIndirectX(); cmp(A());							break;	// CMP (indexed indirect X)
	case 0xc2:	AM_Immediate();												break;	// *NOP (immediate)
	case 0xc3:	AM_IndexedIndirectX(); dcp();								break;	// *DCP (indexed indirect X)
	case 0xc4:	AM_ZeroPage(); cmp(Y());									break;	// CPY (zero page)
	case 0xc5:	AM_ZeroPage(); cmp(A());									break;	// CMP (zero page)
	case 0xc6:	AM_ZeroPage(); MW(dec);										break;	// DEC (zero page)
	case 0xc7:	AM_ZeroPage(); dcp();										break;	// *DCP (zero page)
	case 0xc8:	swallow(); Y() = inc(Y());									break;	// INY (implied)
	case 0xc9:	AM_Immediate(); cmp(A());									break;	// CMP (immediate)
	case 0xca:	swallow(); X() = dec(X());									break;	// DEX (implied)
	case 0xcb:	AM_Immediate(); axs();										break;	// *AXS (immediate)
	case 0xcc:	AM_Absolute(); cmp(Y());									break;	// CPY (absolute)
	case 0xcd:	AM_Absolute(); cmp(A());									break;	// CMP (absolute)
	case 0xce:	AM_Absolute(); MW(dec);										break;	// DEC (absolute)
	case 0xcf:	AM_Absolute(); dcp();										break;	// *DCP (absolute)

	case 0xd0:	branch(zero() == 0);										break;	// BNE (relative)
	case 0xd1:	AM_IndirectIndexedY(); cmp(A());							break;	// CMP (indirect indexed Y)
	case 0xd2:	jam();														break;	// *JAM
	case 0xd3:	Address_IndirectIndexedY(); fixupR(); dcp();				break;	// *DCP (indirect indexed Y)
	case 0xd4:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0xd5:	AM_ZeroPageX(); cmp(A());									break;	// CMP (zero page, X)
	case 0xd6:	AM_ZeroPageX(); MW(dec);									break;	// DEC (zero page, X)
	case 0xd7:	AM_ZeroPageX(); dcp();										break;	// *DCP (zero page, X)
	case 0xd8:	swallow(); reset_flag(DF);									break;	// CLD (implied)
	case 0xd9:	AM_AbsoluteY(); cmp(A());									break;	// CMP (absolute, Y)
	case 0xda:	swallow();													break;	// *NOP (implied)
	case 0xdb:	Address_AbsoluteY(); fixupR(); dcp();						break;	// *DCP (absolute, Y)
	case 0xdc:	Address_AbsoluteX(); fixup();								break;	// *NOP (absolute, X)
	case 0xdd:	AM_AbsoluteX(); cmp(A());									break;	// CMP (absolute, X)
	case 0xde:	Address_AbsoluteX(); fixupR(); MW(dec);						break;	// DEC (absolute, X)
	case 0xdf:	Address_AbsoluteX(); fixupR(); dcp();						break;	// *DCP (absolute, X)

	case 0xe0:	AM_Immediate(); cmp(X());									break;	// CPX (immediate)
	case 0xe1:	AM_IndexedIndirectX(); sbc();								break;	// SBC (indexed indirect X)
	case 0xe2:	AM_Immediate();												break;	// *NOP (immediate)
	case 0xe3:	AM_IndexedIndirectX(); isb();								break;	// *ISB (indexed indirect X)
	case 0xe4:	AM_ZeroPage(); cmp(X());									break;	// CPX (zero page)
	case 0xe5:	AM_ZeroPage(); sbc();										break;	// SBC (zero page)
	case 0xe6:	AM_ZeroPage(); MW(inc);										break;	// INC (zero page)
	case 0xe7:	AM_ZeroPage(); isb();										break;	// *ISB (zero page)
	case 0xe8:	swallow(); X() = inc(X());									break;	// INX (implied)
	case 0xe9:	AM_Immediate(); sbc();										break;	// SBC (immediate)
	case 0xea:	swallow();													break;	// NOP (implied)
	case 0xeb:	AM_Immediate(); sbc();										break;	// *SBC (immediate)
	case 0xec:	AM_Absolute(); cmp(X());									break;	// CPX (absolute)
	case 0xed:	AM_Absolute(); sbc();										break;	// SBC (absolute)
	case 0xee:	AM_Absolute(); MW(inc);										break;	// INC (absolute)
	case 0xef:	AM_Absolute(); isb();										break;	// *ISB (absolute)

	case 0xf0:	branch(zero());												break;	// BEQ (relative)
	case 0xf1:	AM_IndirectIndexedY(); sbc();								break;	// SBC (indirect indexed Y)
	case 0xf2:	jam();														break;	// *JAM
	case 0xf3:	Address_IndirectIndexedY(); fixupR(); isb();				break;	// *ISB (indirect indexed Y)
	case 0xf4:	AM_ZeroPageX();												break;	// *NOP (zero page, X)
	case 0xf5:	AM_ZeroPageX(); sbc();										break;	// SBC (zero page, X)
	case 0xf6:	AM_ZeroPageX(); MW(inc);									break;	// INC (zero page, X)
	case 0xf7:	AM_ZeroPageX(); isb();										break;	// *ISB (zero page, X)
	case 0xf8:	swallow(); set_flag(DF);									break;	// SED (implied)
	case 0xf9:	AM_AbsoluteY(); sbc();										break;	// SBC (absolute, Y)
	case 0xfa:	swallow();													break;	// *NOP (implied)
	case 0xfb:	Address_AbsoluteY(); fixupR(); isb();						break;	// *ISB (absolute, Y)
	case 0xfc:	Address_AbsoluteX(); fixup();								break;	// *NOP (absolute, X)
	case 0xfd:	AM_AbsoluteX(); sbc();										break;	// SBC (absolute, X)
	case 0xfe:	Address_AbsoluteX(); fixupR(); MW(inc);						break;	// INC (absolute, X)
	case 0xff:	Address_AbsoluteX(); fixupR(); isb();						break;	// *ISB (absolute, X)
	}
}

////

void EightBit::MOS6502::push(uint8_t value) noexcept {
	pushDownStackAddress(value);
	memoryWrite();
}

uint8_t EightBit::MOS6502::pop() noexcept {
	popUpStackAddress();
	return memoryRead();
}

void EightBit::MOS6502::dummyPush(uint8_t value) noexcept {
	pushDownStackAddress(value);
	tick();	// In place of the memory write
}

////

void EightBit::MOS6502::branch(const int condition) noexcept {
	const auto relative = int8_t(fetchByte());
	if (condition) {
		swallow();
		m_unfixed_page = PC().high;
		jump(PC() + relative);
		BUS().ADDRESS() = PC();
		maybe_fixup();
	}
}

////

void EightBit::MOS6502::sbc() noexcept {

	const auto operand = A();
	A() = sub(operand, carry(~P()));

	const auto difference = m_intermediate;
	adjustOverflow_subtract(operand, BUS().DATA(), difference.low);
	adjustNZ(difference.low);
	reset_flag(CF, difference.high);
}

uint8_t EightBit::MOS6502::sub(const uint8_t operand, const int borrow) noexcept {
	const auto data = BUS().DATA();
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

void EightBit::MOS6502::adc() noexcept {
	A() = add(A(), carry());
}

uint8_t EightBit::MOS6502::add(uint8_t operand, int carrying) noexcept {
	const auto data = BUS().DATA();
	return decimal() ? add_d(operand, data, carrying) : add_b(operand, data, carrying);
}

uint8_t EightBit::MOS6502::add_b(uint8_t operand, uint8_t data, int carrying) noexcept {
	m_intermediate.word = operand + data + carrying;

	adjustOverflow_add(operand, data, m_intermediate.low);
	set_flag(CF, carry(m_intermediate.high));

	adjustNZ(m_intermediate.low);

	return m_intermediate.low;
}

uint8_t EightBit::MOS6502::add_d(uint8_t operand, uint8_t data, int carry) noexcept {

	register16_t low = lowerNibble(operand) + lowerNibble(data) + carry;
	register16_t high = higherNibble(operand) + higherNibble(data);

	adjustZero((low + high).low);

	if (low.word > 0x09) {
		high += 0x10;
		low += 0x06;
	}

	adjustNegative(high.low);
	adjustOverflow_add(operand, data, high.low);

	if (high.word > 0x90)
		high += 0x60;

	set_flag(CF, high.high);

	return lowerNibble(low.low) | higherNibble(high.low);
}

void EightBit::MOS6502::andr() noexcept {
	A() = through(A() & BUS().DATA());
}

void EightBit::MOS6502::bit() noexcept {
	const auto data = BUS().DATA();
	set_flag(VF, overflow(data));
	adjustZero(A() & data);
	adjustNegative(data);
}

void EightBit::MOS6502::cmp(const uint8_t first) noexcept {
	const auto second = BUS().DATA();
	const register16_t result = first - second;
	adjustNZ(result.low);
	reset_flag(CF, result.high);
}

uint8_t EightBit::MOS6502::dec(const uint8_t value) noexcept {
	return through(value - 1);
}

void EightBit::MOS6502::eorr() noexcept {
	A() = through(A() ^ BUS().DATA());
}

uint8_t EightBit::MOS6502::inc(const uint8_t value) noexcept {
	return through(value + 1);
}

void EightBit::MOS6502::jsr() noexcept {
	const auto low = fetchByte();
	swallow_stack();
	pushWord(PC());
	PC().high = fetchByte();
	PC().low = low;
}

void EightBit::MOS6502::orr() noexcept {
	A() = through(A() | BUS().DATA());
}

void EightBit::MOS6502::php() noexcept {
	push(P() | BF);
}

void EightBit::MOS6502::plp() noexcept {
	swallow_stack();
	P() = (pop() | RF) & ~BF;
}

void EightBit::MOS6502::rti() noexcept {
	plp();
	ret();
}

void EightBit::MOS6502::rts() noexcept {
	swallow_stack();
	ret();
	swallow_fetch();
}

// Undocumented compound instructions

void EightBit::MOS6502::anc() noexcept {
	andr();
	set_flag(CF, A() & NF);
}

void EightBit::MOS6502::arr() noexcept {
	const auto value = BUS().DATA();
	decimal() ? arr_d(value) : arr_b(value);
}

void EightBit::MOS6502::arr_d(const uint8_t value) noexcept {

	// With thanks to https://github.com/TomHarte/CLK
	// What a very strange instruction ARR is...

	A() &= value;
	auto unshiftedA = A();
	A() = through((A() >> 1) | (carry() << 7));
	set_flag(VF, overflow((A() ^ (A() << 1))));

	if (lowerNibble(unshiftedA) + (unshiftedA & 0x1) > 5)
		A() = lowerNibble(A() + 6) | higherNibble(A());

	set_flag(CF, higherNibble(unshiftedA) + (unshiftedA & 0x10) > 0x50);
	
	if (carry())
		A() += 0x60;
}

void EightBit::MOS6502::arr_b(const uint8_t value) noexcept {
	A() &= value;
	A() = through((A() >> 1) | (carry() << 7));
	set_flag(CF, A() & VF);
	set_flag(VF, overflow((A() ^ (A() << 1))));
}

void EightBit::MOS6502::axs() noexcept {
	X() = through(sub_b(A() & X(), BUS().DATA()));
	reset_flag(CF, m_intermediate.high);
}

void EightBit::MOS6502::jam() noexcept {
	BUS().ADDRESS() = PC()--;
	memoryRead();
	memoryRead();
}
