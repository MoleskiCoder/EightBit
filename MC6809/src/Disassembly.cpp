#include "stdafx.h"
#include "../inc/Disassembly.h"

#include <vector>
#include <sstream>
#include <iomanip>
#include <functional>

using namespace std::placeholders;

EightBit::Disassembly::Disassembly(Bus& bus, mc6809& targetProcessor)
: m_bus(bus),
  m_cpu(targetProcessor) {
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
	out << std::hex << std::nouppercase << std::setw(width) << std::setfill('0') << value;
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

std::string EightBit::Disassembly::dump_WordValue(register16_t value) {
	return dump_WordValue(value.word);
}

std::string EightBit::Disassembly::dump_WordValue(uint16_t value) {
	std::ostringstream output;
	dump(output, value, 4);
	return output.str();
}

std::string EightBit::Disassembly::dump_RelativeValue(register16_t value) {
	return dump_RelativeValue((int16_t)value.word);
}

std::string EightBit::Disassembly::dump_RelativeValue(int16_t value) {
	std::ostringstream output;
	output << (int)value;
	return output.str();
}

//

std::string EightBit::Disassembly::trace() {
	return trace(CPU().PC());
}

std::string EightBit::Disassembly::trace(register16_t current) {
	return trace(current.word);
}

std::string EightBit::Disassembly::trace(uint16_t current) {

	std::ostringstream output;

	output
		<< dump_WordValue(current) << "| "
		<< disassemble(current)
		<< "\t\t"
		<< "cc=" << dump_ByteValue(CPU().CC()) << " "
		<< "a=" << dump_ByteValue(CPU().A()) << " "
		<< "b=" << dump_ByteValue(CPU().B()) << " "
		<< "dp=" << dump_ByteValue(CPU().DP()) << " "
		<< "x=" << dump_WordValue(CPU().X()) << " "
		<< "y=" << dump_WordValue(CPU().Y()) << " "
		<< "u=" << dump_WordValue(CPU().U()) << " "
		<< "s=" << dump_WordValue(CPU().S());

	return output.str();
}

//

std::string EightBit::Disassembly::disassemble() {
	return disassemble(CPU().PC());
}

std::string EightBit::Disassembly::disassemble(register16_t current) {
	return disassemble(current.word);
}

bool EightBit::Disassembly::ignore() {
	return
			CPU().lowered(CPU().HALT())
		||	CPU().lowered(CPU().RESET())
		||	CPU().lowered(CPU().NMI())
		|| (CPU().lowered(CPU().FIRQ()) && !(CPU().CC() & mc6809::FF))
		|| (CPU().lowered(CPU().INT()) && !(CPU().CC() & mc6809::IF));
}

std::string EightBit::Disassembly::disassemble(uint16_t current) {

	m_address = current;

	std::ostringstream output;
	if (CPU().powered()) {
		if (m_prefix10)
			output << disassemble10();
		else if (m_prefix11)
			output << disassemble11();
		else
			output << disassembleUnprefixed();
	}

	return output.str();
}

////

std::string EightBit::Disassembly::disassembleUnprefixed() {

	std::ostringstream output;

	const auto opcode = getByte(m_address);
	output << dump_ByteValue(opcode);

	switch (opcode) {

	case 0x10:	m_prefix10 = true;	output << disassemble(m_address + 1);	break;
	case 0x11:	m_prefix11 = true;	output << disassemble(m_address + 1);	break;

	// ABX
	case 0x3a:	output << "\tABX";						break;		// ABX (inherent)

	// ADC
	case 0x89:	output << AM_immediate_byte("ADCA");		break;		// ADC (ADCA immediate)
	case 0x99:	output << Address_direct("ADCA");		break;		// ADC (ADCA direct)
	case 0xa9:	output << Address_indexed("ADCA");		break;		// ADC (ADCA indexed)
	case 0xb9:	output << Address_extended("ADCA");		break;		// ADC (ADCA extended)

	case 0xc9:	output << AM_immediate_byte("ADCB");		break;		// ADC (ADCB immediate)
	case 0xd9:	output << Address_direct("ADCB");		break;		// ADC (ADCB direct)
	case 0xe9:	output << Address_indexed("ADCB");		break;		// ADC (ADCB indexed)
	case 0xf9:	output << Address_extended("ADCB");		break;		// ADC (ADCB extended)

	// ADD
	case 0x8b: output << Address_extended("ADDA");		break;		// ADD (ADDA immediate)
	case 0x9b: output << Address_direct("ADDA");			break;		// ADD (ADDA direct)
	case 0xab: output << Address_indexed("ADDA");		break;		// ADD (ADDA indexed)
	case 0xbb: output << Address_extended("ADDA");		break;		// ADD (ADDA extended)

	case 0xcb: output << Address_extended("ADDB");		break;		// ADD (ADDB immediate)
	case 0xdb: output << Address_direct("ADDB");			break;		// ADD (ADDB direct)
	case 0xeb: output << Address_indexed("ADDB");		break;		// ADD (ADDB indexed)
	case 0xfb: output << Address_extended("ADDB");		break;		// ADD (ADDB extended)

	case 0xc3: output << AM_immediate_word("ADDD");		break;		// ADD (ADDD immediate)
	case 0xd3: output << Address_direct("ADDD");			break;		// ADD (ADDD direct)
	case 0xe3: output << Address_indexed("ADDD");		break;		// ADD (ADDD indexed)
	case 0xf3: output << Address_extended("ADDD");		break;		// ADD (ADDD extended)

	// AND
	case 0x84:	output << AM_immediate_byte("ANDA");		break;		// AND (ANDA immediate)
	case 0x94:	output << Address_direct("ANDA");		break;		// AND (ANDA direct)
	case 0xa4:	output << Address_indexed("ANDA");		break;		// AND (ANDA indexed)
	case 0xb4:	output << Address_extended("ANDA");		break;		// AND (ANDA extended)

	case 0xc4:	output << AM_immediate_byte("ANDB");		break;		// AND (ANDB immediate)
	case 0xd4:	output << Address_direct("ANDB");		break;		// AND (ANDB direct)
	case 0xe4:	output << Address_indexed("ANDB");		break;		// AND (ANDB indexed)
	case 0xf4:	output << Address_extended("ANDB");		break;		// AND (ANDB extended)

	case 0x1c:	output << AM_immediate_byte("ANDCC");	break;		// AND (ANDCC immediate)

	// ASL/LSL
	case 0x08:	output << Address_direct("ASL");			break;		// ASL (direct)
	case 0x48:	output << "\tASLA\t";					break;		// ASL (ASLA inherent)
	case 0x58:	output << "\tASLB\t";					break;		// ASL (ASLB inherent)
	case 0x68:	output << Address_indexed("ASL");		break;		// ASL (indexed)
	case 0x78:	output << Address_extended("ASL");		break;		// ASL (extended)

	// ASR
	case 0x07:	output << Address_direct("ASR");			break;		// ASR (direct)
	case 0x47:	output << "\tASRA\t";					break;		// ASR (ASRA inherent)
	case 0x57:	output << "\tASRB\t";					break;		// ASR (ASRB inherent)
	case 0x67:	output << Address_indexed("ASR");		break;		// ASR (indexed)
	case 0x77:	output << Address_extended("ASR");		break;		// ASR (extended)

	// BIT
	case 0x85:	output << AM_immediate_byte("BITA");		break;		// BIT (BITA immediate)
	case 0x95:	output << Address_direct("BITA");		break;		// BIT (BITA direct)
	case 0xa5:	output << Address_indexed("BITA");		break;		// BIT (BITA indexed)
	case 0xb5:	output << Address_extended("BITA");		break;		// BIT (BITA extended)

	case 0xc5:	output << AM_immediate_byte("BITB");		break;		// BIT (BITB immediate)
	case 0xd5:	output << Address_direct("BITB");		break;		// BIT (BITB direct)
	case 0xe5:	output << Address_indexed("BITB");		break;		// BIT (BITB indexed)
	case 0xf5:	output << Address_extended("BITB");		break;		// BIT (BITB extended)

	// CLR
	case 0x0f:	output << Address_direct("CLR");			break;		// CLR (direct)
	case 0x4f:	output << "\tCLRA\t";					break;		// CLR (CLRA implied)
	case 0x5f:	output << "\tCLRB\t";					break;		// CLR (CLRB implied)
	case 0x6f:	output << Address_indexed("CLR");		break;		// CLR (indexed)
	case 0x7f:	output << Address_extended("CLR");		break;		// CLR (extended)

	// CMP

	// CMPA
	case 0x81:	output << AM_immediate_byte("CMPA");		break;		// CMP (CMPA, immediate)
	case 0x91:	output << Address_direct("CMPA");		break;		// CMP (CMPA, direct)
	case 0xa1:	output << Address_indexed("CMPA");		break;		// CMP (CMPA, indexed)
	case 0xb1:	output << Address_extended("CMPA");		break;		// CMP (CMPA, extended)

	// CMPB
	case 0xc1:	output << AM_immediate_byte("CMPB");		break;		// CMP (CMPB, immediate)
	case 0xd1:	output << Address_direct("CMPB");		break;		// CMP (CMPB, direct)
	case 0xe1:	output << Address_indexed("CMPB");		break;		// CMP (CMPB, indexed)
	case 0xf1:	output << Address_extended("CMPB");		break;		// CMP (CMPB, extended)

	// CMPX
	case 0x8c:	output << AM_immediate_word("CMPX");		break;		// CMP (CMPX, immediate)
	case 0x9c:	output << Address_direct("CMPX");		break;		// CMP (CMPX, direct)
	case 0xac:	output << Address_indexed("CMPX");		break;		// CMP (CMPX, indexed)
	case 0xbc:	output << Address_extended("CMPX");		break;		// CMP (CMPX, extended)

	// COM
	case 0x03:	output << Address_direct("COM");			break;		// COM (direct)
	case 0x43:	output << "\tCOMA\t";					break;		// COM (COMA inherent)
	case 0x53:	output << "\tCOMB\t";					break;		// COM (COMB inherent)
	case 0x63:	output << Address_indexed("COM");		break;		// COM (indexed)
	case 0x73:	output << Address_extended("COM");		break;		// COM (extended)

	// CWAI
	case 0x3c:	output << Address_direct("CWAI");		break;		// CWAI (direct)

	// DAA
	case 0x19:	output << "\tDAA";						break;		// DAA (inherent)

	// DEC
	case 0x0a:	output << Address_direct("DEC");			break;		// DEC (direct)
	case 0x4a:	output << "\tDECA\t";					break;		// DEC (DECA inherent)
	case 0x5a:	output << "\tDECB\t";					break;		// DEC (DECB inherent)
	case 0x6a:	output << Address_indexed("DEC");		break;		// DEC (indexed)
	case 0x7a:	output << Address_extended("DEC");		break;		// DEC (extended)

	// EOR

	// EORA
	case 0x88:	output << AM_immediate_byte("EORA");		break;		// EOR (EORA immediate)
	case 0x98:	output << Address_direct("EORA");		break;		// EOR (EORA direct)
	case 0xa8:	output << Address_indexed("EORA");		break;		// EOR (EORA indexed)
	case 0xb8:	output << Address_extended("EORA");		break;		// EOR (EORA extended)

	// EORB
	case 0xc8:	output << AM_immediate_byte("EORB");		break;		// EOR (EORB immediate)
	case 0xd8:	output << Address_direct("EORB");		break;		// EOR (EORB direct)
	case 0xe8:	output << Address_indexed("EORB");		break;		// EOR (EORB indexed)
	case 0xf8:	output << Address_extended("EORB");		break;		// EOR (EORB extended)

	// EXG
	case 0x1e:	output << tfr("EXG");					break;		// EXG (R1,R2 immediate)

	// INC
	case 0x0c:	output << Address_direct("INC");			break;		// INC (direct)
	case 0x4c:	output << "\tINCA\t";;					break;		// INC (INCA inherent)
	case 0x5c:	output << "\tINCB\t";;					break;		// INC (INCB inherent)
	case 0x6c:	output << Address_indexed("INC");		break;		// INC (indexed)
	case 0x7c:	output << Address_extended("INC");		break;		// INC (extended)

	// JMP
	case 0x0e:	output << Address_direct("JMP");			break;		// JMP (direct)
	case 0x6e:	output << Address_indexed("JMP");		break;		// JMP (indexed)
	case 0x7e:	output << Address_extended("JMP");		break;		// JMP (extended)

	// JSR
	case 0x9d:	output << Address_direct("JSR");			break;		// JSR (direct)
	case 0xad:	output << Address_indexed("JSR");		break;		// JSR (indexed)
	case 0xbd:	output << Address_extended("JSR");		break;		// JSR (extended)

	// LD

	// LDA
	case 0x86:	output << AM_immediate_byte("LDA");		break;		// LD (LDA immediate)
	case 0x96:	output << Address_direct("LDA");			break;		// LD (LDA direct)
	case 0xa6:	output << Address_indexed("LDA");		break;		// LD (LDA indexed)
	case 0xb6:	output << Address_extended("LDA");		break;		// LD (LDA extended)

	// LDB
	case 0xc6:	output << AM_immediate_byte("LDB");		break;		// LD (LDB immediate)
	case 0xd6:	output << Address_direct("LDB");			break;		// LD (LDB direct)
	case 0xe6:	output << Address_indexed("LDB");		break;		// LD (LDB indexed)
	case 0xf6:	output << Address_extended("LDB");		break;		// LD (LDB extended)

	// LDD
	case 0xcc:	output << AM_immediate_word("LDD");		break;		// LD (LDD immediate)
	case 0xdc:	output << Address_direct("LDD");			break;		// LD (LDD direct)
	case 0xec:	output << Address_indexed("LDD");		break;		// LD (LDD indexed)
	case 0xfc:	output << Address_extended("LDD");		break;		// LD (LDD extended)

	// LDU
	case 0xce:	output << AM_immediate_word("LDU");		break;		// LD (LDU immediate)
	case 0xde:	output << Address_direct("LDU");			break;		// LD (LDU direct)
	case 0xee:	output << Address_indexed("LDU");		break;		// LD (LDU indexed)
	case 0xfe:	output << Address_extended("LDU");		break;		// LD (LDU extended)

	// LDX
	case 0x8e:	output << AM_immediate_word("LDX");		break;		// LD (LDX immediate)
	case 0x9e:	output << Address_direct("LDX");			break;		// LD (LDX direct)
	case 0xae:	output << Address_indexed("LDX");		break;		// LD (LDX indexed)
	case 0xbe:	output << Address_extended("LDX");		break;		// LD (LDX extended)

	// LEA
	case 0x30:	output << Address_indexed("LEAX");		break;		// LEA (LEAX indexed)
	case 0x31:	output << Address_indexed("LEAY");		break;		// LEA (LEAY indexed)
	case 0x32:	output << Address_indexed("LEAS");		break;		// LEA (LEAS indexed)
	case 0x33:	output << Address_indexed("LEAU");		break;		// LEA (LEAU indexed)

	// LSR
	case 0x04:	output << Address_direct("LSR");			break;		// LSR (direct)
	case 0x44:	output << "\tLSRA\t";					break;		// LSR (LSRA inherent)
	case 0x54:	output << "\tLSRB\t";					break;		// LSR (LSRB inherent)
	case 0x64:	output << Address_indexed("LSR");		break;		// LSR (indexed)
	case 0x74:	output << Address_extended("LSR");		break;		// LSR (extended)

	// MUL
	case 0x3d:	output << "\tMUL\t";						break;		// MUL (inherent)

	// NEG
	case 0x00:	output << Address_direct("NEG");			break;		// NEG (direct)
	case 0x40:	output << "\tNEGA\t";					break;		// NEG (NEGA, inherent)
	case 0x50:	output << "\tNEGB\t";					break;		// NEG (NEGB, inherent)
	case 0x60:	output << Address_indexed("NEG");		break;		// NEG (indexed)
	case 0x70:	output << Address_extended("NEG");		break;		// NEG (extended)

	// NOP
	case 0x12:	output << "\tNOP\t";						break;		// NOP (inherent)

	// OR

	// ORA
	case 0x8a:	output << AM_immediate_byte("ORA");		break;		// OR (ORA immediate)
	case 0x9a:	output << Address_direct("ORA");			break;		// OR (ORA direct)
	case 0xaa:	output << Address_indexed("ORA");		break;		// OR (ORA indexed)
	case 0xba:	output << Address_extended("ORA");		break;		// OR (ORA extended)

	// ORB
	case 0xca:	output << AM_immediate_byte("ORB");		break;		// OR (ORB immediate)
	case 0xda:	output << Address_direct("ORB");			break;		// OR (ORB direct)
	case 0xea:	output << Address_indexed("ORB");		break;		// OR (ORB indexed)
	case 0xfa:	output << Address_extended("ORB");		break;		// OR (ORB extended)

	// ORCC
	case 0x1a:	output << AM_immediate_byte("ORCC");		break;		// OR (ORCC immediate)

	// PSH
	case 0x34:	output << pshS();						break;		// PSH (PSHS immediate)
	case 0x36:	output << pshU();						break;		// PSH (PSHU immediate)

	// PUL
	case 0x35:	output << pulS();						break;		// PUL (PULS immediate)
	case 0x37:	output << pulU();						break;		// PUL (PULU immediate)

	// ROL
	case 0x09:	output << Address_direct("ROL");			break;		// ROL (direct)
	case 0x49:	output << "\tROLA\t";					break;		// ROL (ROLA inherent)
	case 0x59:	output << "\tROLB\t";					break;		// ROL (ROLB inherent)
	case 0x69:	output << Address_indexed("ROL");		break;		// ROL (indexed)
	case 0x79:	output << Address_extended("ROL");		break;		// ROL (extended)

	// ROR
	case 0x06:	output << Address_direct("ROR");			break;		// ROR (direct)
	case 0x46:	output << "\tRORA\t";					break;		// ROR (RORA inherent)
	case 0x56:	output << "\tRORB\t";					break;		// ROR (RORB inherent)
	case 0x66:	output << Address_indexed("ROR");		break;		// ROR (indexed)
	case 0x76:	output << Address_extended("ROR");		break;		// ROR (extended)

	// RTI
	case 0x3B:	output << "\tRTI\t";						break;		// RTI (inherent)

	// RTS
	case 0x39:	output << "\tRTS\t";						break;		// RTS (inherent)

	// SBC

	// SBCA
	case 0x82:	output << AM_immediate_byte("SBCA");		break;		// SBC (SBCA immediate)
	case 0x92:	output << Address_direct("SBCA");		break;		// SBC (SBCA direct)
	case 0xa2:	output << Address_indexed("SBCA");		break;		// SBC (SBCA indexed)
	case 0xb2:	output << Address_extended("SBCA");		break;		// SBC (SBCA extended)

	// SBCB
	case 0xc2:	output << AM_immediate_byte("SBCB");		break;		// SBC (SBCB immediate)
	case 0xd2:	output << Address_direct("SBCB");		break;		// SBC (SBCB direct)
	case 0xe2:	output << Address_indexed("SBCB");		break;		// SBC (SBCB indexed)
	case 0xf2:	output << Address_extended("SBCB");		break;		// SBC (SBCB extended)

	// SEX
	case 0x1d:	output << "\tSEX\t";						break;		// SEX (inherent)

	// ST

	// STA
	case 0x97:	output << Address_direct("STA");			break;		// ST (STA direct)
	case 0xa7:	output << Address_indexed("STA");		break;		// ST (STA indexed)
	case 0xb7:	output << Address_extended("STA");		break;		// ST (STA extended)

	// STB
	case 0xd7:	output << Address_direct("STB");			break;		// ST (STB direct)
	case 0xe7:	output << Address_indexed("STB");		break;		// ST (STB indexed)
	case 0xf7:	output << Address_extended("STB");		break;		// ST (STB extended)

	// STD
	case 0xdd:	output << Address_direct("STD");			break;		// ST (STD direct)
	case 0xed:	output << Address_indexed("STD");		break;		// ST (STD indexed)
	case 0xfd:	output << Address_extended("STD");		break;		// ST (STD extended)

	// STU
	case 0xdf:	output << Address_direct("STU");			break;		// ST (STU direct)
	case 0xef:	output << Address_indexed("STU");		break;		// ST (STU indexed)
	case 0xff:	output << Address_extended("STU");		break;		// ST (STU extended)

	// STX
	case 0x9f:	output << Address_direct("STX");			break;		// ST (STX direct)
	case 0xaf:	output << Address_indexed("STX");		break;		// ST (STX indexed)
	case 0xbf:	output << Address_extended("STX");		break;		// ST (STX extended)

	// SUB

	// SUBA
	case 0x80:	output << AM_immediate_byte("SUBA");		break;		// SUB (SUBA immediate)
	case 0x90:	output << Address_direct("SUBA");		break;		// SUB (SUBA direct)
	case 0xa0:	output << Address_indexed("SUBA");		break;		// SUB (SUBA indexed)
	case 0xb0:	output << Address_extended("SUBA");		break;		// SUB (SUBA extended)

	// SUBB
	case 0xc0:	output << AM_immediate_byte("SUBB");		break;		// SUB (SUBB immediate)
	case 0xd0:	output << Address_direct("SUBB");		break;		// SUB (SUBB direct)
	case 0xe0:	output << Address_indexed("SUBB");		break;		// SUB (SUBB indexed)
	case 0xf0:	output << Address_extended("SUBB");		break;		// SUB (SUBB extended)

	// SUBD
	case 0x83:	output << AM_immediate_word("SUBD");		break;		// SUB (SUBD immediate)
	case 0x93:	output << Address_direct("SUBD");		break;		// SUB (SUBD direct)
	case 0xa3:	output << Address_indexed("SUBD");		break;		// SUB (SUBD indexed)
	case 0xb3:	output << Address_extended("SUBD");		break;		// SUB (SUBD extended)

	// SWI
	case 0x3f:	output << "\tSWI\t";						break;		// SWI (inherent)

	// SYNC
	case 0x13:	output << "\tSYNC\t";					break;		// SYNC (inherent)

	// TFR
	case 0x1f:	output << tfr("TFR");					break;		// TFR (immediate)

	// TST
	case 0x0d:	output << Address_direct("TST");			break;		// TST (direct)
	case 0x4d:	output << "\tTSTA\t";					break;		// TST (TSTA inherent)
	case 0x5d:	output << "\tTSTB\t";					break;		// TST (TSTB inherent)
	case 0x6d:	output << Address_indexed("TST");		break;		// TST (indexed)
	case 0x7d:	output << Address_extended("TST");		break;		// TST (extended)

	// Branching

	case 0x16:	output << branchLong("LBRA");			break;		// BRA (LBRA relative)
	case 0x17:	output << branchLong("LBSR");			break;		// BSR (LBSR relative)
	case 0x20:	output << branchShort("BRA");			break;		// BRA (relative)
	case 0x21:	output << branchShort("BRN");			break;		// BRN (relative)
	case 0x22:	output << branchShort("BHI");			break;		// BHI (relative)
	case 0x23:	output << branchShort("BLS");			break;		// BLS (relative)
	case 0x24:	output << branchShort("BCC");			break;		// BCC (relative)
	case 0x25:	output << branchShort("BCS");			break;		// BCS (relative)
	case 0x26:	output << branchShort("BNE");			break;		// BNE (relative)
	case 0x27:	output << branchShort("BEQ");			break;		// BEQ (relative)
	case 0x28: 	output << branchShort("BVC");			break;		// BVC (relative)
	case 0x29: 	output << branchShort("BVS");			break;		// BVS (relative)
	case 0x2a: 	output << branchShort("BPL");			break;		// BPL (relative)
	case 0x2b: 	output << branchShort("BMI");			break;		// BMI (relative)
	case 0x2c:	output << branchShort("BGE");			break;		// BGE (relative)
	case 0x2d:	output << branchShort("BLT");			break;		// BLT (relative)
	case 0x2e:	output << branchShort("BGT");			break;		// BGT (relative)
	case 0x2f:	output << branchShort("BLE");			break;		// BLE (relative)

	case 0x8d:	output << branchShort("BSR");			break;		// BSR (relative)

	default:
		UNREACHABLE;
	}

	return output.str();
}

std::string EightBit::Disassembly::disassemble10() {

	std::ostringstream output;

	const auto opcode = getByte(m_address);
	output << dump_ByteValue(opcode);

	switch (opcode) {

	// CMP

	// CMPD
	case 0x83:	output << AM_immediate_word("CMPD");		break;		// CMP (CMPD, immediate)
	case 0x93:	output << Address_direct("CMPD");		break;		// CMP (CMPD, direct)
	case 0xa3:	output << Address_indexed("CMPD");		break;		// CMP (CMPD, indexed)
	case 0xb3:	output << Address_extended("CMPD");		break;		// CMP (CMPD, extended)

	// CMPY
	case 0x8c:	output << AM_immediate_word("CMPY");		break;		// CMP (CMPY, immediate)
	case 0x9c:	output << Address_direct("CMPY");		break;		// CMP (CMPY, direct)
	case 0xac:	output << Address_indexed("CMPY");		break;		// CMP (CMPY, indexed)
	case 0xbc:	output << Address_extended("CMPY");		break;		// CMP (CMPY, extended)

	// LD

	// LDS
	case 0xce:	output << AM_immediate_word("LDS");		break;		// LD (LDS immediate)
	case 0xde:	output << Address_direct("LDS");			break;		// LD (LDS direct)
	case 0xee:	output << Address_indexed("LDS");		break;		// LD (LDS indexed)
	case 0xfe:	output << Address_extended("LDS");		break;		// LD (LDS extended)

	// LDY
	case 0x8e:	output << AM_immediate_word("LDY");		break;		// LD (LDY immediate)
	case 0x9e:	output << Address_direct("LDY");			break;		// LD (LDY direct)
	case 0xae:	output << Address_indexed("LDY");		break;		// LD (LDY indexed)
	case 0xbe:	output << Address_extended("LDY");		break;		// LD (LDY extended)

	// Branching

	case 0x21:	output << branchLong("LBRN");			break;		// BRN (LBRN relative)
	case 0x22:	output << branchLong("LBHI");			break;		// BHI (LBHI relative)
	case 0x23:	output << branchLong("LBLS");			break;		// BLS (LBLS relative)
	case 0x24:	output << branchLong("LBCC");			break;		// BCC (LBCC relative)
	case 0x25:	output << branchLong("LBCS");			break;		// BCS (LBCS relative)
	case 0x26:	output << branchLong("LBNE");			break;		// BNE (LBNE relative)
	case 0x27:	output << branchLong("LBEQ");			break;		// BEQ (LBEQ relative)
	case 0x28:	output << branchLong("LBVC");			break;		// BVC (LBVC relative)
	case 0x29:	output << branchLong("LBVS");			break;		// BVS (LBVS relative)
	case 0x2a:	output << branchLong("LBPL");			break;		// BPL (LBPL relative)
	case 0x2b: 	output << branchLong("LBMI");			break;		// BMI (LBMI relative)
	case 0x2c:	output << branchLong("LBGE");			break;		// BGE (LBGE relative)
	case 0x2d:	output << branchLong("LBLT");			break;		// BLT (LBLT relative)
	case 0x2e:	output << branchLong("LBGT");			break;		// BGT (LBGT relative)
	case 0x2f:	output << branchLong("LBLE");			break;		// BLE (LBLE relative)

	// STS
	case 0xdf:	output << Address_direct("STS");			break;		// ST (STS direct)
	case 0xef:	output << Address_indexed("STS");		break;		// ST (STS indexed)
	case 0xff:	output << Address_extended("STS");		break;		// ST (STS extended)

	// STY
	case 0x9f:	output << Address_extended("STY");		break;		// ST (STY direct)
	case 0xaf:	output << Address_indexed("STY");		break;		// ST (STY indexed)
	case 0xbf:	output << Address_extended("STY");		break;		// ST (STY extended)

	// SWI
	case 0x3f:	output << "\tSWI2\t";					break;		// SWI (SWI2 inherent)

	default:
		UNREACHABLE;
	}

	m_prefix10 = false;

	return output.str();
}

std::string EightBit::Disassembly::disassemble11() {

	std::ostringstream output;

	const auto opcode = getByte(m_address);
	output << dump_ByteValue(opcode);

	switch (opcode) {

	// CMP

	// CMPU
	case 0x83:	output << AM_immediate_word("CMPU");		break;		// CMP (CMPU, immediate)
	case 0x93:	output << Address_direct("CMPU");		break;		// CMP (CMPU, direct)
	case 0xa3:	output << Address_indexed("CMPU");		break;		// CMP (CMPU, indexed)
	case 0xb3:	output << Address_extended("CMPU");		break;		// CMP (CMPU, extended)

	// CMPS
	case 0x8c:	output << AM_immediate_word("CMPS");		break;		// CMP (CMPS, immediate)
	case 0x9c:	output << Address_direct("CMPS");		break;		// CMP (CMPS, direct)
	case 0xac:	output << Address_indexed("CMPS");		break;		// CMP (CMPS, indexed)
	case 0xbc:	output << Address_extended("CMPS");		break;		// CMP (CMPS, extended)

	// SWI
	case 0x3f:	output << "\tSWI3\t";					break;		// SWI (SWI3 inherent)

	default:
		UNREACHABLE;
	}

	m_prefix11 = false;

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

std::string EightBit::Disassembly::Address_direct(std::string mnemomic) {
	std::ostringstream output;
	const auto address = getByte(++m_address);
	output
		<< dump_ByteValue(address)
		<< "\t" << mnemomic << "\t"
		<< "$" << dump_ByteValue(address);
	return output.str();
}

std::string EightBit::Disassembly::wrapIndirect(std::string what, bool indirect) {
	std::ostringstream output;
	output
		<< (indirect ? "[" : "")
		<< what
		<< (indirect ? "]" : "");
	return output.str();
}

std::string EightBit::Disassembly::Address_indexed(std::string mnemomic) {

	std::ostringstream output;

	const auto type = getByte(++m_address);
	const auto r = RR((type & (Chip::Bit6 | Chip::Bit5)) >> 5);

	uint8_t byte = 0xff;
	uint16_t word = 0xffff;

	output << dump_ByteValue(type);

	if (type & Chip::Bit7) {
		const auto indirect = type & Chip::Bit4;
		switch (type & Chip::Mask4) {
		case 0b0000:	// ,R+
			output
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect("," + r + "+", indirect);
			break;
		case 0b0001:	// ,R++
			output
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect("," + r + "++", indirect);
			break;
		case 0b0010:	// ,-R
			output
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect(",-" + r, indirect);
			break;
		case 0b0011:	// ,--R
			output
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect(",--" + r, indirect);
			break;
		case 0b0100:	// ,R
			output
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect("," + r, indirect);
			break;
		case 0b0101:	// B,R
			output
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect("B," + r, indirect);
			break;
		case 0b0110:	// A,R
			output
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect("A," + r, indirect);
			break;
		case 0b1000:	// n,R (eight-bit)
			byte = getByte(++m_address);
			output
				<< dump_ByteValue(byte)
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect(dump_ByteValue(byte) + "," + r, indirect);
			break;
		case 0b1001:	// n,R (sixteen-bit)
			word = getWord(++m_address);
			output
				<< dump_WordValue(word)
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect(dump_WordValue(word) + "," + r, indirect);
			break;
		case 0b1011:	// D,R
			output
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect("D," + r, indirect);
			break;
		case 0b1100:	// n,PCR (eight-bit)
			byte = getByte(++m_address);
			output
				<< dump_ByteValue(byte)
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect(dump_RelativeValue((int8_t)byte) + ",PCR", indirect);
			break;
		case 0b1101:	// n,PCR (sixteen-bit)
			word = getWord(++m_address);
			output
				<< dump_WordValue(word)
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect(dump_RelativeValue((int16_t)word) + ",PCR", indirect);
			break;
		case 0b1111:	// [n]
			assert(indirect);
			word = getWord(++m_address);
			output
				<< dump_WordValue(word)
				<< "\t" << mnemomic << "\t"
				<< wrapIndirect(dump_WordValue(word), indirect);
			break;
		default:
			assert(false);
			break;
		}
	} else {
		// EA = ,R + 5-bit offset
		output
			<< "\t" << mnemomic << "\t"
			<< (int)Processor::signExtend(5, type & Chip::Mask5) << "," << r;
	}

	return output.str();
}

std::string EightBit::Disassembly::Address_extended(std::string mnemomic) {
	std::ostringstream output;
	const auto word = getWord(++m_address);
	output
		<< dump_WordValue(word)
		<< "\t" << mnemomic << "\t"
		<< "$" << dump_WordValue(word);
	return output.str();
}

std::string EightBit::Disassembly::Address_relative_byte(std::string mnemomic) {
	std::ostringstream output;
	const auto byte = getByte(++m_address);
	output
		<< dump_ByteValue(byte)
		<< "\t" << mnemomic << "\t"
		<< "$" << dump_WordValue(++m_address + (int8_t)byte);
	return output.str();
}

std::string EightBit::Disassembly::Address_relative_word(std::string mnemomic) {
	std::ostringstream output;
	const auto word = getWord(++m_address);
	output
		<< dump_WordValue(word)
		<< "\t" << mnemomic << "\t"
		<< "$" << dump_WordValue(++m_address + (int16_t)word);
	return output.str();
}

////

std::string EightBit::Disassembly::AM_immediate_byte(std::string mnemomic) {
	std::ostringstream output;
	const auto byte = getByte(++m_address);
	output
		<< dump_ByteValue(byte)
		<< "\t" << mnemomic << "\t"
		<< "#$" << dump_ByteValue(byte);
	return output.str();
}

std::string EightBit::Disassembly::AM_immediate_word(std::string mnemomic) {
	std::ostringstream output;
	const auto word = getWord(++m_address);
	output
		<< dump_WordValue(word)
		<< "\t" << mnemomic << "\t"
		<< "#$" << dump_WordValue(word);
	return output.str();
}

////

std::string EightBit::Disassembly::branchShort(std::string mnemomic) {
	return Address_relative_byte(mnemomic);
}

std::string EightBit::Disassembly::branchLong(std::string mnemomic) {
	return Address_relative_word(mnemomic);
}

////

std::string EightBit::Disassembly::referenceTransfer8(int specifier) {
	switch (specifier) {
	case 0b1000:
		return "A";
	case 0b1001:
		return "B";
	case 0b1010:
		return "CC";
	case 0b1011:
		return "DP";
	default:
		UNREACHABLE;
	}
}

std::string EightBit::Disassembly::referenceTransfer16(int specifier) {
	switch (specifier) {
	case 0b0000:
		return "D";
	case 0b0001:
		return "X";
	case 0b0010:
		return "Y";
	case 0b0011:
		return "U";
	case 0b0100:
		return "S";
	case 0b0101:
		return "PC";
	default:
		UNREACHABLE;
	}
}

std::string EightBit::Disassembly::tfr(std::string mnemomic) {

	std::ostringstream output;

	const auto data = getByte(++m_address);
	const auto reg1 = Chip::highNibble(data);
	const auto reg2 = Chip::lowNibble(data);

	output
		<< dump_ByteValue(data)
		<< "\t" << mnemomic << "\t";

	const bool type8 = !!(reg1 & Chip::Bit3);	// 8 bit?
	if (type8)
		output << referenceTransfer8(reg1) << "," << referenceTransfer8(reg2);
	else
		output << referenceTransfer16(reg1) << "," << referenceTransfer16(reg2);

	return output.str();
}

////

uint8_t EightBit::Disassembly::getByte(uint16_t address) {
	return BUS().peek(address);
}

uint16_t EightBit::Disassembly::getWord(uint16_t address) {
	return CPU().peekWord(address).word;
}

//

std::string EightBit::Disassembly::pulS() {
	return pulX("PULS", "U");
}

std::string EightBit::Disassembly::pulU() {
	return pulX("PULU", "S");
}

std::string EightBit::Disassembly::pshS() {
	return pshX("PSHS", "U");
}

std::string EightBit::Disassembly::pshU() {
	return pshX("PSHU", "S");
}

std::string EightBit::Disassembly::pulX(std::string mnemomic, std::string upon) {
	const auto data = getByte(++m_address);
	std::ostringstream output;
	output
		<< dump_ByteValue(data)
		<< "\t" << mnemomic << "\t";
	std::vector<std::string> registers;
	if (data & Chip::Bit0)
		registers.push_back("CC");
	if (data & Chip::Bit1)
		registers.push_back("A");
	if (data & Chip::Bit2)
		registers.push_back("B");
	if (data & Chip::Bit3)
		registers.push_back("DP");
	if (data & Chip::Bit4)
		registers.push_back("X");
	if (data & Chip::Bit5)
		registers.push_back("Y");
	if (data & Chip::Bit6)
		registers.push_back(upon);
	if (data & Chip::Bit7)
		registers.push_back("PC");
	for (int i = 0; i < registers.size(); ++i) {
		const bool last = i == registers.size() - 1;
		output << registers[i];
		if (!last)
			output << ",";
	}
	return output.str();
}

std::string EightBit::Disassembly::pshX(std::string mnemomic, std::string upon) {
	const auto data = getByte(++m_address);
	std::ostringstream output;
	output
		<< dump_ByteValue(data)
		<< "\t" << mnemomic << "\t";
	std::vector<std::string> registers;
	if (data & Chip::Bit7)
		registers.push_back("PC");
	if (data & Chip::Bit6)
		registers.push_back(upon);
	if (data & Chip::Bit5)
		registers.push_back("Y");
	if (data & Chip::Bit4)
		registers.push_back("X");
	if (data & Chip::Bit3)
		registers.push_back("DP");
	if (data & Chip::Bit2)
		registers.push_back("B");
	if (data & Chip::Bit1)
		registers.push_back("A");
	if (data & Chip::Bit0)
		registers.push_back("CC");
	for (int i = 0; i < registers.size(); ++i) {
		const bool last = i == registers.size() - 1;
		output << registers[i];
		if (!last)
			output << ",";
	}
	return output.str();
}
