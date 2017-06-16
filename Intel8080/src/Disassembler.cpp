#include "stdafx.h"
#include "Disassembler.h"

#include <sstream>
#include <iomanip>
#include <bitset>

#include "Memory.h"
#include "Intel8080.h"

EightBit::Disassembler::Disassembler() {
}

std::string EightBit::Disassembler::state(Intel8080& cpu) {

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
		<< "PC=" << hex(pc.word)
		<< " "
		<< "SP=" << hex(sp.word)
		<< " " << "A=" << hex(a) << " " << "F=" << flags(f)
		<< " " << "B=" << hex(b) << " " << "C=" << hex(c)
		<< " " << "D=" << hex(d) << " " << "E=" << hex(e)
		<< " " << "H=" << hex(h) << " " << "L=" << hex(l);

	return output.str();
}

std::string EightBit::Disassembler::disassemble(Intel8080& cpu) {

	const auto& memory = cpu.getMemory();
	auto pc = cpu.getProgramCounter();
	auto opcode = memory.peek(pc.word);
	const auto& instruction = cpu.getInstructions()[opcode];

	std::ostringstream output;

	// hex opcode
	output << hex(opcode);

	// hex raw operand
	switch (instruction.mode) {
	case Intel8080::Immediate:
		output << hex(memory.peek(pc.word + 1));
		break;
	case Intel8080::Absolute:
		output << hex(memory.peek(pc.word + 1));
		output << hex(memory.peek(pc.word + 2));
		break;
	default:
		break;
	}
	output << "\t";

	// base disassembly
	output << instruction.disassembly;

	// disassembly operand
	switch (instruction.mode) {
	case Intel8080::Immediate:
		output << hex(memory.peek(pc.word + 1));
		break;
	case Intel8080::Absolute:
		output << hex(memory.peekWord(pc.word + 1));
		break;
	default:
		break;
	}

	return output.str();
}

std::string EightBit::Disassembler::flag(uint8_t value, int flag, const std::string& represents) {
	std::ostringstream output;
	output << (value & flag ? represents : "-");
	return output.str();
}

std::string EightBit::Disassembler::flags(uint8_t value) {
	std::ostringstream output;
	output
		<< flag(value, Intel8080::SF, "S")
		<< flag(value, Intel8080::ZF, "Z")
		<< "0"
		<< flag(value, Intel8080::AC, "A")
		<< "0"
		<< flag(value, Intel8080::PF, "P")
		<< "1"
		<< flag(value, Intel8080::CF, "C");
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

std::string EightBit::Disassembler::invalid(uint8_t value) {
	std::ostringstream output;
	output << "Invalid instruction: " << hex(value) << "(" << binary(value) << ")";
	return output.str();
}