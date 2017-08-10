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
	file >> halted;

	file >> std::dec;
	file >> tstates;
	file >> std::hex;
}
