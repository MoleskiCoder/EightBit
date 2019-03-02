#include "stdafx.h"
#include "Disassembler.h"
#include "Intel8080.h"

#include <sstream>
#include <iomanip>
#include <bitset>

EightBit::Disassembler::Disassembler(Bus& bus) noexcept
: m_bus(bus) {
	// Disable exceptions where too many format arguments are available
	m_formatter.exceptions(boost::io::all_error_bits ^ boost::io::too_many_args_bit);
}

std::string EightBit::Disassembler::state(Intel8080& cpu) {

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

	std::string output;
	output.reserve(80);
	output =
		"PC=" + hex(pc.word)
		+ " SP=" + hex(sp.word)
		+ " A=" + hex(a) + " F=" + flags(f)
		+ " B=" + hex(b) + " C=" + hex(c)
		+ " D=" + hex(d) + " E=" + hex(e)
		+ " H=" + hex(h) + " L=" + hex(l);
	return output;
}

std::string EightBit::Disassembler::RP(int rp) {
	switch (rp) {
	case 0:
		return "B";
	case 1:
		return "D";
	case 2:
		return "H";
	case 3:
		return "SP";
	}
	throw std::logic_error("Unhandled register pair");
}

std::string EightBit::Disassembler::RP2(int rp) {
	switch (rp) {
	case 0:
		return "B";
	case 1:
		return "D";
	case 2:
		return "H";
	case 3:
		return "PSW";
	}
	throw std::logic_error("Unhandled register pair");
}

std::string EightBit::Disassembler::R(int r) {
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
		return "M";
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
		return "SBB";
	case 4:	// AND n
		return "ANA";
	case 5:	// XOR n
		return "XRA";
	case 6:	// OR n
		return "ORA";
	case 7:	// CP n
		return "CMP";
	}
	throw std::logic_error("Unhandled alu operation");
}

std::string EightBit::Disassembler::alu2(int which) {
	switch (which) {
	case 0:	// ADD A,n
		return "ADI";
	case 1:	// ADC
		return "ACI";
	case 2:	// SUB n
		return "SUI";
	case 3:	// SBC A,n
		return "SBI";
	case 4:	// AND n
		return "ANI";
	case 5:	// XOR n
		return "XRI";
	case 6:	// OR n
		return "ORI";
	case 7:	// CP n
		return "CPI";
	}
	throw std::logic_error("Unhandled alu operation");
}

std::string EightBit::Disassembler::disassemble(Intel8080& cpu) {
	return disassemble(cpu, cpu.PC().word);
}

std::string EightBit::Disassembler::disassemble(Intel8080& cpu, uint16_t pc) {

	std::ostringstream output;

	auto opcode = BUS().peek(pc);

	output << hex(opcode);

	auto x = (opcode & 0b11000000) >> 6;
	auto y = (opcode & 0b111000) >> 3;
	auto z = (opcode & 0b111);

	auto p = (y & 0b110) >> 1;
	auto q = (y & 1);

	auto immediate = BUS().peek(pc + 1);
	auto absolute = cpu.peekWord(pc + 1).word;
	auto displacement = (int8_t)immediate;
	auto relative = pc + displacement + 2;
	auto indexedImmediate = BUS().peek(pc + 1);

	auto dumpCount = 0;

	const std::string specification = disassemble(dumpCount, x, y, z, p, q);

	for (int i = 0; i < dumpCount; ++i)
		output << hex(BUS().peek(pc + i + 1));

	m_formatter.parse(specification);
	output << '\t' << m_formatter % (int)immediate % (int)absolute % relative % (int)displacement % indexedImmediate;

	return output.str();
}

std::string EightBit::Disassembler::disassemble(
	int& dumpCount,
	int x, int y, int z,
	int p, int q) {

	std::string specification;
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				specification = "NOP";
				break;
			case 1:	// EX AF AF'
				break;
			case 2:	// DJNZ d
				break;
			case 3:	// JR d
				break;
			default:	// JR cc,d
				break;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0:	// LD rp,nn
				specification = "LXI " + RP(p) + ",%2$04XH";
				dumpCount += 2;
				break;
			case 1:	// ADD HL,rp
				specification = "DAD " + RP(p);
				break;
			}
			break;
		case 2:	// Indirect loading
			switch (q) {
			case 0:
				switch (p) {
				case 0:	// LD (BC),A
					specification = "STAX B";
					break;
				case 1:	// LD (DE),A
					specification = "STAX D";
					break;
				case 2:	// LD (nn),HL
					specification = "SHLD %2$04XH";
					dumpCount += 2;
					break;
				case 3:	// LD (nn),A
					specification = "STA %2$04XH";
					dumpCount += 2;
					break;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					specification = "LDAX B";
					break;
				case 1:	// LD A,(DE)
					specification = "LDAX D";
					break;
				case 2:	// LD HL,(nn)
					specification = "LHLD %2$04XH";
					dumpCount += 2;
					break;
				case 3:	// LD A,(nn)
					specification = "LDA %2$04XH";
					dumpCount += 2;
					break;
				}
				break;
			}
			break;
		case 3:	// 16-bit INC/DEC
			switch (q) {
			case 0:	// INC rp
				specification = "INX " + RP(p);
				break;
			case 1:	// DEC rp
				specification = "DCX " + RP(p);
				break;
			}
			break;
		case 4:	// 8-bit INC
			specification = "INR " + R(y);
			break;
		case 5:	// 8-bit DEC
			specification = "DCR " + R(y);
			break;
		case 6:	// 8-bit load immediate
			specification = "MVI " + R(y) + ",%1$02XH";
			dumpCount++;
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				specification = "RLC";
				break;
			case 1:
				specification = "RRC";
				break;
			case 2:
				specification = "RAL";
				break;
			case 3:
				specification = "RAR";
				break;
			case 4:
				specification = "DAA";
				break;
			case 5:
				specification = "CMA";
				break;
			case 6:
				specification = "STC";
				break;
			case 7:
				specification = "CMC";
				break;
			}
			break;
		}
		break;
	case 1:	// 8-bit loading
		if (z == 6 && y == 6) { 	// Exception (replaces LD (HL), (HL))
			specification = "HLT";
		} else {
			specification = "MOV " + R(y) + "," + R(z);
		}
		break;
	case 2:	// Operate on accumulator and register/memory location
		specification = alu(y) + " " + R(z);
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			specification = "R" + cc(y);
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
					break;
				case 2:	// JP HL
					specification = "PCHL";
					break;
				case 3:	// LD SP,HL
					specification = "SPHL";
					break;
				}
			}
			break;
		case 2:	// Conditional jump
			specification = "J" + cc(y) + " %2$04XH";
			dumpCount += 2;
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				specification = "JMP %2$04XH";
				dumpCount += 2;
				break;
			case 1:	// CB prefix
				break;
			case 2:	// OUT (n),A
				specification = "OUT %1$02XH";
				dumpCount++;
				break;
			case 3:	// IN A,(n)
				specification = "IN %1$02XH";
				dumpCount++;
				break;
			case 4:	// EX (SP),HL
				specification = "XHTL";
				break;
			case 5:	// EX DE,HL
				specification = "XCHG";
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
			specification = "C" + cc(y) + " %2$04XH";
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
					break;
				case 2:	// ED prefix
					break;
				case 3:	// FD prefix
					break;
				}
			}
			break;
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			specification = alu2(y) + " %1$02XH";
			dumpCount++;
			break;
		case 7:	// Restart: RST y * 8
			specification = "RST " + hex((uint8_t)y);
			break;
		}
		break;
	}
	return specification;
}

std::string EightBit::Disassembler::flag(uint8_t value, int flag, std::string represents, std::string off) {
	return value & flag ? represents : off;
}

std::string EightBit::Disassembler::flags(uint8_t value) {
	std::string returned;
	returned.reserve(8);
	returned += flag(value, Intel8080::SF, "S");
	returned += flag(value, Intel8080::ZF, "Z");
	returned += flag(value, Chip::Bit5, "1", "0");
	returned += flag(value, Intel8080::AC, "A");
	returned += flag(value, Chip::Bit3, "1", "0");
	returned += flag(value, Intel8080::PF, "P");
	returned += flag(value, Chip::Bit1, "1", "0");
	returned += flag(value, Intel8080::CF, "C");
	return returned;
}

std::string EightBit::Disassembler::hex(uint8_t value) {
	static char digits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' , 'a', 'b', 'c', 'd', 'e', 'f' };
	std::string returned;
	returned.reserve(2);
	returned += digits[Chip::highNibble(value)];
	returned += digits[Chip::lowNibble(value)];
	return returned;
}

std::string EightBit::Disassembler::hex(uint16_t value) {
	const register16_t converted(value);
	return hex(converted.high) + hex(converted.low);
}

std::string EightBit::Disassembler::binary(uint8_t value) {
	std::ostringstream output;
	output << std::bitset<8>(value);
	return output.str();
}
