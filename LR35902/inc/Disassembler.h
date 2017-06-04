#pragma once

#include <string>
#include <boost/format.hpp>

class LR35902;

class Disassembler {
public:
	Disassembler();

	static std::string state(LR35902& cpu);
	std::string disassemble(LR35902& cpu);

	static std::string flag(uint8_t value, int flag, const std::string& represents);
	static std::string flags(uint8_t value);
	static std::string hex(uint8_t value);
	static std::string hex(uint16_t value);
	static std::string binary(uint8_t value);
	static std::string decimal(uint8_t value);

	static std::string invalid(uint8_t value);

private:
	mutable boost::format m_formatter;
	bool m_prefixCB;

	void disassemble(std::ostringstream& output, LR35902& cpu, uint16_t pc);

	void disassembleCB(
		std::ostringstream& output,
		LR35902& cpu,
		uint16_t pc,
		std::string& specification,
		int& dumpCount,
		int x, int y, int z,
		int p, int q);

	void disassembleOther(
		std::ostringstream& output,
		LR35902& cpu,
		uint16_t pc,
		std::string& specification,
		int& dumpCount,
		int x, int y, int z,
		int p, int q);

	std::string RP(int rp) const;
	std::string RP2(int rp) const;
	std::string R(int r) const;
	static std::string cc(int flag);
	static std::string alu(int which);
};
