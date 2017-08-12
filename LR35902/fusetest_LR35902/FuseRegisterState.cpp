#include "stdafx.h"
#include "FuseRegisterState.h"
#include <Processor.h>

Fuse::RegisterState::RegisterState()
: registers(NUMBER_OF_REGISTERS) {
}

void Fuse::RegisterState::read(std::ifstream& file) {
	readExternal(file);
	readInternal(file);
}

void Fuse::RegisterState::readExternal(std::ifstream& file) {
	for (int idx = 0; idx < registers.size(); ++idx) {
		int input;
		file >> input;
		registers[idx].word = input;
	}
}

void Fuse::RegisterState::readInternal(std::ifstream& file) {
	file >> halted;

	file >> std::dec;
	file >> tstates;
	file >> std::hex;
}

void Fuse::RegisterState::write(std::ofstream& file) {
	writeExternal(file);
	writeInternal(file);
}

std::string Fuse::RegisterState::hex(int value) {
	std::ostringstream output;
	output << std::hex
		<< std::setw(4)
		<< std::setfill('0')
		<< value;
	return output.str();
}

void Fuse::RegisterState::writeExternal(std::ofstream& file) {

	const auto z80_af = registers[AF];
	const auto z80_a = z80_af.high;
	const auto z80_f = z80_af.low;
	EightBit::register16_t lr35902_af;
	auto& lr35902_a = lr35902_af.high;
	auto& lr35902_f = lr35902_af.low;

	lr35902_a = z80_a;
	lr35902_f = 0;

	if (z80_f & EightBit::Processor::Bit6)	// ZF
		lr35902_f |= EightBit::Processor::Bit7;
	if (z80_f & EightBit::Processor::Bit1)	// NF
		lr35902_f |= EightBit::Processor::Bit6;
	if (z80_f & EightBit::Processor::Bit4)	// HC
		lr35902_f |= EightBit::Processor::Bit5;
	if (z80_f & EightBit::Processor::Bit0)	// CF
		lr35902_f |= EightBit::Processor::Bit4;

	file << hex(lr35902_af.word) << " ";
	file << hex(registers[BC].word) << " ";
	file << hex(registers[DE].word) << " ";
	file << hex(registers[HL].word) << " ";

	file << hex(registers[SP].word) << " ";
	file << hex(registers[PC].word) << " ";

	file << std::endl;
}

void Fuse::RegisterState::writeInternal(std::ofstream& file) {

	file
		<< std::hex
		<< std::setfill('0');

	file << halted << " ";

	file << std::dec;
	file << tstates;
	file << std::hex;
}