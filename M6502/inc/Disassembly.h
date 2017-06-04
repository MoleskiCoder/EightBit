#pragma once

#include <map>
#include <cstdint>

#include <mos6502.h>
#include <AddressingMode.h>

#include "Symbols.h"
#include "AddressingModeDumper.h"

class Disassembly
{
public:
	MOS6502& processor;
	const Symbols& symbols;

	std::map<AddressingMode, AddressingModeDumper> dumpers;

	Disassembly(MOS6502& processor, const Symbols& symbols);

	std::string Dump_ByteValue(uint8_t value) const;
	std::string Dump_WordValue(uint16_t value) const;

	std::string DumpBytes(AddressingMode mode, uint16_t current) const;
	std::string Disassemble(uint16_t current) const;
	std::string DumpOperand(AddressingMode mode, uint16_t current) const;

private:
	uint8_t GetByte(uint16_t address) const;
	uint16_t GetWord(uint16_t address) const;

	std::string Dump_Nothing(uint16_t unused) const;
	std::string Dump_Byte(uint16_t address) const;
	std::string Dump_DByte(uint16_t address) const;

	std::string ConvertAddress(uint16_t address) const;
	std::string ConvertAddress(uint8_t address) const;
	std::string ConvertConstant(uint16_t constant) const;
	std::string ConvertConstant(uint8_t constant) const;

	std::string Dump_A(uint16_t unused) const;
	std::string Dump_imm(uint16_t current) const;
	std::string Dump_abs(uint16_t current) const;
	std::string Dump_zp(uint16_t current) const;
	std::string Dump_zpx(uint16_t current) const;
	std::string Dump_zpy(uint16_t current) const;
	std::string Dump_absx(uint16_t current) const;
	std::string Dump_absy(uint16_t current) const;
	std::string Dump_absxind(uint16_t current) const;
	std::string Dump_xind(uint16_t current) const;
	std::string Dump_indy(uint16_t current) const;
	std::string Dump_ind(uint16_t current) const;
	std::string Dump_zpind(uint16_t current) const;
	std::string Dump_rel(uint16_t current) const;
	std::string Dump_zprel(uint16_t current) const;

	const AddressingModeDumper& getDumper(AddressingMode mode) const {
		auto dumper = dumpers.find(mode);
		if (dumper == dumpers.end())
			throw std::invalid_argument("Illegal addressing mode");
		return dumper->second;
	}
};