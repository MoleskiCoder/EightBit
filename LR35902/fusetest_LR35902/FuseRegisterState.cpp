#include "stdafx.h"
#include "FuseRegisterState.h"

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
	file >> i;
	file >> r;
	file >> iff1;
	file >> iff2;
	file >> im;
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

	file << hex(registers[AF].word) << " ";
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