#include "stdafx.h"
#include "Disassembly.h"

#include <sstream>
#include <iomanip>
#include <functional>

using namespace std::placeholders;

EightBit::Disassembly::Disassembly(mc6809& targetProcessor)
: m_cpu(targetProcessor) {
}

std::string EightBit::Disassembly::dump_Flags(uint8_t value) {
	std::string returned;
	returned += (value & mc6809::EF) ? "E" : "-";
	returned += (value & mc6809::FF) ? "F" : "-";
	returned += (value & mc6809::HF) ? "H" : "-";
	returned += (value & mc6809::IF) ? "I" : "-";
	returned += (value & mc6809::NF) ? "N" : "-";
	returned += (value & mc6809::ZF) ? "Z" : "-";
	returned += (value & mc6809::VF) ? "V" : "-";
	returned += (value & mc6809::CF) ? "C" : "-";
	return returned;
}

void EightBit::Disassembly::dump(std::ostream& out, int value, int width) {
	out << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << value;
}

std::string EightBit::Disassembly::dump_ByteValue(uint8_t value) {
	std::ostringstream output;
	dump(output, value, 2);
	return output.str();
}

std::string EightBit::Disassembly::dump_RelativeValue(int8_t value) {
	std::ostringstream output;
	output << (int)value;
	return output.str();
}

std::string EightBit::Disassembly::dump_WordValue(uint16_t value) {
	std::ostringstream output;
	dump(output, value, 4);
	return output.str();
}

std::string EightBit::Disassembly::dump_RelativeValue(int16_t value) {
	std::ostringstream output;
	output << (int)value;
	return output.str();
}

std::string EightBit::Disassembly::disassemble(uint16_t current) {

	m_address = current;

	std::ostringstream output;
	if (CPU().powered()) {

		m_prefix10 = m_prefix11 = false;

		const bool ignore =
			CPU().lowered(CPU().HALT())
			|| CPU().lowered(CPU().RESET())
			|| CPU().lowered(CPU().NMI())
			|| (CPU().lowered(CPU().FIRQ()) && !(CPU().CC() & mc6809::FF))
			|| (CPU().lowered(CPU().FIRQ()) && !(CPU().CC() & mc6809::FF))
			|| (CPU().lowered(CPU().IRQ()) && !(CPU().CC() & mc6809::IF));

		if (!ignore) {
			if (m_prefix10)
				output << disassemble10();
			else if (m_prefix11)
				output << disassemble11();
			else
				output << disassembleUnprefixed();
		}
	}

	return output.str();
}

////

std::string EightBit::Disassembly::disassembleUnprefixed() {

	std::ostringstream output;

	const auto opcode = CPU().BUS().peek(m_address);
	output << dump_ByteValue(opcode);

	switch (opcode) {
	case 0x10:	m_prefix10 = true;	output << disassemble(m_address + 1);	break;
	case 0x11:	m_prefix11 = true;	output << disassemble(m_address + 1);	break;
	}

	switch (opcode) {
	// ABX
	//case 0x3a:	X() += B();											break;		// ABX (inherent)

	//// ADC
	//case 0x89:	A() = adc(A(), AM_immediate_byte());				break;		// ADC (ADCA immediate)
	//case 0x99:	A() = adc(A(), AM_direct_byte());					break;		// ADC (ADCA direct)
	//case 0xa9:	A() = adc(A(), AM_indexed_byte());					break;		// ADC (ADCA indexed)
	//case 0xb9:	A() = adc(A(), AM_extended_byte());					break;		// ADC (ADCA extended)

	//case 0xc9:	B() = adc(B(), AM_immediate_byte());				break;		// ADC (ADCB immediate)
	//case 0xd9:	B() = adc(B(), AM_direct_byte());					break;		// ADC (ADCB direct)
	//case 0xe9:	B() = adc(B(), AM_indexed_byte());					break;		// ADC (ADCB indexed)
	//case 0xf9:	B() = adc(B(), AM_extended_byte());					break;		// ADC (ADCB extended)

	//// ADD
	//case 0x8b: A() = add(A(), AM_immediate_byte());				break;		// ADD (ADDA immediate)
	//case 0x9b: A() = add(A(), AM_direct_byte());					break;		// ADD (ADDA direct)
	//case 0xab: A() = add(A(), AM_indexed_byte());					break;		// ADD (ADDA indexed)
	//case 0xbb: A() = add(A(), AM_extended_byte());					break;		// ADD (ADDA extended)

	//case 0xcb: B() = add(B(), AM_immediate_byte());				break;		// ADD (ADDB immediate)
	//case 0xdb: B() = add(B(), AM_direct_byte());					break;		// ADD (ADDB direct)
	//case 0xeb: B() = add(B(), AM_indexed_byte());					break;		// ADD (ADDB indexed)
	//case 0xfb: B() = add(B(), AM_extended_byte());					break;		// ADD (ADDB extended)

	//case 0xc3: D() = add(D(), AM_immediate_word());				break;		// ADD (ADDD immediate)
	//case 0xd3: D() = add(D(), AM_direct_word());					break;		// ADD (ADDD direct)
	//case 0xe3: D() = add(D(), AM_indexed_word());					break;		// ADD (ADDD indexed)
	//case 0xf3: D() = add(D(), AM_extended_word());					break;		// ADD (ADDD extended)

	//// AND
	//case 0x84:	A() = andr(A(), AM_immediate_byte());				break;		// AND (ANDA immediate)
	//case 0x94:	A() = andr(A(), AM_direct_byte());					break;		// AND (ANDA direct)
	//case 0xa4:	A() = andr(A(), AM_indexed_byte());					break;		// AND (ANDA indexed)
	//case 0xb4:	A() = andr(A(), AM_extended_byte());				break;		// AND (ANDA extended)

	//case 0xc4:	B() = andr(B(), AM_immediate_byte());				break;		// AND (ANDB immediate)
	//case 0xd4:	B() = andr(B(), AM_direct_byte());					break;		// AND (ANDB direct)
	//case 0xe4:	B() = andr(B(), AM_indexed_byte());					break;		// AND (ANDB indexed)
	//case 0xf4:	B() = andr(B(), AM_extended_byte());				break;		// AND (ANDB extended)

	//case 0x1c:	CC() &= AM_immediate_byte();						break;		// AND (ANDCC immediate)

	//// ASL/LSL
	//case 0x08:	BUS().write(asl(AM_direct_byte()));					break;		// ASL (direct)
	//case 0x48:	A() = asl(A());										break;		// ASL (ASLA inherent)
	//case 0x58:	B() = asl(B());										break;		// ASL (ASLB inherent)
	//case 0x68:	BUS().write(asl(AM_indexed_byte()));				break;		// ASL (indexed)
	//case 0x78:	BUS().write(asl(AM_extended_byte()));				break;		// ASL (extended)

	//// ASR
	//case 0x07:	BUS().write(asr(AM_direct_byte()));					break;		// ASR (direct)
	//case 0x47:	A() = asr(A());										break;		// ASR (ASRA inherent)
	//case 0x57:	B() = asr(B());										break;		// ASR (ASRB inherent)
	//case 0x67:	BUS().write(asr(AM_indexed_byte()));				break;		// ASR (indexed)
	//case 0x77:	BUS().write(asr(AM_extended_byte()));				break;		// ASR (extended)

	//// BIT
	//case 0x85:	andr(A(), AM_immediate_byte());						break;		// BIT (BITA immediate)
	//case 0x95:	andr(A(), AM_direct_byte());						break;		// BIT (BITA direct)
	//case 0xa5:	andr(A(), AM_indexed_byte());						break;		// BIT (BITA indexed)
	//case 0xb5:	andr(A(), AM_extended_byte());						break;		// BIT (BITA extended)

	//case 0xc5:	andr(B(), AM_immediate_byte());						break;		// BIT (BITB immediate)
	//case 0xd5:	andr(B(), AM_direct_byte());						break;		// BIT (BITB direct)
	//case 0xe5:	andr(B(), AM_indexed_byte());						break;		// BIT (BITB indexed)
	//case 0xf5:	andr(B(), AM_extended_byte());						break;		// BIT (BITB extended)

	//// CLR
	//case 0x0f:	BUS().write(Address_direct(), clr());				break;		// CLR (direct)
	//case 0x4f:	A() = clr();										break;		// CLR (CLRA implied)
	//case 0x5f:	B() = clr();										break;		// CLR (CLRB implied)
	//case 0x6f:	BUS().write(Address_indexed(), clr());				break;		// CLR (indexed)
	//case 0x7f:	BUS().write(Address_extended(), clr());				break;		// CLR (extended)

	//// CMP

	//// CMPA
	//case 0x81:	cmp(A(), AM_immediate_byte());						break;		// CMP (CMPA, immediate)
	//case 0x91:	cmp(A(), AM_direct_byte());							break;		// CMP (CMPA, direct)
	//case 0xa1:	cmp(A(), AM_indexed_byte());						break;		// CMP (CMPA, indexed)
	//case 0xb1:	cmp(A(), AM_extended_byte());						break;		// CMP (CMPA, extended)

	//// CMPB
	//case 0xc1:	cmp(B(), AM_immediate_byte());						break;		// CMP (CMPB, immediate)
	//case 0xd1:	cmp(B(), AM_direct_byte());							break;		// CMP (CMPB, direct)
	//case 0xe1:	cmp(B(), AM_indexed_byte());						break;		// CMP (CMPB, indexed)
	//case 0xf1:	cmp(B(), AM_extended_byte());						break;		// CMP (CMPB, extended)

	//// CMPX
	//case 0x8c:	cmp(X(), AM_immediate_word());						break;		// CMP (CMPX, immediate)
	//case 0x9c:	cmp(X(), AM_direct_word());							break;		// CMP (CMPX, direct)
	//case 0xac:	cmp(X(), AM_indexed_word());						break;		// CMP (CMPX, indexed)
	//case 0xbc:	cmp(X(), AM_extended_word());						break;		// CMP (CMPX, extended)

	//// COM
	//case 0x03:	BUS().write(com(AM_direct_byte()));					break;		// COM (direct)
	//case 0x43:	A() = com(A());										break;		// COM (COMA inherent)
	//case 0x53:	B() = com(B());										break;		// COM (COMB inherent)
	//case 0x63:	BUS().write(com(AM_indexed_byte()));				break;		// COM (indexed)
	//case 0x73:	BUS().write(com(AM_extended_byte()));				break;		// COM (extended)

	//// CWAI
	//case 0x3c:	cwai(AM_direct_byte());								break;		// CWAI (direct)

	//// DAA
	//case 0x19:	A() = da(A());										break;		// DAA (inherent)

	//// DEC
	//case 0x0a:	BUS().write(dec(AM_direct_byte()));					break;		// DEC (direct)
	//case 0x4a:	A() = dec(A());										break;		// DEC (DECA inherent)
	//case 0x5a:	B() = dec(B());										break;		// DEC (DECB inherent)
	//case 0x6a:	BUS().write(dec(AM_indexed_byte()));				break;		// DEC (indexed)
	//case 0x7a:	BUS().write(dec(AM_extended_byte()));				break;		// DEC (extended)

	//// EOR

	//// EORA
	//case 0x88:	A() = eor(A(), AM_immediate_byte());				break;		// EOR (EORA immediate)
	//case 0x98:	A() = eor(A(), AM_direct_byte());					break;		// EOR (EORA direct)
	//case 0xa8:	A() = eor(A(), AM_indexed_byte());					break;		// EOR (EORA indexed)
	//case 0xb8:	A() = eor(A(), AM_extended_byte());					break;		// EOR (EORA extended)

	//// EORB
	//case 0xc8:	B() = eor(B(), AM_immediate_byte());				break;		// EOR (EORB immediate)
	//case 0xd8:	B() = eor(B(), AM_direct_byte());					break;		// EOR (EORB direct)
	//case 0xe8:	B() = eor(B(), AM_indexed_byte());					break;		// EOR (EORB indexed)
	//case 0xf8:	B() = eor(B(), AM_extended_byte());					break;		// EOR (EORB extended)

	//// EXG
	//case 0x1e:	exg(AM_immediate_byte());							break;		// EXG (R1,R2 immediate)

	//// INC
	//case 0x0c:	BUS().write(inc(AM_direct_byte()));					break;		// INC (direct)
	//case 0x4c:	A() = inc(A());										break;		// INC (INCA inherent)
	//case 0x5c:	B() = inc(B());										break;		// INC (INCB inherent)
	//case 0x6c:	BUS().write(inc(AM_indexed_byte()));				break;		// INC (indexed)
	//case 0x7c:	BUS().write(inc(AM_extended_byte()));				break;		// INC (extended)

	//// JMP
	//case 0x0e:	jump(Address_direct());								break;		// JMP (direct)
	//case 0x6e:	jump(Address_indexed());							break;		// JMP (indexed)
	//case 0x7e:	jump(Address_extended());							break;		// JMP (extended)

	//// JSR
	//case 0x9d:	call(Address_direct());								break;		// JSR (direct)
	//case 0xad:	call(Address_indexed());							break;		// JSR (indexed)
	//case 0xbd:	call(Address_extended());							break;		// JSR (extended)

	//// LD

	//// LDA
	//case 0x86:	A() = ld(AM_immediate_byte());						break;		// LD (LDA immediate)
	//case 0x96:	A() = ld(AM_direct_byte());							break;		// LD (LDA direct)
	//case 0xa6:	A() = ld(AM_indexed_byte());						break;		// LD (LDA indexed)
	//case 0xb6:	A() = ld(AM_extended_byte());						break;		// LD (LDA extended)

	//// LDB
	//case 0xc6:	B() = ld(AM_immediate_byte());						break;		// LD (LDB immediate)
	//case 0xd6:	B() = ld(AM_direct_byte());							break;		// LD (LDB direct)
	//case 0xe6:	B() = ld(AM_indexed_byte());						break;		// LD (LDB indexed)
	//case 0xf6:	B() = ld(AM_extended_byte());						break;		// LD (LDB extended)

	//// LDD
	//case 0xcc:	D() = ld(AM_immediate_word());						break;		// LD (LDD immediate)
	//case 0xdc:	D() = ld(AM_direct_word());							break;		// LD (LDD direct)
	//case 0xec:	D() = ld(AM_indexed_word());						break;		// LD (LDD indexed)
	//case 0xfc:	D() = ld(AM_extended_word());						break;		// LD (LDD extended)

	//// LDU
	//case 0xce:	U() = ld(AM_immediate_word());						break;		// LD (LDU immediate)
	//case 0xde:	U() = ld(AM_direct_word());							break;		// LD (LDU direct)
	//case 0xee:	U() = ld(AM_indexed_word());						break;		// LD (LDU indexed)
	//case 0xfe:	U() = ld(AM_extended_word());						break;		// LD (LDU extended)

	//// LDX
	//case 0x8e:	X() = ld(AM_immediate_word());						break;		// LD (LDX immediate)
	//case 0x9e:	X() = ld(AM_direct_word());							break;		// LD (LDX direct)
	//case 0xae:	X() = ld(AM_indexed_word());						break;		// LD (LDX indexed)
	//case 0xbe:	X() = ld(AM_extended_word());						break;		// LD (LDX extended)

	//// LEA
	//case 0x30:	adjustZero(X() = Address_indexed());				break;		// LEA (LEAX indexed)
	case 0x31:	output << Address_indexed("LEAY");	break;	// LEA (LEAY indexed)
	//case 0x32:	S() = Address_indexed();							break;		// LEA (LEAS indexed)
	//case 0x33:	U() = Address_indexed();							break;		// LEA (LEAU indexed)

	//// LSR
	//case 0x04:	BUS().write(lsr(AM_direct_byte()));					break;		// LSR (direct)
	//case 0x44:	A() = lsr(A());										break;		// LSR (LSRA inherent)
	//case 0x54:	B() = lsr(B());										break;		// LSR (LSRB inherent)
	//case 0x64:	BUS().write(lsr(AM_indexed_byte()));				break;		// LSR (indexed)
	//case 0x74:	BUS().write(lsr(AM_extended_byte()));				break;		// LSR (extended)

	//// MUL
	//case 0x3d:	D() = mul(A(), B());								break;		// MUL (inherent)

	//// NEG
	//case 0x00:	BUS().write(neg(AM_direct_byte()));					break;		// NEG (direct)
	//case 0x40:	A() = neg(A());										break;		// NEG (NEGA, inherent)
	//case 0x50:	B() = neg(B());										break;		// NEG (NEGB, inherent)
	//case 0x60:	BUS().write(neg(AM_indexed_byte()));				break;		// NEG (indexed)
	//case 0x70:	BUS().write(neg(AM_extended_byte()));				break;		// NEG (extended)

	//// NOP
	//case 0x12:	break;		// NOP (inherent)

	//// OR

	//// ORA
	//case 0x8a:	A() = orr(A(), AM_immediate_byte());				break;		// OR (ORA immediate)
	//case 0x9a:	A() = orr(A(), AM_direct_byte());					break;		// OR (ORA direct)
	//case 0xaa:	A() = orr(A(), AM_indexed_byte());					break;		// OR (ORA indexed)
	//case 0xba:	A() = orr(A(), AM_extended_byte());					break;		// OR (ORA extended)

	//// ORB
	//case 0xca:	A() = orr(A(), AM_immediate_byte());				break;		// OR (ORB immediate)
	//case 0xda:	A() = orr(A(), AM_direct_byte());					break;		// OR (ORB direct)
	//case 0xea:	A() = orr(A(), AM_indexed_byte());					break;		// OR (ORB indexed)
	//case 0xfa:	A() = orr(A(), AM_extended_byte());					break;		// OR (ORB extended)

	//// ORCC
	//case 0x1a:	CC() |= AM_immediate_byte();						break;		// OR (ORCC immediate)

	//// PSH
	//case 0x34:	pshs(AM_immediate_byte());							break;		// PSH (PSHS immediate)
	//case 0x36:	pshu(AM_immediate_byte());							break;		// PSH (PSHU immediate)

	//// PUL
	//case 0x35:	puls(AM_immediate_byte());							break;		// PUL (PULS immediate)
	//case 0x37:	pulu(AM_immediate_byte());							break;		// PUL (PULU immediate)

	//// ROL
	//case 0x09:	BUS().write(rol(AM_direct_byte()));					break;		// ROL (direct)
	//case 0x49:	A() = rol(A());										break;		// ROL (ROLA inherent)
	//case 0x59:	B() = rol(B());										break;		// ROL (ROLB inherent)
	//case 0x69:	BUS().write(rol(AM_indexed_byte()));				break;		// ROL (indexed)
	//case 0x79:	BUS().write(rol(AM_extended_byte()));				break;		// ROL (extended)

	//// ROR
	//case 0x06:	BUS().write(ror(AM_direct_byte()));					break;		// ROR (direct)
	//case 0x46:	A() = ror(A());										break;		// ROR (RORA inherent)
	//case 0x56:	B() = ror(B());										break;		// ROR (RORB inherent)
	//case 0x66:	BUS().write(ror(AM_indexed_byte()));				break;		// ROR (indexed)
	//case 0x76:	BUS().write(ror(AM_extended_byte()));				break;		// ROR (extended)

	//// RTI
	//case 0x3B:	rti();												break;		// RTI (inherent)

	//// RTS
	//case 0x39:	rts();												break;		// RTS (inherent)

	//// SBC

	//// SBCA
	//case 0x82:	A() = sbc(A(), AM_immediate_byte());				break;		// SBC (SBCA immediate)
	//case 0x92:	A() = sbc(A(), AM_direct_byte());					break;		// SBC (SBCA direct)
	//case 0xa2:	A() = sbc(A(), AM_indexed_byte());					break;		// SBC (SBCA indexed)
	//case 0xb2:	A() = sbc(A(), AM_extended_byte());					break;		// SBC (SBCB extended)

	//// SBCB
	//case 0xc2:	B() = sbc(B(), AM_immediate_byte());				break;		// SBC (SBCB immediate)
	//case 0xd2:	B() = sbc(B(), AM_direct_byte());					break;		// SBC (SBCB direct)
	//case 0xe2:	B() = sbc(B(), AM_indexed_byte());					break;		// SBC (SBCB indexed)
	//case 0xf2:	B() = sbc(B(), AM_extended_byte());					break;		// SBC (SBCB extended)

	//// SEX
	//case 0x1d:	A() = sex(B());										break;		// SEX (inherent)

	//// ST

	//// STA
	//case 0x97:	BUS().write(Address_direct(), st(A()));				break;		// ST (STA direct)
	//case 0xa7:	BUS().write(Address_indexed(), st(A()));			break;		// ST (STA indexed)
	//case 0xb7:	BUS().write(Address_extended(), st(A()));			break;		// ST (STA extended)

	//// STB
	//case 0xd7:	BUS().write(Address_direct(), st(B()));				break;		// ST (STB direct)
	//case 0xe7:	BUS().write(Address_indexed(), st(B()));			break;		// ST (STB indexed)
	//case 0xf7:	BUS().write(Address_extended(), st(B()));			break;		// ST (STB extended)

	//// STD
	//case 0xdd:	Processor::setWord(Address_direct(), st(D()));		break;		// ST (STD direct)
	//case 0xed:	Processor::setWord(Address_indexed(), st(D()));		break;		// ST (STD indexed)
	//case 0xfd:	Processor::setWord(Address_extended(), st(D()));	break;		// ST (STD extended)

	//// STU
	//case 0xdf:	Processor::setWord(Address_direct(), st(U()));		break;		// ST (STU direct)
	//case 0xef:	Processor::setWord(Address_indexed(), st(U()));		break;		// ST (STU indexed)
	//case 0xff:	Processor::setWord(Address_extended(), st(U()));	break;		// ST (STU extended)

	//// STX
	//case 0x9f:	Processor::setWord(Address_direct(), st(X()));		break;		// ST (STX direct)
	//case 0xaf:	Processor::setWord(Address_indexed(), st(X()));		break;		// ST (STX indexed)
	//case 0xbf:	Processor::setWord(Address_extended(), st(X()));	break;		// ST (STX extended)

	//// SUB

	//// SUBA
	//case 0x80:	A() = sub(A(), AM_immediate_byte());				break;		// SUB (SUBA immediate)
	//case 0x90:	A() = sub(A(), AM_direct_byte());					break;		// SUB (SUBA direct)
	//case 0xa0:	A() = sub(A(), AM_indexed_byte());					break;		// SUB (SUBA indexed)
	//case 0xb0:	A() = sub(A(), AM_extended_byte());					break;		// SUB (SUBA extended)

	//// SUBB
	//case 0xc0:	B() = sub(B(), AM_immediate_byte());				break;		// SUB (SUBB immediate)
	//case 0xd0:	B() = sub(B(), AM_direct_byte());					break;		// SUB (SUBB direct)
	//case 0xe0:	B() = sub(B(), AM_indexed_byte());					break;		// SUB (SUBB indexed)
	//case 0xf0:	B() = sub(B(), AM_extended_byte());					break;		// SUB (SUBB extended)

	//// SUBD
	//case 0x83:	D() = sub(D(), AM_immediate_word());				break;		// SUB (SUBD immediate)
	//case 0x93:	D() = sub(D(), AM_direct_word());					break;		// SUB (SUBD direct)
	//case 0xa3:	D() = sub(D(), AM_indexed_word());					break;		// SUB (SUBD indexed)
	//case 0xb3:	D() = sub(D(), AM_extended_word());					break;		// SUB (SUBD extended)

	//// SWI
	//case 0x3f:	swi();												break;		// SWI (inherent)

	//// SYNC
	//case 0x13:	halt();												break;		// SYNC (inherent)

	//// TFR
	//case 0x1f:	tfr(AM_immediate_byte());							break;		// TFR (immediate)

	//// TST
	//case 0x0d:	tst(AM_direct_byte());								break;		// TST (direct)
	//case 0x4d:	tst(A());											break;		// TST (TSTA inherent)
	//case 0x5d:	tst(B());											break;		// TST (TSTB inherent)
	//case 0x6d:	tst(AM_indexed_byte());								break;		// TST (indexed)
	//case 0x7d:	tst(AM_extended_byte());							break;		// TST (extended)

	//// Branching

	//case 0x16:	jump(Address_relative_word());						break;		// BRA (LBRA relative)
	//case 0x17:	call(Address_relative_word());						break;		// BSR (LBSR relative)
	//case 0x20:	jump(Address_relative_byte());						break;		// BRA (relative)
	//case 0x21:	Address_relative_byte();							break;		// BRN (relative)
	//case 0x22:	branchShort(BHI());									break;		// BHI (relative)
	//case 0x23:	branchShort(BLS());									break;		// BLS (relative)
	//case 0x24:	branchShort(!carry());								break;		// BCC (relative)
	//case 0x25:	branchShort(carry());								break;		// BCS (relative)
	//case 0x26:	branchShort(!zero());								break;		// BNE (relative)
	//case 0x27:	branchShort(zero());								break;		// BEQ (relative)
	//case 0x28: 	branchShort(!overflow());							break;		// BVC (relative)
	//case 0x29: 	branchShort(overflow());							break;		// BVS (relative)
	//case 0x2a: 	branchShort(!negative());							break;		// BPL (relative)
	//case 0x2b: 	branchShort(negative());							break;		// BMI (relative)
	//case 0x2c:	branchShort(BGE());									break;		// BGE (relative)
	//case 0x2d:	branchShort(BLT());									break;		// BLT (relative)
	//case 0x2e:	branchShort(BGT());									break;		// BGT (relative)
	//case 0x2f:	branchShort(BLE());									break;		// BLE (relative)

	//case 0x8d:	call(Address_relative_byte());						break;		// BSR (relative)

	//default:
	//	UNREACHABLE;
	}

	return output.str();
}

std::string EightBit::Disassembly::disassemble10() {

	std::ostringstream output;

	const auto opcode = CPU().BUS().peek(m_address);
	output << dump_ByteValue(opcode);

	switch (opcode) {

	// CMP

	// CMPD
	//case 0x83:	cmp(D(), AM_immediate_word());						break;		// CMP (CMPD, immediate)
	//case 0x93:	cmp(D(), AM_direct_word());							break;		// CMP (CMPD, direct)
	//case 0xa3:	cmp(D(), AM_indexed_word());						break;		// CMP (CMPD, indexed)
	//case 0xb3:	cmp(D(), AM_extended_word());						break;		// CMP (CMPD, extended)

	//// CMPY
	//case 0x8c:	cmp(Y(), AM_immediate_word());						break;		// CMP (CMPY, immediate)
	//case 0x9c:	cmp(Y(), AM_direct_word());							break;		// CMP (CMPY, direct)
	//case 0xac:	cmp(Y(), AM_indexed_word());						break;		// CMP (CMPY, indexed)
	//case 0xbc:	cmp(Y(), AM_extended_word());						break;		// CMP (CMPY, extended)

	//// LD

	//// LDS
	//case 0xce:	S() = ld(AM_immediate_word());						break;		// LD (LDS immediate)
	//case 0xde:	S() = ld(AM_direct_word());							break;		// LD (LDS direct)
	//case 0xee:	S() = ld(AM_indexed_word());						break;		// LD (LDS indexed)
	//case 0xfe:	S() = ld(AM_extended_word());						break;		// LD (LDS extended)

	//// LDY
	//case 0x8e:	Y() = ld(AM_immediate_word());						break;		// LD (LDY immediate)
	//case 0x9e:	Y() = ld(AM_direct_word());							break;		// LD (LDY direct)
	//case 0xae:	Y() = ld(AM_indexed_word());						break;		// LD (LDY indexed)
	//case 0xbe:	Y() = ld(AM_extended_word());						break;		// LD (LDY extended)

	//// Branching

	//case 0x21:	Address_relative_word();							break;		// BRN (LBRN relative)
	//case 0x22:	if (branchLong(BHI())) addCycle();					break;		// BHI (LBHI relative)
	//case 0x23:	if (branchLong(BLS())) addCycle();					break;		// BLS (LBLS relative)
	//case 0x24:	if (branchLong(!carry())) addCycle();				break;		// BCC (LBCC relative)
	//case 0x25:	if (branchLong(carry())) addCycle();				break;		// BCS (LBCS relative)
	//case 0x26:	if (branchLong(!zero())) addCycle();				break;		// BNE (LBNE relative)
	//case 0x27:	if (branchLong(zero())) addCycle();					break;		// BEQ (LBEQ relative)
	//case 0x28:	if (branchLong(!overflow())) addCycle();			break;		// BVC (LBVC relative)
	//case 0x29:	if (branchLong(overflow())) addCycle();				break;		// BVS (LBVS relative)
	//case 0x2a:	if (branchLong(!negative())) addCycle();			break;		// BPL (LBPL relative)
	//case 0x2b: 	if (branchLong(negative())) addCycle();				break;		// BMI (LBMI relative)
	//case 0x2c:	if (branchLong(BGE())) addCycle();					break;		// BGE (LBGE relative)
	//case 0x2d:	if (branchLong(BLT())) addCycle();					break;		// BLT (LBLT relative)
	//case 0x2e:	if (branchLong(BGT())) addCycle();					break;		// BGT (LBGT relative)
	//case 0x2f:	if (branchLong(BLE())) addCycle();					break;		// BLE (LBLE relative)

	//// STS
	//case 0xdf:	Processor::setWord(Address_direct(), st(S()));		break;		// ST (STS direct)
	//case 0xef:	Processor::setWord(Address_indexed(), st(S()));		break;		// ST (STS indexed)
	//case 0xff:	Processor::setWord(Address_extended(), st(S()));	break;		// ST (STS extended)

	//// STY
	//case 0x9f:	Processor::setWord(Address_extended(), st(Y()));	break;		// ST (STY direct)
	//case 0xaf:	Processor::setWord(Address_indexed(), st(Y()));		break;		// ST (STY indexed)
	//case 0xbf:	Processor::setWord(Address_extended(), st(Y()));	break;		// ST (STY extended)

	//// SWI
	//case 0x3f:	swi2();												break;		// SWI (SWI2 inherent)

	//default:
	//	UNREACHABLE;
	}

	return output.str();
}

std::string EightBit::Disassembly::disassemble11() {

	std::ostringstream output;

	const auto opcode = CPU().BUS().peek(m_address);
	output << dump_ByteValue(opcode);

	switch (opcode) {

	//// CMP

	//// CMPU
	//case 0x83:	cmp(U(), AM_immediate_word());						break;		// CMP (CMPU, immediate)
	//case 0x93:	cmp(U(), AM_direct_word());							break;		// CMP (CMPU, direct)
	//case 0xa3:	cmp(U(), AM_indexed_word());						break;		// CMP (CMPU, indexed)
	//case 0xb3:	cmp(U(), AM_extended_word());						break;		// CMP (CMPU, extended)

	//// CMPS
	//case 0x8c:	cmp(S(), AM_immediate_word());						break;		// CMP (CMPS, immediate)
	//case 0x9c:	cmp(S(), AM_direct_word());							break;		// CMP (CMPS, direct)
	//case 0xac:	cmp(S(), AM_indexed_word());						break;		// CMP (CMPS, indexed)
	//case 0xbc:	cmp(S(), AM_extended_word());						break;		// CMP (CMPS, extended)

	//// SWI
	//case 0x3f:	swi3();												break;		// SWI (SWI3 inherent)

	//default:
	//	UNREACHABLE;
	}

	return output.str();
}

////

std::string EightBit::Disassembly::RR(int which) {
	switch (which) {
	case 0b00:
		return "X";
	case 0b01:
		return "Y";
	case 0b10:
		return "U";
	case 0b11:
		return "S";
	default:
		UNREACHABLE;
	}
}

std::string EightBit::Disassembly::Address_indexed(std::string mnemomic) {

	std::ostringstream output;

	auto& bus = CPU().BUS();

	const auto type = bus.peek(++m_address);
	const auto r = RR((type & (Processor::Bit6 | Processor::Bit5)) >> 5);

	auto byte = 0xff;
	auto word = 0xffff;

	output << dump_ByteValue(type);

	if (type & Processor::Bit7) {
		const auto indirect = type & Processor::Bit4;
		switch (type & Processor::Mask4) {
		case 0b0000:	// ,R+
			output << mnemomic << " ," << r << "+";
			break;
		case 0b0001:	// ,R++
			output << mnemomic << " ," << r << "++";
			break;
		case 0b0010:	// ,-R
			output << mnemomic << " ,-" << r;
			break;
		case 0b0011:	// ,--R
			output << mnemomic << " ,--" << r;
			break;
		case 0b0100:	// ,R
			output << mnemomic << " ," << r;
			break;
		case 0b0101:	// B,R
			output << mnemomic << " B," << r;
			break;
		case 0b0110:	// A,R
			output << mnemomic << " A," << r;
			break;
		case 0b1000:	// n,R (eight-bit)
			byte = bus.peek(++m_address);
			output << mnemomic << " " << dump_Byte(byte) << "," << r;
			break;
		case 0b1001:	// n,R (sixteen-bit)
			word = bus.peekWord(++m_address);
			output << dump_WordValue(word) << "\t";
			output << mnemomic << " " << dump_Word(word) << "," << r;
			break;
		case 0b1011:	// D,R
			output << mnemomic << " D," << r;
			break;
		case 0b1100:	// n,PCR (eight-bit)
			byte = bus.peek(++m_address);
			output << dump_ByteValue(byte) << "\t";
			output << mnemomic << " " << dump_RelativeValue((int8_t)byte) << ",PCR";
			break;
		case 0b1101:	// n,PCR (sixteen-bit)
			word = bus.peekWord(++m_address);
			output << dump_WordValue(word) << "\t";
			output << mnemomic << " " << dump_Word(word) << ",PCR";
			break;
		}
	} else {
		// EA = ,R + 5-bit offset
		output << mnemomic << dump_Byte(type & Processor::Mask5) << "," << r;
}

	return output.str();
}

////

uint8_t EightBit::Disassembly::getByte(uint16_t address) {
	return CPU().BUS().peek(address);
}

uint16_t EightBit::Disassembly::getWord(uint16_t address) {
	return CPU().BUS().peekWord(address);
}

////

std::string EightBit::Disassembly::dump_Byte(uint16_t address) {
	return dump_ByteValue(getByte(address));
}

std::string EightBit::Disassembly::dump_DByte(uint16_t address) {
	return dump_Byte(address) + " " + dump_Byte(address + 1);
}

std::string EightBit::Disassembly::dump_Word(uint16_t address) {
	return dump_WordValue(getWord(address));
}
 