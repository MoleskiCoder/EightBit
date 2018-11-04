#include "stdafx.h"
#include "Disassembler.h"

#include <sstream>
#include <iomanip>
#include <bitset>
#include <iostream>

#include "Z80.h"

EightBit::Disassembler::Disassembler(Bus& bus) noexcept
: m_bus(bus) {
	// Disable exceptions where too many format arguments are available
	m_formatter.exceptions(boost::io::all_error_bits ^ boost::io::too_many_args_bit);
}

std::string EightBit::Disassembler::state(Z80& cpu) {

	auto pc = cpu.PC();
	auto sp = cpu.SP();

	auto a = cpu.A();
	auto f = cpu.F();

	auto b = cpu.B();
	auto c = cpu.C();

	auto d = cpu.D();
	auto e = cpu.E();

	auto h = cpu.H();
	auto l = cpu.L();

	auto i = cpu.IV();
	uint8_t r = cpu.REFRESH();

	auto im = cpu.IM();

	std::ostringstream output;

	output
		<< "PC=" << pc
		<< " "
		<< "SP=" << sp
		<< " " << "A=" << hex(a) << " " << "F=" << flags(f)
		<< " " << "B=" << hex(b) << " " << "C=" << hex(c)
		<< " " << "D=" << hex(d) << " " << "E=" << hex(e)
		<< " " << "H=" << hex(h) << " " << "L=" << hex(l)
		<< " " << "I=" << hex(i) << " " << "R=" << hex(r)
		<< " " << "IM=" << im;

	return output.str();
}

std::string EightBit::Disassembler::RP(int rp) const {
	switch (rp) {
	case 0:
		return "BC";
	case 1:
		return "DE";
	case 2:
		if (m_prefixDD)
			return "IX";
		if (m_prefixFD)
			return "IY";
		return "HL";
	case 3:
		return "SP";
	}
	throw std::logic_error("Unhandled register pair");
}

std::string EightBit::Disassembler::RP2(int rp) const {
	switch (rp) {
	case 0:
		return "BC";
	case 1:
		return "DE";
	case 2:
		if (m_prefixDD)
			return "IX";
		if (m_prefixFD)
			return "IY";
		return "HL";
	case 3:
		return "AF";
	}
	throw std::logic_error("Unhandled register pair");
}

std::string EightBit::Disassembler::R(int r) const {
	switch (r) {
	case 0:
		return "B";
	case 1:
		return "C";
	case 2:
		return "D";
	case 3:
		return "E";
	case 4:
		if (m_prefixDD)
			return "IXH";
		if (m_prefixFD)
			return "IYH";
		return "H";
	case 5:
		if (m_prefixDD)
			return "IXL";
		if (m_prefixFD)
			return "IYL";
		return "L";
	case 6:
		if (m_prefixDD || m_prefixFD) {
			if (m_prefixDD)
				return "IX+%4%";
			if (m_prefixFD)
				return "IY+%4%";
		}
		else {
			return "(HL)";
		}
	case 7:
		return "A";
	}
	throw std::logic_error("Unhandled register");
}

std::string EightBit::Disassembler::cc(int flag) {
	switch (flag) {
	case 0:
		return "NZ";
	case 1:
		return "Z";
	case 2:
		return "NC";
	case 3:
		return "C";
	case 4:
		return "PO";
	case 5:
		return "PE";
	case 6:
		return "P";
	case 7:
		return "M";
	}
	throw std::logic_error("Unhandled condition");
}

std::string EightBit::Disassembler::alu(int which) {
	switch (which) {
	case 0:	// ADD A,n
		return "ADD";
	case 1:	// ADC
		return "ADC";
	case 2:	// SUB n
		return "SUB";
	case 3:	// SBC A,n
		return "SBC";
	case 4:	// AND n
		return "AND";
	case 5:	// XOR n
		return "XOR";
	case 6:	// OR n
		return "OR";
	case 7:	// CP n
		return "CP";
	}
	throw std::logic_error("Unhandled alu operation");
}

std::string EightBit::Disassembler::disassemble(Z80& cpu) {
	m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
	std::ostringstream output;
	disassemble(output, cpu, cpu.PC().word);
	return output.str();
}

void EightBit::Disassembler::disassemble(std::ostringstream& output, Z80& cpu, uint16_t pc) {

	auto opcode = BUS().peek(pc);

	const auto& decoded = cpu.getDecodedOpcode(opcode);

	auto x = decoded.x;
	auto y = decoded.y;
	auto z = decoded.z;

	auto p = decoded.p;
	auto q = decoded.q;

	auto immediate = BUS().peek(pc + 1);
	auto absolute = cpu.peekWord(pc + 1).word;
	auto displacement = (int8_t)immediate;
	auto relative = pc + displacement + 2;
	auto indexedImmediate = BUS().peek(pc + 1);

	auto dumpCount = 0;

	output << hex(opcode);

	std::string specification = "";

	if (m_prefixCB)
		disassembleCB(
			output, cpu, pc,
			specification, dumpCount,
			x, y, z, p, q);
	else if (m_prefixED)
		disassembleED(
			output, cpu, pc,
			specification, dumpCount,
			x, y, z, p, q);
	else
		disassembleOther(
			output, cpu, pc,
			specification, dumpCount,
			x, y, z, p, q);

	for (int i = 0; i < dumpCount; ++i)
		output << hex(BUS().peek(pc + i + 1));

	auto outputFormatSpecification = !m_prefixDD;
	if (m_prefixDD) {
		if (opcode != 0xdd) {
			outputFormatSpecification = true;
		}
	}

	if (outputFormatSpecification) {
		output << '\t';
		m_formatter.parse(specification);
		output << m_formatter % (int)immediate % (int)absolute % relative % (int)displacement % indexedImmediate;
	}
}

void EightBit::Disassembler::disassembleCB(
	std::ostringstream& output,
	const Z80& cpu,
	uint16_t pc,
	std::string& specification,
	int& dumpCount,
	int x, int y, int z,
	int p, int q) const {

	switch (x) {
	case 0:	// rot[y] r[z]
		switch (y) {
		case 0:
			specification = "RLC " + R(z);
			break;
		case 1:
			specification = "RRC " + R(z);
			break;
		case 2:
			specification = "RL " + R(z);
			break;
		case 3:
			specification = "RR " + R(z);
			break;
		case 4:
			specification = "SLA " + R(z);
			break;
		case 5:
			specification = "SRA " + R(z);
			break;
		case 6:
			specification = "SWAP " + R(z);
			break;
		case 7:
			specification = "SRL " + R(z);
			break;
		}
		break;
	case 1: // BIT y, r[z]
		specification = "BIT " + decimal(y) + "," + R(z);
		break;
	case 2:	// RES y, r[z]
		specification = "RES " + decimal(y) + "," + R(z);
		break;
	case 3:	// SET y, r[z]
		specification = "SET " + decimal(y) + "," + R(z);
		break;
	}
}

void EightBit::Disassembler::disassembleED(
		std::ostringstream& output,
		const Z80& cpu,
		uint16_t pc,
		std::string& specification,
		int& dumpCount,
		int x, int y, int z,
		int p, int q) const {
	switch (x) {
	case 0:
	case 3:
		specification = "NONI NOP";
		break;
	case 1:
		switch (z) {
		case 2:
			switch (q) {
			case 0:	// SBC HL,rp
				specification = "SBC HL," + RP(p);
				break;
			case 1:	// ADC HL,rp
				specification = "ADC HL," + RP(p);
				break;
			}
		case 3:
			switch (q) {
			case 0:	// LD (nn),rp
				specification = "LD (%2$04XH)," + RP(p);
				break;
			case 1:	// LD rp,(nn)
				specification = "LD " + RP(p) + ",(%2$04XH)";
				break;
			}
			dumpCount += 2;
			break;
		case 7:
			switch (y) {
			case 0:
				specification = "LD I,A";
				break;
			case 1:
				specification = "LD R,A";
				break;
			case 2:
				specification = "LD A,I";
				break;
			case 3:
				specification = "LD A,R";
				break;
			case 4:
				specification = "RRD";
				break;
			case 5:
				specification = "RLD";
				break;
			case 6:
			case 7:
				specification = "NOP";
				break;
			}
			break;
		}
		break;
	case 2:
		switch (z) {
		case 0:	// LD
			switch (y) {
			case 4:	// LDI
				specification = "LDI";
				break;
			case 5:	// LDD
				specification = "LDD";
				break;
			case 6:	// LDIR
				specification = "LDIR";
				break;
			case 7:	// LDDR
				specification = "LDDR";
				break;
			}
			break;
		case 1:	// CP
			switch (y) {
			case 4:	// CPI
				specification = "CPI";
				break;
			case 5:	// CPD
				specification = "CPD";
				break;
			case 6:	// CPIR
				specification = "CPIR";
				break;
			case 7:	// CPDR
				specification = "CPDR";
				break;
			}
			break;
		case 2:	// IN
			switch (y) {
			case 4:	// INI
				specification = "INI";
				break;
			case 5:	// IND
				specification = "IND";
				break;
			case 6:	// INIR
				specification = "INIR";
				break;
			case 7:	// INDR
				specification = "INDR";
				break;
			}
			break;
		case 3:	// OUT
			switch (y) {
			case 4:	// OUTI
				specification = "OUTI";
				break;
			case 5:	// OUTD
				specification = "OUTD";
				break;
			case 6:	// OTIR
				specification = "OTIR";
				break;
			case 7:	// OTDR
				specification = "OTDR";
				break;
			}
			break;
		}
		break;
	}
}

void EightBit::Disassembler::disassembleOther(
	std::ostringstream& output,
	Z80& cpu,
	uint16_t pc,
	std::string& specification,
	int& dumpCount,
	int x, int y, int z,
	int p, int q) {

	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				specification = "NOP";
				break;
			case 1:	// EX AF AF'
				specification = "EX AF AF'";
				break;
			case 2:	// DJNZ d
				specification = "DJNZ %3$04XH";
				dumpCount += 2;
				break;
			case 3:	// JR d
				specification = "JR %3$04XH";
				dumpCount++;
				break;
			default:	// JR cc,d
				specification = "JR " + cc(y - 4) + ",%3$04XH";
				dumpCount++;
				break;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0:	// LD rp,nn
				specification = "LD " + RP(p) + ",%2$04XH";
				dumpCount += 2;
				break;
			case 1:	// ADD HL,rp
				specification = "ADD HL," + RP(p);
				break;
			}
			break;
		case 2:	// Indirect loading
			switch (q) {
			case 0:
				switch (p) {
				case 0:	// LD (BC),A
					specification = "LD (BC),A";
					break;
				case 1:	// LD (DE),A
					specification = "LD (DE),A";
					break;
				case 2:	// LD (nn),HL
					specification = "LD (%2$04XH),HL";
					dumpCount += 2;
					break;
				case 3:	// LD (nn),A
					specification = "LD (%2$04XH),A";
					dumpCount += 2;
					break;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					specification = "LD A,(BC)";
					break;
				case 1:	// LD A,(DE)
					specification = "LD A,(DE)";
					break;
				case 2:	// LD HL,(nn)
					specification = "LD HL,(%2$04XH)";
					dumpCount += 2;
					break;
				case 3:	// LD A,(nn)
					specification = "LD A,(%2$04XH)";
					dumpCount += 2;
					break;
				}
				break;
			}
			break;
		case 3:	// 16-bit INC/DEC
			switch (q) {
			case 0:	// INC rp
				specification = "INC " + RP(p);
				break;
			case 1:	// DEC rp
				specification = "DEC " + RP(p);
				break;
			}
			break;
		case 4:	// 8-bit INC
			specification = "INC " + R(y);
			break;
		case 5:	// 8-bit DEC
			specification = "DEC " + R(y);
			break;
		case 6:	// 8-bit load immediate
			specification = "LD " + R(y);
			if (y == 6 && (m_prefixDD || m_prefixFD)) {
				specification += ",%5$02XH";
				dumpCount++;
			} else {
				specification += ",%1$02XH";
			}
			dumpCount++;
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				specification = "RLCA";
				break;
			case 1:
				specification = "RRCA";
				break;
			case 2:
				specification = "RLA";
				break;
			case 3:
				specification = "RRA";
				break;
			case 4:
				specification = "DAA";
				break;
			case 5:
				specification = "CPL";
				break;
			case 6:
				specification = "SCF";
				break;
			case 7:
				specification = "CCF";
				break;
			}
			break;
		}
		break;
	case 1:	// 8-bit loading
		if (z == 6 && y == 6) { 	// Exception (replaces LD (HL), (HL))
			specification = "HALT";
		} else {
			specification = "LD " + R(y) + "," + R(z);
		}
		break;
	case 2:	// Operate on accumulator and register/memory location
		specification = alu(y) + " A," + R(z);
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			specification = "RET " + cc(y);
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				specification = "POP " + RP2(p);
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					specification = "RET";
					break;
				case 1:	// EXX
					specification = "EXX";
					break;
				case 2:	// JP (HL)
					specification = "JP (HL)";
					break;
				case 3:	// LD SP,HL
					specification = "LD SP,HL";
					break;
				}
			}
			break;
		case 2:	// Conditional jump
			specification = "JP " + cc(y) + ",%2$04XH";
			dumpCount += 2;
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				specification = "JP %2$04XH";
				dumpCount += 2;
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				disassemble(output, cpu, pc + 1);
				break;
			case 2:	// OUT (n),A
				specification = "OUT (%1$02XH),A";
				dumpCount++;
				break;
			case 3:	// IN A,(n)
				specification = "IN A,(%1$02XH)";
				dumpCount++;
				break;
			case 4:	// EX (SP),HL
				specification = "EX (SP),HL";
				break;
			case 5:	// EX DE,HL
				specification = "EX DE,HL";
				break;
			case 6:	// DI
				specification = "DI";
				break;
			case 7:	// EI
				specification = "EI";
				break;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			specification = "CALL " + cc(y) + ",%2$04XH";
			dumpCount += 2;
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				specification = "PUSH " + RP2(p);
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					specification = "CALL %2$04XH";
					dumpCount += 2;
					break;
				case 1:	// DD prefix
					m_prefixDD = true;
					disassemble(output, cpu, pc + 1);
					break;
				case 2:	// ED prefix
					m_prefixED = true;
					disassemble(output, cpu, pc + 1);
					break;
				case 3:	// FD prefix
					m_prefixFD = true;
					disassemble(output, cpu, pc + 1);
					break;
				}
			}
			break;
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			specification = alu(y) + " A,%1$02XH";
			dumpCount++;
			break;
		case 7:	// Restart: RST y * 8
			specification = "RST " + hex((uint8_t)(y * 8));
			break;
		}
		break;
	}
}

std::string EightBit::Disassembler::flag(uint8_t value, int flag, const std::string& represents) {
	std::ostringstream output;
	output << (value & flag ? represents : "-");
	return output.str();
}

std::string EightBit::Disassembler::flags(uint8_t value) {
	std::ostringstream output;
	output
		<< flag(value, Z80::SF, "S")
		<< flag(value, Z80::ZF, "Z")
		<< flag(value, Z80::YF, "Y")
		<< flag(value, Z80::HC, "H")
		<< flag(value, Z80::XF, "X")
		<< flag(value, Z80::PF, "P")
		<< flag(value, Z80::NF, "N")
		<< flag(value, Z80::CF, "C");
	return output.str();
}

std::string EightBit::Disassembler::hex(uint8_t value) {
	std::ostringstream output;
	output << std::hex << std::setw(2) << std::setfill('0') << (int)value;
	return output.str();
}

std::string EightBit::Disassembler::hex(uint16_t value) {
	std::ostringstream output;
	output << std::hex << std::setw(4) << std::setfill('0') << (int)value;
	return output.str();
}

std::string EightBit::Disassembler::binary(uint8_t value) {
	std::ostringstream output;
	output << std::bitset<8>(value);
	return output.str();
}

std::string EightBit::Disassembler::decimal(uint8_t value) {
	std::ostringstream output;
	output << (int)value;
	return output.str();
}

std::string EightBit::Disassembler::invalid(uint8_t value) {
	std::ostringstream output;
	output << "Invalid instruction: " << hex(value) << "(" << binary(value) << ")";
	return output.str();
}