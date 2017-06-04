#include "stdafx.h"
#include "Disassembler.h"

#include <sstream>
#include <iomanip>
#include <bitset>

#include "Memory.h"
#include "LR35902.h"
#include "StatusFlags.h"

Disassembler::Disassembler() {
	// Disable exceptions where too many format arguments are available
	m_formatter.exceptions(boost::io::all_error_bits ^ boost::io::too_many_args_bit);
}

std::string Disassembler::state(LR35902& cpu) {

	auto pc = cpu.getProgramCounter();
	auto sp = cpu.getStackPointer();

	auto a = cpu.A();
	auto f = cpu.F();

	auto b = cpu.B();
	auto c = cpu.C();

	auto d = cpu.D();
	auto e = cpu.E();

	auto h = cpu.H();
	auto l = cpu.L();

	std::ostringstream output;

	output
		<< "PC=" << hex(pc)
		<< " "
		<< "SP=" << hex(sp)
		<< " " << "A=" << hex(a) << " " << "F=" << flags(f)
		<< " " << "B=" << hex(b) << " " << "C=" << hex(c)
		<< " " << "D=" << hex(d) << " " << "E=" << hex(e)
		<< " " << "H=" << hex(h) << " " << "L=" << hex(l);

	return output.str();
}

std::string Disassembler::RP(int rp) const {
	switch (rp) {
	case 0:
		return "BC";
	case 1:
		return "DE";
	case 2:
		return "HL";
	case 3:
		return "SP";
	}
	throw std::logic_error("Unhandled register pair");
}

std::string Disassembler::RP2(int rp) const {
	switch (rp) {
	case 0:
		return "BC";
	case 1:
		return "DE";
	case 2:
		return "HL";
	case 3:
		return "AF";
	}
	throw std::logic_error("Unhandled register pair");
}

std::string Disassembler::R(int r) const {
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
		return "H";
	case 5:
		return "L";
	case 6:
		return "(HL)";
	case 7:
		return "A";
	}
	throw std::logic_error("Unhandled register");
}

std::string Disassembler::cc(int flag) {
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

std::string Disassembler::alu(int which) {
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

std::string Disassembler::disassemble(LR35902& cpu) {
	m_prefixCB = false;
	std::ostringstream output;
	disassemble(output, cpu, cpu.getProgramCounter());
	return output.str();
}

void Disassembler::disassemble(std::ostringstream& output, LR35902& cpu, uint16_t pc) {

	auto& memory = cpu.getMemory();
	auto opcode = memory.peek(pc);

	// hex opcode
	output << hex(opcode);

	auto x = (opcode & 0b11000000) >> 6;
	auto y = (opcode & 0b111000) >> 3;
	auto z = (opcode & 0b111);

	auto p = (y & 0b110) >> 1;
	auto q = (y & 1);

	auto immediate = memory.peek(pc + 1);
	auto absolute = cpu.getWord(pc + 1);
	auto displacement = (int8_t)immediate;
	auto relative = pc + displacement + 2;
	auto indexedImmediate = memory.peek(pc + 1);

	auto dumpCount = 0;

	std::string specification = "";

	if (m_prefixCB)
		disassembleCB(
			output, cpu, pc,
			specification, dumpCount,
			x, y, z, p, q);
	else
		disassembleOther(
			output, cpu, pc,
			specification, dumpCount,
			x, y, z, p, q);

	for (int i = 0; i < dumpCount; ++i)
		output << hex(memory.peek(pc + i + 1));

	output << '\t';
	m_formatter.parse(specification);
	output << m_formatter % (int)immediate % (int)absolute % relative % (int)displacement % indexedImmediate;
}

void Disassembler::disassembleCB(
	std::ostringstream& output,
	LR35902& cpu,
	uint16_t pc,
	std::string& specification,
	int& dumpCount,
	int x, int y, int z,
	int p, int q) {

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

void Disassembler::disassembleOther(
	std::ostringstream& output,
	LR35902& cpu,
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
			case 1:	// GB: LD (nn),SP
				specification = "LD (%2$04XH),SP";
				dumpCount += 2;
				break;
			case 2:	// GB: STOP
				specification = "STOP";
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
				case 2:	// GB: LDI (HL),A
					specification = "LDI (HL),A";
					break;
				case 3:	// GB: LDD (HL),A
					specification = "LDD (HL),A";
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
				case 2:	// GB: LDI A,(HL)
					specification = "LDI A,(HL)";
					break;
				case 3:	// GB: LDD A,(HL)
					specification = "LDD A,(HL)";
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
			specification = "LD " + R(y) + ",%1$02XH";
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
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				specification = "RET " + cc(y);
				break;
			case 4:
				specification = "LD (FF00H+%1$02XH),A";
				dumpCount++;
				break;
			case 5:
				specification = "ADD SP,%3$04XH";
				dumpCount++;
				break;
			case 6:
				specification = "LD A,(FF00H+%1$02XH)";
				dumpCount++;
				break;
			case 7:
				specification = "LD HL,SP+%3$04XH";
				dumpCount++;
				break;
			}
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
				case 1:	// GB: RETI
					specification = "RETI";
					break;
				case 2:	// JP (HL)
					specification = "JP (HL)";
					break;
				case 3:	// LD SP,HL
					specification = "LD SP,Hl";
					break;
				}
			}
			break;
		case 2:	// Conditional jump
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				specification = "JP " + cc(y) + ",%2$04XH";
				dumpCount += 2;
				break;
			case 4:
				specification = "LD (FF00H+C),A";
				break;
			case 5:
				specification = "LD (%2$04XH),A";
				dumpCount += 2;
				break;
			case 6:
				specification = "LD A,(FF00H+C)";
				break;
			case 7:
				specification = "LD A,(%2$04XH)";
				dumpCount += 2;
				break;
			}
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
				}
			}
			break;
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			specification = alu(y) + " A,%1$02XH";
			dumpCount++;
			break;
		case 7:	// Restart: RST y * 8
			specification = "RST " + hex((uint8_t)(y * 8)) + "H";
			break;
		}
		break;
	}
}

std::string Disassembler::flag(uint8_t value, int flag, const std::string& represents) {
	std::ostringstream output;
	output << (value & flag ? represents : "-");
	return output.str();
}

std::string Disassembler::flags(uint8_t value) {
	std::ostringstream output;
	output
		<< flag(value, LR35902::ZF, "Z")
		<< flag(value, LR35902::NF, "N")
		<< flag(value, LR35902::HC, "H")
		<< flag(value, LR35902::CF, "C")
		<< flag(value, Processor::Bit3, "+")
		<< flag(value, Processor::Bit2, "+")
		<< flag(value, Processor::Bit1, "+")
		<< flag(value, Processor::Bit0, "+");
		return output.str();
}

std::string Disassembler::hex(uint8_t value) {
	std::ostringstream output;
	output << std::hex << std::setw(2) << std::setfill('0') << (int)value;
	return output.str();
}

std::string Disassembler::hex(uint16_t value) {
	std::ostringstream output;
	output << std::hex << std::setw(4) << std::setfill('0') << (int)value;
	return output.str();
}

std::string Disassembler::binary(uint8_t value) {
	std::ostringstream output;
	output << std::bitset<8>(value);
	return output.str();
}

std::string Disassembler::decimal(uint8_t value) {
	std::ostringstream output;
	output << (int)value;
	return output.str();
}

std::string Disassembler::invalid(uint8_t value) {
	std::ostringstream output;
	output << "Invalid instruction: " << hex(value) << "(" << binary(value) << ")";
	return output.str();
}