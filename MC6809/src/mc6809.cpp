#include "stdafx.h"
#include "../inc/mc6809.h"

#include <algorithm>
#include <cassert>

EightBit::mc6809::mc6809(Bus& bus)
: BigEndianProcessor(bus) {
	RaisedPOWER.connect([this](EventArgs) {
		lowerBA();
		lowerBS();
		lowerRW();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(NMI, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(FIRQ, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(HALT, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(BA, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(BS, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(RW, mc6809);

void EightBit::mc6809::swallowRead(int ticks) {
	for (int i = 0; i < ticks; i++)
		base::memoryRead(0xff, 0xff);
}

void EightBit::mc6809::swallowCurrent(int ticks) {
	for (int i = 0; i < ticks; i++)
		base::memoryRead(PC());
}

void EightBit::mc6809::swallowPop(register16_t stack) {
	base::memoryRead(stack);
}

void EightBit::mc6809::swallowEffectiveAddress() {
	base::memoryRead(EA());
}

void EightBit::mc6809::swallowSpin(int ticks) {
	for (int i = 0; i < ticks; i++)
		memoryRead();
}

void EightBit::mc6809::poweredStep() noexcept {
	m_prefix10 = m_prefix11 = false;
	if (halted())
		handleHALT();
	else if (lowered(RESET()))
		handleRESET();
	else if (lowered(NMI()))
		handleNMI();
	else if (lowered(FIRQ()) && !fastInterruptMasked())
		handleFIRQ();
	else if (lowered(INT()) && !interruptMasked())
		handleINT();
	else
		base::execute(fetchInstruction());
}

// Interrupt (etc.) handlers

void EightBit::mc6809::handleRESET() noexcept {
	base::handleRESET();
	raiseNMI();
	lowerBA();
	raiseBS();
	DP() = 0;
	CC() = setBit(CC(), IF);	// Disable IRQ
	CC() = setBit(CC(), FF);	// Disable FIRQ
	BUS().ADDRESS() = { _vectorRESET, 0xff };
	swallowSpin(3);
	base::getPagedInto(PC());
	swallowRead();
}

void EightBit::mc6809::handleINT() noexcept {
	base::handleINT();
	swallowCurrent(2);
	lowerBA();
	raiseBS();
	saveEntireRegisterState();
	CC() = setBit(CC(), IF);	// Disable IRQ
	swallowRead();
	Processor::getPagedInto(0xff, _vectorIRQ, PC());
	swallowRead();
}

void EightBit::mc6809::handleHALT() noexcept {
	raiseBA();
	raiseBS();
}

void EightBit::mc6809::handleNMI() noexcept {
	swallowCurrent(2);
	raiseNMI();
	lowerBA();
	raiseBS();
	saveEntireRegisterState();
	CC() = setBit(CC(), IF);	// Disable IRQ
	CC() = setBit(CC(), FF);	// Disable FIRQ
	swallowRead();
	Processor::getPagedInto(0xff, _vectorNMI, PC());
	swallowRead();
}

void EightBit::mc6809::handleFIRQ() noexcept {
	swallowCurrent(2);
	raiseFIRQ();
	lowerBA();
	raiseBS();
	savePartialRegisterState();
	swallowRead();
	CC() = setBit(CC(), IF);	// Disable IRQ
	CC() = setBit(CC(), FF);	// Disable FIRQ
	Processor::getPagedInto(0xff, _vectorFIRQ, PC());
	swallowRead();
}

//

void EightBit::mc6809::memoryWrite() noexcept {
	WritingMemory.fire();
		tick();
		lowerRW();
		base::memoryWrite();
	WrittenMemory.fire();
}

void EightBit::mc6809::memoryRead() noexcept {
	ReadingMemory.fire();
		tick();
		raiseRW();
		base::memoryRead();
	ReadMemory.fire();
}

//

void EightBit::mc6809::execute() noexcept {
	lowerBA();
	lowerBS();
	if (m_prefix10) {
		execute10();
	} else if (m_prefix11) {
		execute11();
	} else {
		executeUnprefixed();
	}
}

void EightBit::mc6809::executeUnprefixed() {

	switch (opcode()) {

	case 0x10:	prefix10();	break;
	case 0x11:	prefix11();	break;

	// ABX
	case 0x3a:	swallowCurrent(); ABX(); break;			// ABX (inherent)

	// ADC
	case 0x89:	immediateByte(); ADCA(); break;			// ADC (ADCA immediate)
	case 0x99:	directByte(); ADCA(); break;			// ADC (ADCA direct)
	case 0xa9:	indexedByte(); ADCA(); break;			// ADC (ADCA indexed)
	case 0xb9:	extendedByte(); ADCA(); break;			// ADC (ADCA extended)

	case 0xc9:	immediateByte(); ADCB(); break;			// ADC (ADCB immediate)
	case 0xd9:	directByte(); ADCB(); break;			// ADC (ADCB direct)
	case 0xe9:	indexedByte(); ADCB(); break;			// ADC (ADCB indexed)
	case 0xf9:	extendedByte(); ADCB(); break;			// ADC (ADCB extended)

	// ADD
	case 0x8b:	immediateByte(); ADDA(); break;			// ADD (ADDA immediate)
	case 0x9b:	directByte(); ADDA(); break;			// ADD (ADDA direct)
	case 0xab:	indexedByte(); ADDA(); break;			// ADD (ADDA indexed)
	case 0xbb:	extendedByte(); ADDA(); break;			// ADD (ADDA extended)

	case 0xcb:	immediateByte(); ADDB(); break;			// ADD (ADDB immediate)
	case 0xdb:	directByte(); ADDB(); break;			// ADD (ADDB direct)
	case 0xeb:	indexedByte(); ADDB(); break;			// ADD (ADDB indexed)
	case 0xfb:	extendedByte(); ADDB(); break;			// ADD (ADDB extended)

	case 0xc3:	immediateShort(); ADDD(); break;		// ADD (ADDD immediate)
	case 0xd3:	directShort(); ADDD(); break;			// ADD (ADDD direct)
	case 0xe3:	indexedShort(); ADDD(); break;			// ADD (ADDD indexed)
	case 0xf3:	extendedShort(); ADDD();break;			// ADD (ADDD extended)

	// AND
	case 0x84: immediateByte(); ANDA(); break;			// AND (ANDA immediate)
	case 0x94: directByte(); ANDA(); break;				// AND (ANDA direct)
	case 0xa4: indexedByte(); ANDA(); break;			// AND (ANDA indexed)
	case 0xb4: extendedByte(); ANDA(); break;			// AND (ANDA extended)

	case 0xc4: immediateByte(); ANDB(); break;			// AND (ANDB immediate)
	case 0xd4: directByte(); ANDB(); break;				// AND (ANDB direct)
	case 0xe4: indexedByte(); ANDB(); break;			// AND (ANDB indexed)
	case 0xf4: extendedByte(); ANDB(); break;			// AND (ANDB extended)

	case 0x1c: immediateByte(); ANDCC(); break;			// AND (ANDCC immediate)

	// ASL/LSL
	case 0x08: directByte(); ASL(); break;				// ASL (direct)
	case 0x48: swallowCurrent(); ASLA(); break;			// ASL (ASLA inherent)
	case 0x58: swallowCurrent(); ASLB(); break;			// ASL (ASLB inherent)
	case 0x68: indexedByte(); ASL(); break;				// ASL (indexed)
	case 0x78: extendedByte(); ASL(); break;			// ASL (extended)

	// ASR
	case 0x07: directByte(); ASR(); break;				// ASR (direct)
	case 0x47: swallowCurrent(); ASRA(); break;			// ASR (ASRA inherent)
	case 0x57: swallowCurrent(); ASRB(); break;			// ASR (ASRB inherent)
	case 0x67: indexedByte(); ASR(); break;				// ASR (indexed)
	case 0x77: extendedByte(); ASR(); break;			// ASR (extended)

	// BIT
	case 0x85: immediateByte(); BITA(); break;			// BIT (BITA immediate)
	case 0x95: directByte(); BITA(); break;				// BIT (BITA direct)
	case 0xa5: indexedByte(); BITA(); break;			// BIT (BITA indexed)
	case 0xb5: extendedByte(); BITA(); break;			// BIT (BITA extended)

	case 0xc5: immediateByte(); BITB(); break;			// BIT (BITB immediate)
	case 0xd5: directByte(); BITB(); break;				// BIT (BITB direct)
	case 0xe5: indexedByte(); BITB(); break;			// BIT (BITB indexed)
	case 0xf5: extendedByte(); BITB(); break;			// BIT (BITB extended)

	// CLR
	case 0x0f: directAddress(); CLR(); break;			// CLR (direct)
	case 0x4f: swallowCurrent(); CLRA(); break;			// CLR (CLRA implied)
	case 0x5f: swallowCurrent(); CLRB(); break;			// CLR (CLRB implied)
	case 0x6f: indexedAddress(); CLR(); break;			// CLR (indexed)
	case 0x7f: extendedAddress(); CLR(); break;			// CLR (extended)

	// CMP

	// CMPA
	case 0x81: immediateByte(); CMPA(); break;			// CMP (CMPA, immediate)
	case 0x91: directByte(); CMPA(); break;				// CMP (CMPA, direct)
	case 0xa1: indexedByte(); CMPA(); break;			// CMP (CMPA, indexed)
	case 0xb1: extendedByte(); CMPA(); break;			// CMP (CMPA, extended)

	// CMPB
	case 0xc1: immediateByte(); CMPB(); break;			// CMP (CMPB, immediate)
	case 0xd1: directByte(); CMPB(); break;				// CMP (CMPB, direct)
	case 0xe1: indexedByte(); CMPB(); break;			// CMP (CMPB, indexed)
	case 0xf1: extendedByte(); CMPB(); break;			// CMP (CMPB, extended)

	// CMPX
	case 0x8c: immediateShort(); CMPX(); break;			// CMP (CMPX, immediate)
	case 0x9c: directShort(); CMPX(); break;			// CMP (CMPX, direct)
	case 0xac: indexedShort(); CMPX(); break;			// CMP (CMPX, indexed)
	case 0xbc: extendedShort(); CMPX(); break;			// CMP (CMPX, extended)

	// COM
	case 0x03: directByte(); COM(); break;				// COM (direct)
	case 0x43: swallowCurrent(); COMA(); break;			// COM (COMA inherent)
	case 0x53: swallowCurrent(); COMB(); break;			// COM (COMB inherent)
	case 0x63: indexedByte(); COM(); break;				// COM (indexed)
	case 0x73: extendedByte(); COM(); break;			// COM (extended)

	// CWAI
	case 0x3c: immediateByte(); CWAI(); break;			// CWAI (immediate) - cycles omitted: halts before full interrupt response

	// DAA
	case 0x19: swallowCurrent(); DAA(); break;			// DAA (inherent)

	// DEC
	case 0x0a: directByte(); DEC(); break;				// DEC (direct)
	case 0x4a: swallowCurrent(); DECA(); break;			// DEC (DECA inherent)
	case 0x5a: swallowCurrent(); DECB(); break;			// DEC (DECB inherent)
	case 0x6a: indexedByte(); DEC(); break;				// DEC (indexed)
	case 0x7a: extendedByte(); DEC(); break;			// DEC (extended)

	// EOR

	// EORA
	case 0x88: immediateByte(); EORA(); break;			// EOR (EORA immediate)
	case 0x98: directByte(); EORA(); break;				// EOR (EORA direct)
	case 0xa8: indexedByte(); EORA(); break;			// EOR (EORA indexed)
	case 0xb8: extendedByte(); EORA(); break;			// EOR (EORA extended)

	// EORB
	case 0xc8: immediateByte(); EORB(); break;			// EOR (EORB immediate)
	case 0xd8: directByte(); EORB(); break;				// EOR (EORB direct)
	case 0xe8: indexedByte(); EORB(); break;			// EOR (EORB indexed)
	case 0xf8: extendedByte(); EORB(); break;			// EOR (EORB extended)

	// EXG
	case 0x1e: immediateByte(); EXG(); break;			// EXG (R1,R2 immediate)

	// INC
	case 0x0c: directByte(); INC(); break;				// INC (direct)
	case 0x4c: swallowCurrent(); INCA(); break;			// INC (INCA inherent)
	case 0x5c: swallowCurrent(); INCB(); break;			// INC (INCB inherent)
	case 0x6c: indexedByte(); INC(); break;				// INC (indexed)
	case 0x7c: extendedByte(); INC(); break;			// INC (extended)

	// JMP
	case 0x0e: directAddress(); JMP(); break;			// JMP (direct)
	case 0x6e: indexedAddress(); JMP(); break;			// JMP (indexed)
	case 0x7e: extendedAddress(); JMP(); break;			// JMP (extended)

	// JSR
	case 0x9d: directAddress(); JSR(); break;			// JSR (direct)
	case 0xad: indexedAddress(); JSR(); break;			// JSR (indexed)
	case 0xbd: extendedAddress(); JSR(); break;			// JSR (extended)

	// LD

	// LDA
	case 0x86: immediateByte(); LDA(); break;			// LD (LDA immediate)
	case 0x96: directByte(); LDA(); break;				// LD (LDA direct)
	case 0xa6: indexedByte(); LDA(); break;				// LD (LDA indexed)
	case 0xb6: extendedByte(); LDA(); break;			// LD (LDA extended)

	// LDB
	case 0xc6: immediateByte(); LDB(); break;			// LD (LDB immediate)
	case 0xd6: directByte(); LDB(); break;				// LD (LDB direct)
	case 0xe6: indexedByte(); LDB(); break;				// LD (LDB indexed)
	case 0xf6: extendedByte(); LDB(); break;			// LD (LDB extended)

	// LDD
	case 0xcc: immediateShort(); LDD(); break;			// LD (LDD immediate)
	case 0xdc: directShort(); LDD(); break;				// LD (LDD direct)
	case 0xec: indexedShort(); LDD(); break;			// LD (LDD indexed)
	case 0xfc: extendedShort(); LDD(); break;			// LD (LDD extended)

	// LDU
	case 0xce: immediateShort(); LDU(); break;			// LD (LDU immediate)
	case 0xde: directShort(); LDU(); break;				// LD (LDU direct)
	case 0xee: indexedShort(); LDU(); break;			// LD (LDU indexed)
	case 0xfe: extendedShort(); LDU(); break;			// LD (LDU extended)

	// LDX
	case 0x8e: immediateShort(); LDX(); break;			// LD (LDX immediate)
	case 0x9e: directShort(); LDX(); break;				// LD (LDX direct)
	case 0xae: indexedShort(); LDX(); break;			// LD (LDX indexed)
	case 0xbe: extendedShort(); LDX(); break;			// LD (LDX extended)

	// LEA
	case 0x30: indexedAddress(); LEAX(); break;			// LEA (LEAX indexed)
	case 0x31: indexedAddress(); LEAY(); break;			// LEA (LEAY indexed)
	case 0x32: indexedAddress(); LEAS(); break;			// LEA (LEAS indexed)
	case 0x33: indexedAddress(); LEAU(); break;			// LEA (LEAU indexed)

	// LSR
	case 0x04: directByte(); LSR(); break;				// LSR (direct)
	case 0x44: swallowCurrent(); LSRA(); break;			// LSR (LSRA inherent)
	case 0x54: swallowCurrent(); LSRB(); break;			// LSR (LSRB inherent)
	case 0x64: indexedByte(); LSR(); break;				// LSR (indexed)
	case 0x74: extendedByte(); LSR(); break;			// LSR (extended)

	// MUL
	case 0x3d: swallowCurrent(); MUL(); break;			// MUL (inherent)

	// NEG
	case 0x00: directByte(); NEG(); break;				// NEG (direct)
	case 0x40: swallowCurrent(); NEGA(); break;			// NEG (NEGA, inherent)
	case 0x50: swallowCurrent(); NEGB(); break;			// NEG (NEGB, inherent)
	case 0x60: indexedByte(); NEG(); break;				// NEG (indexed)
	case 0x70: extendedByte(); NEG(); break;			// NEG (extended)

	// NOP
	case 0x12: swallowCurrent(); NOP(); break;			// NOP (inherent)

	// OR

	// ORA
	case 0x8a: immediateByte(); ORA(); break;			// OR (ORA immediate)
	case 0x9a: directByte(); ORA(); break;				// OR (ORA direct)
	case 0xaa: indexedByte(); ORA(); break;				// OR (ORA indexed)
	case 0xba: extendedByte(); ORA(); break;			// OR (ORA extended)

	// ORB
	case 0xca: immediateByte(); ORB(); break;			// OR (ORB immediate)
	case 0xda: directByte(); ORB(); break;				// OR (ORB direct)
	case 0xea: indexedByte(); ORB(); break;				// OR (ORB indexed)
	case 0xfa: extendedByte(); ORB(); break;			// OR (ORB extended)

	// ORCC
	case 0x1a: immediateByte(); ORCC(); break;			// OR (ORCC immediate)

	// PSH
	case 0x34: immediateByte(); PSHS(); break;			// PSH (PSHS immediate)
	case 0x36: immediateByte(); PSHU(); break;			// PSH (PSHU immediate)

	// PUL
	case 0x35: immediateByte(); PULS(); break;			// PUL (PULS immediate)
	case 0x37: immediateByte(); PULU(); break;			// PUL (PULU immediate)

	// ROL
	case 0x09: directByte(); ROL(); break;				// ROL (direct)
	case 0x49: swallowCurrent(); ROLA(); break;			// ROL (ROLA inherent)
	case 0x59: swallowCurrent(); ROLB(); break;			// ROL (ROLB inherent)
	case 0x69: indexedByte(); ROL(); break;				// ROL (indexed)
	case 0x79: extendedByte(); ROL(); break;			// ROL (extended)

	// ROR
	case 0x06: directByte(); ROR(); break;				// ROR (direct)
	case 0x46: swallowCurrent(); RORA(); break;			// ROR (RORA inherent)
	case 0x56: swallowCurrent(); RORB(); break;			// ROR (RORB inherent)
	case 0x66: indexedByte(); ROR(); break;				// ROR (indexed)
	case 0x76: extendedByte(); ROR(); break;			// ROR (extended)

	// RTI
	case 0x3B: swallowCurrent(); RTI(); break;			// RTI (inherent)

	// RTS
	case 0x39: swallowCurrent(); RTS(); break;			// RTS (inherent)

	// SBC

	// SBCA
	case 0x82: immediateByte(); SBCA(); break;			// SBC (SBCA immediate)
	case 0x92: directByte(); SBCA(); break;				// SBC (SBCA direct)
	case 0xa2: indexedByte(); SBCA(); break;			// SBC (SBCA indexed)
	case 0xb2: extendedByte(); SBCA(); break;			// SBC (SBCB extended)

	// SBCB
	case 0xc2: immediateByte(); SBCB(); break;			// SBC (SBCB immediate)
	case 0xd2: directByte(); SBCB(); break;				// SBC (SBCB direct)
	case 0xe2: indexedByte(); SBCB(); break;			// SBC (SBCB indexed)
	case 0xf2: extendedByte(); SBCB(); break;			// SBC (SBCB extended)

	// SEX
	case 0x1d: swallowCurrent(); SEX(); break;			// SEX (inherent)

	// ST

	// STA
	case 0x97: directAddress(); STA(); break;			// ST (STA direct)
	case 0xa7: indexedAddress(); STA(); break;			// ST (STA indexed)
	case 0xb7: extendedAddress(); STA(); break;			// ST (STA extended)

	// STB
	case 0xd7: directAddress(); STB(); break;			// ST (STB direct)
	case 0xe7: indexedAddress(); STB(); break;			// ST (STB indexed)
	case 0xf7: extendedAddress(); STB(); break;			// ST (STB extended)

	// STD
	case 0xdd: directAddress(); STD(); break;			// ST (STD direct)
	case 0xed: indexedAddress(); STD(); break;			// ST (STD indexed)
	case 0xfd: extendedAddress(); STD(); break;			// ST (STD extended)

	// STU
	case 0xdf: directAddress(); STU(); break;			// ST (STU direct)
	case 0xef: indexedAddress(); STU(); break;			// ST (STU indexed)
	case 0xff: extendedAddress(); STU(); break;			// ST (STU extended)

	// STX
	case 0x9f: directAddress(); STX(); break;			// ST (STX direct)
	case 0xaf: indexedAddress(); STX(); break;			// ST (STX indexed)
	case 0xbf: extendedAddress(); STX(); break;			// ST (STX extended)

	// SUB

	// SUBA
	case 0x80: immediateByte(); SUBA(); break;			// SUB (SUBA immediate)
	case 0x90: directByte(); SUBA(); break;				// SUB (SUBA direct)
	case 0xa0: indexedByte(); SUBA(); break;			// SUB (SUBA indexed)
	case 0xb0: extendedByte(); SUBA(); break;			// SUB (SUBA extended)

	// SUBB
	case 0xc0: immediateByte(); SUBB(); break;			// SUB (SUBB immediate)
	case 0xd0: directByte(); SUBB(); break;				// SUB (SUBB direct)
	case 0xe0: indexedByte(); SUBB(); break;			// SUB (SUBB indexed)
	case 0xf0: extendedByte(); SUBB(); break;			// SUB (SUBB extended)

	// SUBD
	case 0x83: immediateShort(); SUBD(); break;			// SUB (SUBD immediate)
	case 0x93: directShort(); SUBD(); break;			// SUB (SUBD direct)
	case 0xa3: indexedShort(); SUBD(); break;			// SUB (SUBD indexed)
	case 0xb3: extendedShort(); SUBD(); break;			// SUB (SUBD extended)

	// SWI
	case 0x3f: swallowCurrent(); SWI(); break;			// SWI (inherent)

	// SYNC
	case 0x13: swallowCurrent(); SYNC(); break;			// SYNC (inherent)

	// TFR
	case 0x1f: immediateByte(); TFR(); break;			// TFR (immediate)

	// TST
	case 0x0d: directByte(); TST(); break;				// TST (direct)
	case 0x4d: swallowCurrent(); TSTA(); break;			// TST (TSTA inherent)
	case 0x5d: swallowCurrent(); TSTB(); break;			// TST (TSTB inherent)
	case 0x6d: indexedByte(); TST(); break;				// TST (indexed)
	case 0x7d: extendedByte(); TST(); break;			// TST (extended)

	// Branching
	case 0x16: relativeWordAddress(); LBRA(); break;	// BRA (LBRA relative)
	case 0x17: relativeWordAddress(); LBSR(); break;	// BSR (LBSR relative)
	case 0x20: relativeByteAddress(); BRA(); break;		// BRA (relative)
	case 0x21: relativeByteAddress(); BRN(); break;		// BRN (relative)
	case 0x22: relativeByteAddress(); BHI(); break;		// BHI (relative)
	case 0x23: relativeByteAddress(); BLS(); break;		// BLS (relative)
	case 0x24: relativeByteAddress(); BCC(); break;		// BCC (relative)
	case 0x25: relativeByteAddress(); BCS(); break;		// BCS (relative)
	case 0x26: relativeByteAddress(); BNE(); break;		// BNE (relative)
	case 0x27: relativeByteAddress(); BEQ(); break;		// BEQ (relative)
	case 0x28: relativeByteAddress(); BVC(); break;		// BVC (relative)
	case 0x29: relativeByteAddress(); BVS(); break;		// BVS (relative)
	case 0x2a: relativeByteAddress(); BPL(); break;		// BPL (relative)
	case 0x2b: relativeByteAddress(); BMI(); break;		// BMI (relative)
	case 0x2c: relativeByteAddress(); BGE(); break;		// BGE (relative)
	case 0x2d: relativeByteAddress(); BLT(); break;		// BLT (relative)
	case 0x2e: relativeByteAddress(); BGT(); break;		// BGT (relative)
	case 0x2f: relativeByteAddress(); BLE(); break;		// BLE (relative)

	case 0x8d: relativeByteAddress(); BSR(); break;		// BSR (relative)

	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::execute10() {

	switch (opcode()) {

	// CMP

	// CMPD
	case 0x83: immediateShort(); CMPD(); break;			// CMP (CMPD, immediate)
	case 0x93: directShort(); CMPD(); break;			// CMP (CMPD, direct)
	case 0xa3: indexedShort(); CMPD(); break;			// CMP (CMPD, indexed)
	case 0xb3: extendedShort(); CMPD(); break;			// CMP (CMPD, extended)

	// CMPY
	case 0x8c: immediateShort(); CMPY(); break;			// CMP (CMPY, immediate)
	case 0x9c: directShort(); CMPY(); break;			// CMP (CMPY, direct)
	case 0xac: indexedShort(); CMPY(); break;			// CMP (CMPY, indexed)
	case 0xbc: extendedShort(); CMPY(); break;			// CMP (CMPY, extended)

	// LD

	// LDS
	case 0xce: immediateShort(); LDS(); break;			// LD (LDS immediate)
	case 0xde: directShort(); LDS(); break;				// LD (LDS direct)
	case 0xee: indexedShort(); LDS(); break;			// LD (LDS indexed)
	case 0xfe: extendedShort(); LDS(); break;			// LD (LDS extended)

	// LDY
	case 0x8e: immediateShort(); LDY(); break;			// LD (LDY immediate)
	case 0x9e: directShort(); LDY(); break;				// LD (LDY direct)
	case 0xae: indexedShort(); LDY(); break;			// LD (LDY indexed)
	case 0xbe: extendedShort(); LDY(); break;			// LD (LDY extended)

	// Branching
	case 0x21: relativeWordAddress(); LBRN(); break;	// BRN (LBRN relative)
	case 0x22: relativeWordAddress(); LBHI(); break;	// BHI (LBHI relative)
	case 0x23: relativeWordAddress(); LBLS(); break;	// BLS (LBLS relative)
	case 0x24: relativeWordAddress(); LBCC(); break;	// BCC (LBCC relative)
	case 0x25: relativeWordAddress(); LBCS(); break;	// BCS (LBCS relative)
	case 0x26: relativeWordAddress(); LBNE(); break;	// BNE (LBNE relative)
	case 0x27: relativeWordAddress(); LBEQ(); break;	// BEQ (LBEQ relative)
	case 0x28: relativeWordAddress(); LBVC(); break;	// BVC (LBVC relative)
	case 0x29: relativeWordAddress(); LBVS(); break;	// BVS (LBVS relative)
	case 0x2a: relativeWordAddress(); LBPL(); break;	// BPL (LBPL relative)
	case 0x2b: relativeWordAddress(); LBMI(); break;	// BMI (LBMI relative)
	case 0x2c: relativeWordAddress(); LBGE(); break;	// BGE (LBGE relative)
	case 0x2d: relativeWordAddress(); LBLT(); break;	// BLT (LBLT relative)
	case 0x2e: relativeWordAddress(); LBGT(); break;	// BGT (LBGT relative)
	case 0x2f: relativeWordAddress(); LBLE(); break;	// BLE (LBLE relative)

	// STS
	case 0xdf: directAddress(); STS(); break;			// ST (STS direct)
	case 0xef: indexedAddress(); STS(); break;			// ST (STS indexed)
	case 0xff: extendedAddress(); STS(); break;			// ST (STS extended)

	// STY
	case 0x9f: directAddress(); STY(); break;			// ST (STY direct)
	case 0xaf: indexedAddress(); STY(); break;			// ST (STY indexed)
	case 0xbf: extendedAddress(); STY(); break;			// ST (STY extended)

	// SWI
	case 0x3f: swallowCurrent(); SWI2(); break;			// SWI (SWI2 inherent)

	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::execute11() {

	switch (opcode()) {

	// CMP

	// CMPU
	case 0x83: immediateShort(); CMPU(); break;			// CMP (CMPU, immediate)
	case 0x93: directShort(); CMPU(); break;			// CMP (CMPU, direct)
	case 0xa3: indexedShort(); CMPU(); break;			// CMP (CMPU, indexed)
	case 0xb3: extendedShort(); CMPU(); break;			// CMP (CMPU, extended)

	// CMPS
	case 0x8c: immediateShort(); CMPS(); break;			// CMP (CMPS, immediate)
	case 0x9c: directShort(); CMPS(); break;			// CMP (CMPS, direct)
	case 0xac: indexedShort(); CMPS(); break;			// CMP (CMPS, indexed)
	case 0xbc: extendedShort(); CMPS(); break;			// CMP (CMPS, extended)

	// SWI
	case 0x3f: swallowCurrent(); SWI3(); break;			// SWI (SWI3 inherent)

	default:
		UNREACHABLE;
	}
}

#pragma region Miscellaneous instruction implementations

void EightBit::mc6809::prefix10() {
	m_prefix10 = true;
	base::execute(fetchInstruction());
}

void EightBit::mc6809::prefix11() {
	m_prefix11 = true;
	base::execute(fetchInstruction());
}

#pragma endregion

#pragma region Push / Pop

void EightBit::mc6809::pop() noexcept {
	popS();
}

void EightBit::mc6809::push(uint8_t value) noexcept {
	pushS(value);
}

void EightBit::mc6809::push(register16_t& stack, uint8_t value) {
	base::memoryWrite(--stack, value);
}

void EightBit::mc6809::pushS(uint8_t value) noexcept {
	push(S(), value);
}

void EightBit::mc6809::push(register16_t& stack, register16_t value) {
	push(stack, value.low);
	push(stack, value.high);
}

void EightBit::mc6809::pop(register16_t& stack) noexcept {
	base::memoryRead(stack++);
}

void EightBit::mc6809::popS() noexcept {
	pop(S());
}

EightBit::register16_t EightBit::mc6809::popWord(register16_t& stack) {
	pop(stack);
	intermediate().high = BUS().DATA();
	pop(stack);
	intermediate().low = BUS().DATA();
	return intermediate();
}

#pragma endregion

#pragma region Addressing modes

void EightBit::mc6809::immediateAddress() noexcept {
	EA() = PC();
	++PC();
}

void EightBit::mc6809::relativeByteAddress() {
	fetchByte();
	const int8_t offset = BUS().DATA();
	EA().joined = PC().joined + offset;
}

void EightBit::mc6809::relativeWordAddress() {
	fetchShort();
	const int16_t offset = intermediate().joined;
	EA().joined = PC().joined + offset;
}

void EightBit::mc6809::directAddress() {
	fetchByte();
	EA() = { BUS().DATA(), DP() };
	swallowRead();
}

void EightBit::mc6809::extendedAddress() {
	fetchInto(intermediate());
	EA() = intermediate();
	swallowRead();
}

EightBit::register16_t& EightBit::mc6809::RR(int which) {
	switch (which) {
	case 0b00:
		return X();
	case 0b01:
		return Y();
	case 0b10:
		return U();
	case 0b11:
		return S();
	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::indexedAddress() {
	fetchByte();
	const auto type = BUS().DATA();
	auto& r = RR((type & (Bit6 | Bit5)) >> 5);
	if ((type & Bit7) != 0) {
		switch (type & Mask4) {
		case 0b0000: // ,R+
			EA() = r;
			++r;
			swallowCurrent();
			swallowRead(2);
			break;
		case 0b0001: // ,R++
			EA() = r;
			r += 2;
			swallowCurrent();
			swallowRead(3);
			break;
		case 0b0010: // ,-R
			--r;
			EA() = r;
			swallowCurrent();
			swallowRead(2);
			break;
		case 0b0011: // ,--R
			r -= 2;
			EA() = r;
			swallowCurrent();
			swallowRead(3);
			break;
		case 0b0100: // ,R
			EA() = r;
			swallowCurrent();
			break;
		case 0b0101: // B,R
			EA().joined = r.joined + B();
			swallowCurrent();
			swallowRead();
			break;
		case 0b0110: // A,R
			EA().joined = r.joined + (int8_t)A();
			swallowCurrent();
			swallowRead();
			break;
		case 0b1000: // n,R (eight-bit)
			fetchByte();
			EA().joined = r.joined + (int8_t)BUS().DATA();
			swallowRead();
			break;
		case 0b1001: // n,R (sixteen-bit)
			fetchInto(intermediate());
			EA() = intermediate();
			EA().joined += r.joined;
			swallowCurrent();
			swallowRead(2);
			break;
		case 0b1011: // D,R
			EA().joined = r.joined + D().joined;
			swallowCurrent(3);
			swallowRead(2);
			break;
		case 0b1100: // n,PCR (eight-bit)
			relativeByteAddress();
			swallowRead();
			break;
		case 0b1101: // n,PCR (sixteen-bit)
			relativeWordAddress();
			swallowCurrent();
			swallowRead(3);
			break;
		case 0b1111: // [n]
			fetchInto(intermediate());
			EA() = intermediate();
			swallowCurrent();
			break;
		default:
			assert(false && "Invalid index type");
		}

		const auto indirect = type & Bit4;
		if (indirect != 0) {
			LEA(BUS().ADDRESS());
			getInto(EA());
			swallowRead();
		}
	}
	else {
		// EA = ,R + 5-bit offset
		EA().joined = r.joined + signExtend(5, type & Mask5);
		swallowCurrent();
		swallowRead();
	}
}

void EightBit::mc6809::immediateByte() {
	immediateAddress();
	base::memoryRead(EA());
}

void EightBit::mc6809::fetchByte() noexcept {
	immediateByte();
}

void EightBit::mc6809::directByte() {
	directAddress();
	base::memoryRead(EA());
}

void EightBit::mc6809::indexedByte() {
	indexedAddress();
	base::memoryRead(EA());
}

void EightBit::mc6809::extendedByte() {
	extendedAddress();
	base::memoryRead(EA());
}

void EightBit::mc6809::immediateShort() {
	fetchShort();
}

void EightBit::mc6809::directShort() {
	directAddress();
	getShort(EA());
}

void EightBit::mc6809::indexedShort() {
	indexedAddress();
	getShort(EA());
}

void EightBit::mc6809::extendedShort() {
	extendedAddress();
	getShort(EA());
}

#pragma endregion

#pragma region Load / store 8 or 16 - bit data

uint8_t EightBit::mc6809::through(uint8_t data) {
	CC() = clearBit(CC(), VF);
	CC() = adjustNZ(data);
	return data;
}

void EightBit::mc6809::assign(uint8_t& destination) {
	destination = through(BUS().DATA());
}

void EightBit::mc6809::LDA() {
	assign(A());
}

void EightBit::mc6809::LDB() {
	assign(B());
}

uint16_t EightBit::mc6809::through(uint16_t data) {
	CC() = clearBit(CC(), VF);
	CC() = adjustNZ(data);
	return data;
}

EightBit::register16_t EightBit::mc6809::through(register16_t data) {
	return through(data.joined);
}

void EightBit::mc6809::assign(register16_t& destination) {
	destination = through(intermediate());
}

void EightBit::mc6809::LDD() {
	assign(D());
}

void EightBit::mc6809::LDS() {
	assign(S());
}

void EightBit::mc6809::LDU() {
	assign(U());
}

void EightBit::mc6809::LDX() {
	assign(X());
}

void EightBit::mc6809::LDY() {
	assign(Y());
}

void EightBit::mc6809::store(uint8_t data) {
	base::memoryWrite(EA(), through(data));
}

void EightBit::mc6809::STA() {
	store(A());
}

void EightBit::mc6809::STB() {
	store(B());
}

void EightBit::mc6809::store(register16_t data) {
	Processor::setShort(EA(), through(data));
}

void EightBit::mc6809::STD() {
	store(D());
}

void EightBit::mc6809::STU() {
	store(U());
}

void EightBit::mc6809::STS() {
	store(S());
}

void EightBit::mc6809::STX() {
	store(X());
}

void EightBit::mc6809::STY() {
	store(Y());
}

#pragma endregion

#pragma region Branching

void EightBit::mc6809::LBSR() {
	swallowRead(4);
	call(EA());
}

void EightBit::mc6809::BSR() {
	swallowRead(3);
	call(EA());
}

bool EightBit::mc6809::branch(register16_t destination, bool condition) {
	swallowRead();
	if (condition)
		jump(destination);
	return condition;
}

void EightBit::mc6809::branchShort(bool condition) {
	branch(EA(), condition);
}

void EightBit::mc6809::branchLong(bool condition) {
	if (branch(EA(), condition))
		swallowRead();
}

void EightBit::mc6809::BRA() { branchShort(true); }
void EightBit::mc6809::BRN() { branchShort(false); }
void EightBit::mc6809::BHI() { branchShort(HI()); }
void EightBit::mc6809::BLS() { branchShort(LS()); }
void EightBit::mc6809::BCC() { branchShort(!carry()); }
void EightBit::mc6809::BCS() { branchShort(carry()); }
void EightBit::mc6809::BNE() { branchShort(!zero()); }
void EightBit::mc6809::BEQ() { branchShort(zero()); }
void EightBit::mc6809::BVC() { branchShort(!overflow()); }
void EightBit::mc6809::BVS() { branchShort(overflow()); }
void EightBit::mc6809::BPL() { branchShort(!negative()); }
void EightBit::mc6809::BMI() { branchShort(negative()); }
void EightBit::mc6809::BGE() { branchShort(GE()); }
void EightBit::mc6809::BLT() { branchShort(LT()); }
void EightBit::mc6809::BGT() { branchShort(GT()); }
void EightBit::mc6809::BLE() { branchShort(LE()); }

void EightBit::mc6809::LBRA() { branchLong(true); }
void EightBit::mc6809::LBRN() { branchLong(false); }
void EightBit::mc6809::LBHI() { branchLong(HI()); }
void EightBit::mc6809::LBLS() { branchLong(LS()); }
void EightBit::mc6809::LBCC() { branchLong(!carry()); }
void EightBit::mc6809::LBCS() { branchLong(carry()); }
void EightBit::mc6809::LBNE() { branchLong(!zero()); }
void EightBit::mc6809::LBEQ() { branchLong(zero()); }
void EightBit::mc6809::LBVC() { branchLong(!overflow()); }
void EightBit::mc6809::LBVS() { branchLong(overflow()); }
void EightBit::mc6809::LBPL() { branchLong(!negative()); }
void EightBit::mc6809::LBMI() { branchLong(negative()); }
void EightBit::mc6809::LBGE() { branchLong(GE()); }
void EightBit::mc6809::LBLT() { branchLong(LT()); }
void EightBit::mc6809::LBGT() { branchLong(GT()); }
void EightBit::mc6809::LBLE() { branchLong(LE()); }

#pragma endregion

#pragma region Miscellaneous instruction implementations

void EightBit::mc6809::SYNC() { halt(); }

void EightBit::mc6809::NOP() {
	// No operation!
}

void EightBit::mc6809::ABX() {
	X().joined += B();
	swallowRead();
}

void EightBit::mc6809::ADCA() { A() = addWithCarry(A()); }
void EightBit::mc6809::ADCB() { B() = addWithCarry(B()); }

uint8_t EightBit::mc6809::addWithCarry(uint8_t operand) {
	return add(operand, BUS().DATA(), carryFlag());
}

void EightBit::mc6809::ADDA() { A() = add(A()); }
void EightBit::mc6809::ADDB() { B() = add(B()); }

uint8_t EightBit::mc6809::add(uint8_t operand) {
	return add(operand, BUS().DATA());
}

uint8_t EightBit::mc6809::add(uint8_t operand, uint8_t data, int carry) {
	intermediate().joined = operand + data + carry;
	CC() = adjustAddition(operand, data, intermediate());
	return intermediate().low;
}

void EightBit::mc6809::ADDD() {
	D() = add(D(), intermediate());
}

EightBit::register16_t EightBit::mc6809::add(register16_t operand, register16_t data, int carry) {
	auto addition = operand.joined + data.joined + carry;
	CC() = adjustAddition(operand, data, addition);
	swallowRead();
	return addition;
}

EightBit::register16_t EightBit::mc6809::add(register16_t operand, register16_t data) {
	return add(operand, data, 0);
}

//

void EightBit::mc6809::ANDCC() {
	CC() &= BUS().DATA();
	swallowRead();
}

void EightBit::mc6809::ANDA() { A() = _and(A()); }
void EightBit::mc6809::ANDB() { B() = _and(B()); }

uint16_t EightBit::mc6809::_and(uint16_t operand, uint16_t data) {
	return through((uint16_t)(operand & data));
}

EightBit::register16_t EightBit::mc6809::_and(register16_t operand, register16_t data) {
	return { _and(operand.joined, data.joined) };
}

uint8_t EightBit::mc6809::_and(uint8_t operand, uint8_t data) {
	return through((uint8_t)(operand & data));
}

uint8_t EightBit::mc6809::_and(uint8_t operand) {
	return _and(operand, BUS().DATA());
}

void EightBit::mc6809::ASLA() { A() = arithmeticShiftLeft(A()); }

void EightBit::mc6809::ASLB() { B() = arithmeticShiftLeft(B()); }

void EightBit::mc6809::ASL() {
	auto result = arithmeticShiftLeft(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::arithmeticShiftLeft(uint8_t operand) {
	CC() = setBit(CC(), CF, operand & Bit7);
	CC() = adjustNZ((uint8_t)(operand <<= 1));
	auto overflow = carryFlag() ^ negativeFlag() >> 3;
	CC() = setBit(CC(), VF, overflow);
	return operand;
}

void EightBit::mc6809::ASRA() { A() = arithmeticShiftRight(A()); }

void EightBit::mc6809::ASRB() { B() = arithmeticShiftRight(B()); }

void EightBit::mc6809::ASR() {
	auto result = arithmeticShiftRight(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::arithmeticShiftRight(uint8_t operand) {
	CC() = setBit(CC(), CF, operand & Bit0);
	uint8_t result = (operand >> 1) | (operand & Bit7);
	CC() = adjustNZ(result);
	return result;
}

void EightBit::mc6809::BITA() { bit(A(), BUS().DATA()); }
void EightBit::mc6809::BITB() { bit(B(), BUS().DATA()); }

void EightBit::mc6809::bit(uint8_t operand, uint8_t data) { auto _ = _and(operand, data); }

void EightBit::mc6809::CLRA() { A() = clear(); }

void EightBit::mc6809::CLRB() { B() = clear(); }

void EightBit::mc6809::CLR() {
	swallowEffectiveAddress();
	auto result = clear();
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::clear() {
	CC() = clearBit(CC(), CF);
	return through((uint8_t)0U);
}

void EightBit::mc6809::CMPA() { compare(A(), BUS().DATA()); }
void EightBit::mc6809::CMPB() { compare(B(), BUS().DATA()); }

void EightBit::mc6809::compare(uint16_t operand, uint16_t data) { auto _ = subtract(operand, data); }

void EightBit::mc6809::compare(register16_t operand, register16_t data) { compare(operand.joined, data.joined); }

void EightBit::mc6809::compare(uint8_t operand, uint8_t data) { auto _ = subtract(operand, data); }

void EightBit::mc6809::CMPU() { compare(U()); }
void EightBit::mc6809::CMPS() { compare(S()); }
void EightBit::mc6809::CMPD() { compare(D()); }
void EightBit::mc6809::CMPX() { compare(X()); }
void EightBit::mc6809::CMPY() { compare(Y()); }

void EightBit::mc6809::compare(register16_t operand) { auto _ = subtract(operand, intermediate()); }

void EightBit::mc6809::COMA() { A() = complement(A()); }

void EightBit::mc6809::COMB() { B() = complement(B()); }

void EightBit::mc6809::COM() {
	auto result = complement(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::complement(uint8_t operand) {
	CC() = setBit(CC(), CF);
	return through((uint8_t)~operand);
}

void EightBit::mc6809::CWAI() {
	CC() &= BUS().DATA();
	swallowCurrent();
	saveEntireRegisterState();
	swallowRead();
	halt();
}

void EightBit::mc6809::DAA() {
	auto original = A();

	auto lowPart = lowNibble(original);
	auto lowAdjust = halfCarry() || lowPart > 9;

	auto highPart = highNibble(original);
	auto highAdjust = carry() || highPart > 9 || (highPart == 9 && lowPart > 9);

	uint8_t correction = 0;

	if (lowAdjust)
		correction |= 0x06;

	if (highAdjust)
		correction |= 0x60;

	uint8_t result = original + correction;
	auto newCarry = (correction & 0x60) != 0;
	A() = through(result);
	CC() = setBit(CC(), CF, newCarry);
}

void EightBit::mc6809::EORA() { A() = exclusiveOr(A()); }
void EightBit::mc6809::EORB() { B() = exclusiveOr(B()); }

uint16_t EightBit::mc6809::exclusiveOr(uint16_t operand, uint16_t data) { return through((uint16_t)(operand ^ data)); }

EightBit::register16_t EightBit::mc6809::exclusiveOr(register16_t operand, register16_t data) { return exclusiveOr(operand.joined, data.joined); }

uint8_t EightBit::mc6809::exclusiveOr(uint8_t operand, uint8_t data) { return through((uint8_t)(operand ^ data)); }

uint8_t EightBit::mc6809::exclusiveOr(uint8_t operand) { return exclusiveOr(operand, BUS().DATA()); }

void EightBit::mc6809::DECA() { A() = decrement(A()); }

void EightBit::mc6809::DECB() { B() = decrement(B()); }

void EightBit::mc6809::DEC() {
	auto result = decrement(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::decrement(uint8_t operand) {
	intermediate().joined = operand - 1;
	auto result = intermediate().low;
	CC() = adjustNZ(result);
	CC() = adjustOverflow(operand, 1, intermediate());
	return result;
}

void EightBit::mc6809::INCA() { A() = increment(A()); }

void EightBit::mc6809::INCB() { B() = increment(B()); }

void EightBit::mc6809::INC() {
	auto result = increment(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::increment(uint8_t operand) {
	intermediate().joined = operand + 1;
	auto result = intermediate().low;
	CC() = adjustNZ(result);
	CC() = adjustOverflow(operand, 1, intermediate());
	return result;
}

void EightBit::mc6809::JMP() { jump(EA()); }

void EightBit::mc6809::JSR() {
	swallowEffectiveAddress();
	swallowRead();
	call(EA());
}

void EightBit::mc6809::LSRA() { A() = logicalShiftRight(A()); }

void EightBit::mc6809::LSRB() { B() = logicalShiftRight(B()); }

void EightBit::mc6809::LSR() {
	auto result = logicalShiftRight(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::logicalShiftRight(uint8_t operand) {
	CC() = setBit(CC(), CF, operand & Bit0);
	CC() = adjustNZ(operand >>= 1);
	return operand;
}

void EightBit::mc6809::MUL() {
	swallowRead(9);
	D().joined = A() * B();
	CC() = adjustZero(D());
	CC() = setBit(CC(), CF, D().low & Bit7);
}

void EightBit::mc6809::NEGA() { A() = negate(A()); }

void EightBit::mc6809::NEGB() { B() = negate(B()); }

void EightBit::mc6809::NEG() {
	auto result = negate(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::negate(uint8_t operand) {
	CC() = setBit(CC(), VF, operand == Bit7);
	intermediate().joined = ~operand + 1;
	operand = intermediate().low;
	CC() = adjustNZ(operand);
	CC() = adjustCarry(intermediate());
	return operand;
}

void EightBit::mc6809::ORCC() {
	CC() |= BUS().DATA();
	swallowRead();
}

void EightBit::mc6809::ORA() { A() = _or(A()); }
void EightBit::mc6809::ORB() { B() = _or(B()); }

uint16_t EightBit::mc6809::_or(uint16_t operand, uint16_t data) { return through((uint16_t)(operand | data)); }

EightBit::register16_t EightBit::mc6809::_or(register16_t operand, register16_t data) { return _or(operand.joined, data.joined); }

uint8_t EightBit::mc6809::_or(uint8_t operand, uint8_t data) { return through((uint8_t)(operand | data)); }

uint8_t EightBit::mc6809::_or(uint8_t operand) { return _or(operand, BUS().DATA()); }

void EightBit::mc6809::ROLA() { A() = rotateLeft(A()); }

void EightBit::mc6809::ROLB() { B() = rotateLeft(B()); }

void EightBit::mc6809::ROL() {
	auto result = rotateLeft(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::rotateLeft(uint8_t operand) {
	auto carryIn = carryFlag();
	CC() = setBit(CC(), CF, operand & Bit7);
	CC() = setBit(CC(), VF, (operand & Bit7) >> 7 ^ (operand & Bit6) >> 6);
	uint8_t result = operand << 1 | carryIn;
	CC() = adjustNZ(result);
	return result;
}

void EightBit::mc6809::RORA() { A() = rotateRight(A()); }

void EightBit::mc6809::RORB() { B() = rotateRight(B()); }

void EightBit::mc6809::ROR() {
	auto result = rotateRight(BUS().DATA());
	swallowEffectiveAddress();
	base::memoryWrite(result);
}

uint8_t EightBit::mc6809::rotateRight(uint8_t operand) {
	auto carryIn = carryFlag();
	CC() = setBit(CC(), CF, operand & Bit0);
	uint8_t result = operand >> 1 | carryIn << 7;
	CC() = adjustNZ(result);
	return result;
}

void EightBit::mc6809::RTI() {
	restoreRegisterState();
	swallowRead();
}

void EightBit::mc6809::RTS() {
	ret();
	swallowRead();
}

void EightBit::mc6809::SBCA() { A() = subtractWithCarry(A()); }
void EightBit::mc6809::SBCB() { B() = subtractWithCarry(B()); }

uint8_t EightBit::mc6809::subtractWithCarry(uint8_t operand) { return subtract(operand, BUS().DATA(), carryFlag()); }

void EightBit::mc6809::SUBA() { A() = subtract(A()); }
void EightBit::mc6809::SUBB() { B() = subtract(B()); }

uint8_t EightBit::mc6809::subtract(uint8_t operand) { return subtract(operand, BUS().DATA()); }

uint8_t EightBit::mc6809::subtract(uint8_t operand, uint8_t data, int carry) {
	intermediate().joined = operand - data - carry;
	CC() = adjustSubtraction(operand, data, intermediate());
	return intermediate().low;
}

void EightBit::mc6809::SUBD() { D() = subtract(D(), intermediate()); }

uint16_t EightBit::mc6809::subtract(uint16_t operand, uint16_t data, int carry) {
	auto subtraction = operand - data - carry;
	CC() = adjustSubtraction(operand, data, subtraction);
	swallowRead();
	return subtraction;
}

EightBit::register16_t EightBit::mc6809::subtract(register16_t operand, register16_t data, int carry) { return subtract(operand.joined, data.joined, carry); }

EightBit::register16_t EightBit::mc6809::subtract(register16_t operand, register16_t data) { return subtract(operand, data, 0); }

void EightBit::mc6809::SEX() { A() = SEX(B()); }

uint8_t EightBit::mc6809::SEX(uint8_t from) {
	CC() = adjustNZ(from);
	return (from & Bit7) != 0 ? Mask8 : (uint8_t)0;
}

void EightBit::mc6809::SWI() {
	saveEntireRegisterState();
	CC() = setBit(CC(), IF | FF);  // Disable IRQ / FIRQ
	assert(fastInterruptMasked() && interruptMasked());
	swallowRead();
	Processor::getPagedInto(0xff, _vectorSWI, EA());
	jump(EA());
	swallowRead();
}

void EightBit::mc6809::SWI2() {
	saveEntireRegisterState();
	swallowRead();
	Processor::getPagedInto(0xff, _vectorSWI2, EA());
	jump(EA());
	swallowRead();
}

void EightBit::mc6809::SWI3() {
	saveEntireRegisterState();
	swallowRead();
	Processor::getPagedInto(0xff, _vectorSWI3, EA());
	jump(EA());
	swallowRead();
}

void EightBit::mc6809::TSTA() { test(A()); }

void EightBit::mc6809::TSTB() { test(B()); }

void EightBit::mc6809::TST() {
	test(BUS().DATA());
	swallowRead(2);
}

void EightBit::mc6809::test(uint8_t data) { auto _ = through(data); }

void EightBit::mc6809::LEAX() {
	LEA(X());
	CC() = adjustZero(X());
	swallowRead();
}

void EightBit::mc6809::LEAY() {
	LEA(Y());
	CC() = adjustZero(Y());
	swallowRead();
}

void EightBit::mc6809::LEAS() {
	LEA(S());
	swallowRead();
}

void EightBit::mc6809::LEAU() {
	LEA(U());
	swallowRead();
}

void EightBit::mc6809::LEA(register16_t& destination) {
	destination = EA();
}

#pragma region Save / restore register state

void EightBit::mc6809::saveEntireRegisterState() {
	CC() = setBit(CC(), EF);
	saveRegisterState();
}

void EightBit::mc6809::savePartialRegisterState() {
	CC() = clearBit(CC(), EF);
	saveRegisterState();
}

void EightBit::mc6809::saveRegisterState() {
	swallowRead();
	PSH(S(), entireFlag() ? Mask8 : 0b10000001);
}

void EightBit::mc6809::restoreRegisterState() {
	PUL(S(), entireFlag() ? Mask8 : 0b10000001);
}

void EightBit::mc6809::PSHS() {
	PSH(S());
}

void EightBit::mc6809::PSHU() {
	PSH(U());
}

void EightBit::mc6809::PSH(register16_t& stack) {
	auto control = BUS().DATA();
	swallowRead(2);
	swallowPop(stack);
	PSH(stack, control);
}

void EightBit::mc6809::PSH(register16_t& stack, uint8_t control) {

	// Reverse order of PUL

	// Eight-bit registers

	if ((control & Bit7) != 0)
		push(stack, PC());

	if ((control & Bit6) != 0)
		// Pushing to the S stack means we must be pushing U
		push(stack, &stack == &(S()) ? U() : S());

	if ((control & Bit5) != 0)
		push(stack, Y());

	if ((control & Bit4) != 0)
		push(stack, X());

	// Eight-bit registers

	if ((control & Bit3) != 0)
		push(stack, DP());

	if ((control & Bit2) != 0)
		push(stack, B());

	if ((control & Bit1) != 0)
		push(stack, A());

	if ((control & Bit0) != 0)
		push(stack, CC());
}

void EightBit::mc6809::PULU() {
	PUL(U());
}

void EightBit::mc6809::PULS() {
	PUL(S());
}

void EightBit::mc6809::PUL(register16_t& stack) {
	auto control = BUS().DATA();
	swallowRead(2);
	PUL(stack, control);
	swallowPop(stack);
}

void EightBit::mc6809::PUL(register16_t& stack, uint8_t control) {

	// Reverse order of PSH

	// Eight-bit registers

	if ((control & Bit0) != 0) {
		pop(stack);
		CC() = BUS().DATA();
	}

	if ((control & Bit1) != 0) {
		pop(stack);
		A() = BUS().DATA();
	}

	if ((control & Bit2) != 0) {
		pop(stack);
		B() = BUS().DATA();
	}

	if ((control & Bit3) != 0) {
		pop(stack);
		DP() = BUS().DATA();
	}

	// Sixteen-bit registers

	if ((control & Bit4) != 0)
		X() = popWord(stack);

	if ((control & Bit5) != 0)
		Y() = popWord(stack);

	if ((control & Bit6) != 0)
		// Pulling from the S stack means we must be pulling U
		(&stack == &(S()) ? U() : S()) = popWord(stack);

	if ((control & Bit7) != 0)
		PC() = popWord(stack);
}

#pragma endregion

#pragma region 8 - bit register transfers

uint8_t& EightBit::mc6809::referenceTransfer8(int specifier) {
	switch (specifier) {
	case 0b1000:
		return A();
	case 0b1001:
		return B();
	case 0b1010:
		return CC();
	case 0b1011:
		return DP();
	default:
		UNREACHABLE;
	}
}

EightBit::register16_t& EightBit::mc6809::referenceTransfer16(int specifier) {
	switch (specifier) {
	case 0b0000:
		return D();
	case 0b0001:
		return X();
	case 0b0010:
		return Y();
	case 0b0011:
		return U();
	case 0b0100:
		return S();
	case 0b0101:
		return PC();
	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::EXG() {
	auto immediate = BUS().DATA();

	auto leftSpecifier = highNibble(immediate);
	auto leftType = leftSpecifier & Bit3;

	auto rightSpecifier = lowNibble(immediate);
	auto rightType = rightSpecifier & Bit3;

	if (leftType == 0) {
		auto& leftRegister = referenceTransfer16(leftSpecifier);
		if (rightType == 0) {
			auto& rightRegister = referenceTransfer16(rightSpecifier);
			(leftRegister.joined, rightRegister.joined) = (rightRegister.joined, leftRegister.joined);
		} else {
			auto& rightRegister = referenceTransfer8(rightSpecifier);
			(leftRegister.low, rightRegister) = (rightRegister, leftRegister.low);
			leftRegister.high = Mask8;
		}
	}
	else {
		auto& leftRegister = referenceTransfer8(leftSpecifier);
		if (rightType == 0) {
			auto& rightRegister = referenceTransfer16(rightSpecifier);
			(leftRegister, rightRegister.low) = (rightRegister.low, leftRegister);
			rightRegister.high = Mask8;
		} else {
			auto& rightRegister = referenceTransfer8(rightSpecifier);
			(leftRegister, rightRegister) = (rightRegister, leftRegister);
		}
	}

	swallowRead(6);
}

void EightBit::mc6809::TFR() {
	auto immediate = BUS().DATA();

	auto sourceSpecifier = highNibble(immediate);
	auto sourceType = sourceSpecifier & Bit3;

	auto destinationSpecifier = lowNibble(immediate);
	auto destinationType = destinationSpecifier & Bit3;

	if (sourceType == 0) {
		auto& sourceRegister = referenceTransfer16(sourceSpecifier);
		if (destinationType == 0)
			referenceTransfer16(destinationSpecifier) = sourceRegister;
		else
			referenceTransfer8(destinationSpecifier) = sourceRegister.low;
	} else {
		auto& sourceRegister = referenceTransfer8(sourceSpecifier);
		if (destinationType == 0)
			referenceTransfer16(destinationSpecifier) = { sourceRegister, Mask8 };
		else
			referenceTransfer8(destinationSpecifier) = sourceRegister;
	}

	swallowRead(4);
}

#pragma endregion

#pragma endregion
