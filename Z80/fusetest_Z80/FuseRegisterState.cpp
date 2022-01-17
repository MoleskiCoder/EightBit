#include "stdafx.h"
#include "FuseRegisterState.h"

#include <Z80.h>

Fuse::RegisterState::RegisterState()
: registers(NUMBER_OF_REGISTERS) {
}

void Fuse::RegisterState::read(std::ifstream& file) {
	readExternal(file);
	readInternal(file);
}

void Fuse::RegisterState::readExternal(std::ifstream& file) {
	for (auto& r : registers) {
		int input;
		file >> input;
		r = input;
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

void Fuse::RegisterState::transfer(EightBit::Z80& cpu) const {

	cpu.AF() = registers[AF_];
	cpu.BC() = registers[BC_];
	cpu.DE() = registers[DE_];
	cpu.HL() = registers[HL_];
	cpu.exx();
	cpu.exxAF();
	cpu.AF() = registers[AF];
	cpu.BC() = registers[BC];
	cpu.DE() = registers[DE];
	cpu.HL() = registers[HL];

	cpu.IX() = registers[IX];
	cpu.IY() = registers[IY];

	cpu.SP() = registers[SP];
	cpu.PC() = registers[PC];

	cpu.MEMPTR() = registers[MEMPTR];

	cpu.IV() = i;
	cpu.REFRESH() = r;
	cpu.IFF1() = iff1;
	cpu.IFF2() = iff2;
	cpu.IM() = im;
}
