#include "stdafx.h"
#include "mos6502.h"

MOS6502::MOS6502(ProcessorType cpuLevel)
:	level(cpuLevel)
{
	Install6502Instructions();
	Install65sc02Instructions();
	Install65c02Instructions();
}

void MOS6502::Initialise() {
	cycles = 0;
	ResetRegisters();
}

void MOS6502::Start(uint16_t address) {
	pc = address;
}

void MOS6502::Run() {
	while (proceed)
		Step();
}

void MOS6502::Step() {
	Execute(FetchByte());
}

void MOS6502::Reset() {
	pc = GetWord(RSTvector);
}

void MOS6502::TriggerIRQ() {
	Interrupt(IRQvector);
}

void MOS6502::TriggerNMI() {
	Interrupt(NMIvector);
}

uint16_t MOS6502::GetWord(uint16_t offset) const {
	auto low = GetByte(offset);
	auto high = GetByte((uint16_t)(offset + 1));
	return MakeWord(low, high);
}

void MOS6502::Interrupt(uint16_t vector) {
	PushWord(pc);
	PushByte(p);
	p.interrupt = true;
	pc = GetWord(vector);
}

void MOS6502::Execute(uint8_t cell) {
	const auto& instruction = instructions[cell];
	const auto& method = instruction.vector;
	method();
	cycles += instruction.count;
}

void MOS6502::___() {
	if (level >= ProcessorType::Cpu65SC02) {
		// Generally, missing instructions act as a one byte,
		// one cycle NOP instruction on 65c02 (ish) processors.
		NOP_imp();
		cycles++;
	} else {
		throw new std::domain_error("Whoops: Invalid instruction.");
	}
}

void MOS6502::ResetRegisters() {
	pc = 0;
	x = 0x80;
	y = 0;
	a = 0;

	p = 0;
	p.reserved = true;

	s = 0xff;
}

MOS6502::Instruction MOS6502::INS(instruction_t method, uint64_t cycles, AddressingMode addressing, std::string display) {
	MOS6502::Instruction returnValue;
	returnValue.vector = method;
	returnValue.count = cycles;
	returnValue.mode = addressing;
	returnValue.display = display;
	return returnValue;
}

uint8_t MOS6502::LowNybble(uint8_t value) {
	return value & 0xf;
}

uint8_t MOS6502::HighNybble(uint8_t value) {
	return DemoteNybble(value);
}

uint8_t MOS6502::PromoteNybble(uint8_t value) {
	return value << 4;
}

uint8_t MOS6502::DemoteNybble(uint8_t value) {
	return value >> 4;
}

uint8_t MOS6502::LowByte(uint16_t value) {
	return value & 0xff;
}

uint8_t MOS6502::HighByte(uint16_t value) {
	return (value & ~0xff) >> 8;
}

uint16_t MOS6502::MakeWord(uint8_t low, uint8_t high) {
	return (high << 8) + low;
}

////

#define BIND(method)	std::bind(&MOS6502:: method, this)

void MOS6502::Install6502Instructions() {
	overlay6502 = {
		////	0															1															2															3													4															5															6															7													8														9															A															B													C															D															E															F
		/* 0 */	INS(BIND(BRK_imp), 7, AddressingMode::Implied, "BRK"),		INS(BIND(ORA_xind), 6, AddressingMode::XIndexed, "ORA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(ORA_zp), 4, AddressingMode::ZeroPage, "ORA"),		INS(BIND(ASL_zp), 5, AddressingMode::ZeroPage, "ASL"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(PHP_imp), 3, AddressingMode::Implied, "PHP"),	INS(BIND(ORA_imm), 2, AddressingMode::Immediate, "ORA"),	INS(BIND(ASL_a), 2, AddressingMode::Accumulator, "ASL"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(ORA_abs), 4, AddressingMode::Absolute, "ORA"),		INS(BIND(ASL_abs), 6, AddressingMode::Absolute, "ASL"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 1 */	INS(BIND(BPL_rel), 2, AddressingMode::Relative, "BPL"),		INS(BIND(ORA_indy), 5, AddressingMode::IndexedY, "ORA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(ORA_zpx), 4, AddressingMode::ZeroPageX, "ORA"),	INS(BIND(ASL_zpx), 6, AddressingMode::ZeroPageX, "ASL"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CLC_imp), 2, AddressingMode::Implied, "CLC"),	INS(BIND(ORA_absy), 4, AddressingMode::AbsoluteY, "ORA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(ORA_absx), 4, AddressingMode::AbsoluteX, "ORA"),	INS(BIND(ASL_absx), 7, AddressingMode::AbsoluteX, "ASL"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 2 */	INS(BIND(JSR_abs), 6, AddressingMode::Absolute, "JSR"),		INS(BIND(AND_xind), 6, AddressingMode::XIndexed, "AND"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(BIT_zp), 3, AddressingMode::ZeroPage, "BIT"),		INS(BIND(AND_zp), 3, AddressingMode::ZeroPage, "AND"),		INS(BIND(ROL_zp), 5, AddressingMode::ZeroPage, "ROL"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(PLP_imp), 4, AddressingMode::Implied, "PLP"),	INS(BIND(AND_imm),2, AddressingMode::Immediate, "AND"),		INS(BIND(ROL_a), 2, AddressingMode::Accumulator, "ROL"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(BIT_abs), 4, AddressingMode::Absolute, "BIT"),		INS(BIND(AND_abs), 4, AddressingMode::Absolute, "AND"),		INS(BIND(ROL_abs), 6, AddressingMode::Absolute, "ROL"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 3 */	INS(BIND(BMI_rel), 2, AddressingMode::Relative, "BMI"),		INS(BIND(AND_indy), 5, AddressingMode::IndexedY, "AND"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(AND_zpx), 4, AddressingMode::ZeroPageX, "AND"),	INS(BIND(ROL_zpx), 6, AddressingMode::ZeroPageX, "ROL"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SEC_imp), 2, AddressingMode::Implied, "SEC"),	INS(BIND(AND_absy), 4, AddressingMode::AbsoluteY, "AND"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(AND_absx), 4, AddressingMode::AbsoluteX, "AND"),	INS(BIND(ROL_absx), 7, AddressingMode::AbsoluteX, "ROL"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 4 */	INS(BIND(RTI_imp), 6, AddressingMode::Implied, "RTI"),		INS(BIND(EOR_xind), 6, AddressingMode::XIndexed, "EOR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(EOR_zp), 3, AddressingMode::ZeroPage, "EOR"),		INS(BIND(LSR_zp), 5, AddressingMode::ZeroPage, "LSR"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(PHA_imp), 3, AddressingMode::Implied, "PHA"),	INS(BIND(EOR_imm),2, AddressingMode::Immediate, "EOR"),		INS(BIND(LSR_a), 2, AddressingMode::Accumulator, "LSR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(JMP_abs), 3, AddressingMode::Absolute, "JMP"),		INS(BIND(EOR_abs), 4, AddressingMode::Absolute, "EOR"),		INS(BIND(LSR_abs), 6, AddressingMode::Absolute, "LSR"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 5 */	INS(BIND(BVC_rel), 2, AddressingMode::Relative, "BVC"),		INS(BIND(EOR_indy), 5, AddressingMode::IndexedY, "EOR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(EOR_zpx), 4, AddressingMode::ZeroPageX, "EOR"),	INS(BIND(LSR_zpx), 6, AddressingMode::ZeroPageX, "LSR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CLI_imp), 2, AddressingMode::Implied, "CLI"),	INS(BIND(EOR_absy), 4, AddressingMode::AbsoluteY, "EOR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(EOR_absx), 4, AddressingMode::AbsoluteX, "EOR"),	INS(BIND(LSR_absx), 7, AddressingMode::AbsoluteX, "LSR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 6 */	INS(BIND(RTS_imp), 6, AddressingMode::Implied, "RTS"),		INS(BIND(ADC_xind), 6, AddressingMode::XIndexed, "ADC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(ADC_zp), 3, AddressingMode::ZeroPage, "ADC"),		INS(BIND(ROR_zp), 5, AddressingMode::ZeroPage, "ROR"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(PLA_imp), 4, AddressingMode::Implied, "PLA"),	INS(BIND(ADC_imm),2, AddressingMode::Immediate, "ADC"),		INS(BIND(ROR_a), 2, AddressingMode::Accumulator, "ROR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(JMP_ind), 5, AddressingMode::Indirect, "JMP"),		INS(BIND(ADC_abs), 4, AddressingMode::Absolute, "ADC"),		INS(BIND(ROR_abs), 6, AddressingMode::Absolute, "ROR"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 7 */	INS(BIND(BVS_rel), 2, AddressingMode::Relative, "BVS"),		INS(BIND(ADC_indy), 5, AddressingMode::IndexedY, "ADC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(ADC_zpx), 4, AddressingMode::ZeroPageX, "ADC"),	INS(BIND(ROR_zpx), 6, AddressingMode::ZeroPageX, "ROR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SEI_imp), 2, AddressingMode::Implied, "SEI"),	INS(BIND(ADC_absy), 4, AddressingMode::AbsoluteY, "ADC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(ADC_absx), 4, AddressingMode::AbsoluteX, "ADC"),	INS(BIND(ROR_absx), 7, AddressingMode::AbsoluteX, "ROR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 8 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(STA_xind), 6, AddressingMode::XIndexed, "STA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(STY_zp), 3, AddressingMode::ZeroPage, "STY"),		INS(BIND(STA_zp), 3, AddressingMode::ZeroPage, "STA"),		INS(BIND(STX_zp), 3, AddressingMode::ZeroPage, "STX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(DEY_imp), 2, AddressingMode::Implied, "DEY"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(TXA_imp), 2, AddressingMode::Implied, "TXA"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(STY_abs), 4, AddressingMode::Absolute, "STY"),		INS(BIND(STA_abs), 4, AddressingMode::Absolute, "STA"),		INS(BIND(STX_abs), 4, AddressingMode::Absolute, "STX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* 9 */	INS(BIND(BCC_rel), 2, AddressingMode::Relative, "BCC"),		INS(BIND(STA_indy), 6, AddressingMode::IndexedY, "STA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(STY_zpx), 4, AddressingMode::ZeroPageX, "STY"),	INS(BIND(STA_zpx), 4, AddressingMode::ZeroPageX, "STA"),	INS(BIND(STX_zpy), 4, AddressingMode::ZeroPageY, "STX"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(TYA_imp), 2, AddressingMode::Implied, "TYA"),	INS(BIND(STA_absy), 5, AddressingMode::AbsoluteY, "STA"),	INS(BIND(TXS_imp), 2, AddressingMode::Implied, "TXS"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(STA_absx), 5, AddressingMode::AbsoluteX, "STA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* A */	INS(BIND(LDY_imm), 2, AddressingMode::Immediate, "LDY"),	INS(BIND(LDA_xind), 6, AddressingMode::XIndexed, "LDA"),	INS(BIND(LDX_imm), 2, AddressingMode::Immediate, "LDX"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(LDY_zp), 3, AddressingMode::ZeroPage, "LDY"),		INS(BIND(LDA_zp), 3, AddressingMode::ZeroPage, "LDA"),		INS(BIND(LDX_zp), 3, AddressingMode::ZeroPage, "LDX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(TAY_imp), 2, AddressingMode::Implied, "TAY"),	INS(BIND(LDA_imm),2, AddressingMode::Immediate, "LDA"),		INS(BIND(TAX_imp), 2, AddressingMode::Implied, "TAX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(LDY_abs), 4, AddressingMode::Absolute, "LDY"),		INS(BIND(LDA_abs), 4, AddressingMode::Absolute, "LDA"),		INS(BIND(LDX_abs), 4, AddressingMode::Absolute, "LDX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* B */	INS(BIND(BCS_rel), 2, AddressingMode::Relative, "BCS"),		INS(BIND(LDA_indy), 5, AddressingMode::IndexedY, "LDA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(LDY_zpx), 4, AddressingMode::ZeroPageX, "LDY"),	INS(BIND(LDA_zpx), 4, AddressingMode::ZeroPageX, "LDA"),	INS(BIND(LDX_zpy), 4, AddressingMode::ZeroPageY, "LDX"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CLV_imp), 2, AddressingMode::Implied, "CLV"),	INS(BIND(LDA_absy), 4, AddressingMode::AbsoluteY, "LDA"),	INS(BIND(TSX_imp), 2, AddressingMode::Implied, "TSX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(LDY_absx), 4, AddressingMode::AbsoluteX, "LDY"),	INS(BIND(LDA_absx), 4, AddressingMode::AbsoluteX, "LDA"),	INS(BIND(LDX_absy), 4, AddressingMode::AbsoluteY, "LDX"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* C */	INS(BIND(CPY_imm), 2, AddressingMode::Immediate, "CPY"),	INS(BIND(CMP_xind), 6, AddressingMode::XIndexed, "CMP"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CPY_zp), 3, AddressingMode::ZeroPage, "CPY"),		INS(BIND(CMP_zp), 3, AddressingMode::ZeroPage, "CMP"),		INS(BIND(DEC_zp), 5, AddressingMode::ZeroPage, "DEC"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(INY_imp), 2, AddressingMode::Implied, "INY"),	INS(BIND(CMP_imm),2, AddressingMode::Immediate, "CMP"),		INS(BIND(DEX_imp), 2, AddressingMode::Implied, "DEX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CPY_abs), 4, AddressingMode::Absolute, "CPY"),		INS(BIND(CMP_abs), 4, AddressingMode::Absolute, "CMP"),		INS(BIND(DEC_abs), 6, AddressingMode::Absolute, "DEC"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* D */	INS(BIND(BNE_rel), 2, AddressingMode::Relative, "BNE"),		INS(BIND(CMP_indy), 5, AddressingMode::IndexedY, "CMP"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(CMP_zpx), 4, AddressingMode::ZeroPageX, "CMP"),	INS(BIND(DEC_zpx), 6, AddressingMode::ZeroPageX, "DEC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CLD_imp), 2, AddressingMode::Implied, "CLD"),	INS(BIND(CMP_absy), 4, AddressingMode::AbsoluteY, "CMP"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(CMP_absx), 4, AddressingMode::AbsoluteX, "CMP"),	INS(BIND(DEC_absx), 7, AddressingMode::AbsoluteX, "DEC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* E */	INS(BIND(CPX_imm), 2, AddressingMode::Immediate, "CPX"),	INS(BIND(SBC_xind), 6, AddressingMode::XIndexed, "SBC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CPX_zp), 3, AddressingMode::ZeroPage, "CPX"),		INS(BIND(SBC_zp), 3, AddressingMode::ZeroPage, "SBC"),		INS(BIND(INC_zp), 5, AddressingMode::ZeroPage, "INC"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(INX_imp), 2, AddressingMode::Implied, "INX"),	INS(BIND(SBC_imm),2, AddressingMode::Immediate, "SBC"),		INS(BIND(NOP_imp), 2, AddressingMode::Implied, "NOP"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CPX_abs), 4, AddressingMode::Absolute, "CPX"),		INS(BIND(SBC_abs), 4, AddressingMode::Absolute, "SBC"),		INS(BIND(INC_abs), 6, AddressingMode::Absolute, "INC"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),
		/* F */	INS(BIND(BEQ_rel), 2, AddressingMode::Relative, "BEQ"),		INS(BIND(SBC_indy), 5, AddressingMode::IndexedY, "SBC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(SBC_zpx), 4, AddressingMode::ZeroPageX, "SBC"),	INS(BIND(INC_zpx), 6, AddressingMode::ZeroPageX, "INC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SED_imp), 2, AddressingMode::Implied, "SED"),	INS(BIND(SBC_absy), 4, AddressingMode::AbsoluteY, "SBC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(SBC_absx), 4, AddressingMode::AbsoluteX, "SBC"),	INS(BIND(INC_absx), 7, AddressingMode::AbsoluteX, "INC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),
	};

	InstallInstructionSet(overlay6502);
}

void MOS6502::Install65sc02Instructions() {
	if (level >= ProcessorType::Cpu65SC02) {
		overlay65sc02 = {
			////	0 														1													2																	3													4															5													6													7													8													9														A															B													C																		D													E															F
			/* 0 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 2, AddressingMode::Implied, "___"),				INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(TSB_zp), 5, AddressingMode::ZeroPage, "TSB"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(TSB_abs), 6, AddressingMode::Absolute, "TSB"),					INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 1 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(ORA_zpind), 5, AddressingMode::ZeroPageIndirect, "ORA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(TRB_zp), 5, AddressingMode::ZeroPage, "TRB"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(INC_a), 2, AddressingMode::Accumulator, "INC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(TRB_abs), 6, AddressingMode::Absolute, "TRB"),					INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 2 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 2, AddressingMode::Implied, "___"),				INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),						INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 3 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(AND_zpind), 5, AddressingMode::ZeroPageIndirect, "AND"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(BIT_zpx), 4, AddressingMode::ZeroPageX, "BIT"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(DEC_a), 2, AddressingMode::Accumulator, "DEC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(BIT_absx), 4, AddressingMode::AbsoluteX, "BIT"),				INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 4 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 2, AddressingMode::Implied, "___"),				INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 3, AddressingMode::Implied, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),						INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 5 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(EOR_zpind), 5, AddressingMode::ZeroPageIndirect, "EOR"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 4, AddressingMode::Implied, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(PHY_imp), 2, AddressingMode::Implied, "PHY"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP3_imp), 8, AddressingMode::Implied, "___"),					INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 6 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 2, AddressingMode::Implied, "___"),				INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(STZ_zp), 3, AddressingMode::ZeroPage, "STZ"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),						INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 7 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(ADC_zpind), 5, AddressingMode::ZeroPageIndirect, "ADC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(STZ_zpx), 4, AddressingMode::ZeroPageX, "STZ"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(PLY_imp), 2, AddressingMode::Implied, "PLY"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(JMP_absxind), 6, AddressingMode::AbsoluteXIndirect, "JMP"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 8 */	INS(BIND(BRA_rel), 2, AddressingMode::Relative, "BRA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 2, AddressingMode::Implied, "___"),				INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(BIT_imm),2, AddressingMode::Immediate, "BIT"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),						INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* 9 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(STA_zpind), 5, AddressingMode::ZeroPageIndirect, "STA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(STZ_abs), 4, AddressingMode::Absolute, "STZ"),					INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(STZ_absx), 2, AddressingMode::AbsoluteX, "STZ"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* A */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),					INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),						INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* B */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(LDA_zpind), 5, AddressingMode::ZeroPageIndirect, "LDA"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),						INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* C */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 2, AddressingMode::Implied, "___"),				INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),						INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* D */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(CMP_zpind), 5, AddressingMode::ZeroPageIndirect, "CMP"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 4, AddressingMode::Implied, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(PHX_imp), 2, AddressingMode::Implied, "PHX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP3_imp), 4, AddressingMode::Implied, "___"),					INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* E */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 2, AddressingMode::Implied, "___"),				INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),						INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
			/* F */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SBC_zpind), 5, AddressingMode::ZeroPageIndirect, "SBC"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP2_imp), 4, AddressingMode::Implied, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),		INS(BIND(PLX_imp), 2, AddressingMode::Implied, "PLX"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(NOP3_imp), 4, AddressingMode::Implied, "___"),					INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),			INS(BIND(___), 2, AddressingMode::Illegal, "___"),
		};

		OverlayInstructionSet(overlay65sc02);
	}
}

void MOS6502::Install65c02Instructions() {
	if (level >= ProcessorType::Cpu65C02) {
		overlay65c02 = {
			////	0 													1													2													3													4													5													6													7                                                           8													9													A													B													    C													D													E													F
			/* 0 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(RMB0_zp), 5, AddressingMode::ZeroPage, "RMB0"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBR0_zprel), 5, AddressingMode::ZeroPageRelative, "BBR0"),
			/* 1 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(RMB1_zp), 5, AddressingMode::ZeroPage, "RMB1"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBR1_zprel), 5, AddressingMode::ZeroPageRelative, "BBR1"),
			/* 2 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(RMB2_zp), 5, AddressingMode::ZeroPage, "RMB2"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBR2_zprel), 5, AddressingMode::ZeroPageRelative, "BBR2"),
			/* 3 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(RMB3_zp), 5, AddressingMode::ZeroPage, "RMB3"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBR3_zprel), 5, AddressingMode::ZeroPageRelative, "BBR3"),
			/* 4 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(RMB4_zp), 5, AddressingMode::ZeroPage, "RMB4"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBR4_zprel), 5, AddressingMode::ZeroPageRelative, "BBR4"),
			/* 5 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(RMB5_zp), 5, AddressingMode::ZeroPage, "RMB5"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBR5_zprel), 5, AddressingMode::ZeroPageRelative, "BBR5"),
			/* 6 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(RMB6_zp), 5, AddressingMode::ZeroPage, "RMB6"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBR6_zprel), 5, AddressingMode::ZeroPageRelative, "BBR6"),
			/* 7 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(RMB7_zp), 5, AddressingMode::ZeroPage, "RMB7"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBR7_zprel), 5, AddressingMode::ZeroPageRelative, "BBR7"),
			/* 8 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SMB0_zp), 5, AddressingMode::ZeroPage, "SMB0"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBS0_zprel), 5, AddressingMode::ZeroPageRelative, "BBS0"),
			/* 9 */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SMB1_zp), 5, AddressingMode::ZeroPage, "SMB1"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(BBS1_zprel), 5, AddressingMode::ZeroPageRelative, "BBS1"),
			/* A */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SMB2_zp), 5, AddressingMode::ZeroPage, "SMB2"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBS2_zprel), 5, AddressingMode::ZeroPageRelative, "BBS2"),
			/* B */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SMB3_zp), 5, AddressingMode::ZeroPage, "SMB3"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBS3_zprel), 5, AddressingMode::ZeroPageRelative, "BBS3"),
			/* C */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SMB4_zp), 5, AddressingMode::ZeroPage, "SMB4"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(WAI_imp), 3, AddressingMode::Implied, "WAI"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBS4_zprel), 5, AddressingMode::ZeroPageRelative, "BBS4"),
			/* D */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SMB5_zp), 5, AddressingMode::ZeroPage, "SMB5"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(STP_imp), 3, AddressingMode::Implied, "STP"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBS5_zprel), 5, AddressingMode::ZeroPageRelative, "BBS5"),
			/* E */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SMB6_zp), 5, AddressingMode::ZeroPage, "SMB6"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBS6_zprel), 5, AddressingMode::ZeroPageRelative, "BBS6"),
			/* F */	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(SMB7_zp), 5, AddressingMode::ZeroPage, "SMB7"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 0, AddressingMode::Illegal, "___"),		INS(BIND(___), 0, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(___), 2, AddressingMode::Illegal, "___"),	INS(BIND(BBS7_zprel), 5, AddressingMode::ZeroPageRelative, "BBS7"),
		};

		OverlayInstructionSet(overlay65c02);
	}
}

void MOS6502::InstallInstructionSet(std::array<Instruction, 0x100> basis) {
	OverlayInstructionSet(basis, true);
}

void MOS6502::OverlayInstructionSet(std::array<Instruction, 0x100> overlay) {
	OverlayInstructionSet(overlay, false);
}

void MOS6502::OverlayInstructionSet(std::array<Instruction, 0x100> overlay, bool includeIllegal) {
	for (uint16_t i = 0; i < 0x100; ++i) {
		auto newInstruction = overlay[i];
		auto illegal = newInstruction.mode == AddressingMode::Illegal;
		if (includeIllegal || !illegal) {
			auto oldInstruction = instructions[i];
			if (oldInstruction.mode != AddressingMode::Illegal) {
				throw new std::domain_error("Whoops: replacing a non-missing instruction.");
			}

			instructions[i] = newInstruction;
		}
	}
}

////

bool MOS6502::UpdateZeroFlag(uint8_t datum) {
	return p.zero = datum == 0;
}

bool MOS6502::UpdateNegativeFlag(int8_t datum) {
	return p.negative = datum < 0;
}

void MOS6502::UpdateZeroNegativeFlags(uint8_t datum) {
	if (UpdateNegativeFlag((int8_t)datum))
		p.zero = false;
	else
		UpdateZeroFlag(datum);
}

////

void MOS6502::PushByte(uint8_t value) {
	SetByte(PageOne + s--, value);
}

uint8_t MOS6502::PopByte() {
	return GetByte(PageOne + ++s);
}

void MOS6502::PushWord(uint16_t value) {
	PushByte(HighByte(value));
	PushByte(LowByte(value));
}

uint16_t MOS6502::PopWord() {
	auto low = PopByte();
	auto high = PopByte();
	return MakeWord(low, high);
}

uint8_t MOS6502::FetchByte() {
	return GetByte(pc++);
}

uint16_t MOS6502::FetchWord() {
	auto word = GetWord(pc);
	pc += 2;
	return word;
}

////

uint16_t MOS6502::Address_ZeroPage() {
	return FetchByte();
}

uint16_t MOS6502::Address_ZeroPageX() {
	return LowByte(FetchByte() + x);
}

uint16_t MOS6502::Address_ZeroPageY() {
	return LowByte(FetchByte() + y);
}

uint16_t MOS6502::Address_IndexedIndirectX() {
	return GetWord(Address_ZeroPageX());
}

uint16_t MOS6502::Address_IndexedIndirectY_Read() {
	auto indirection = GetWord(FetchByte());
	if (LowByte(indirection) == 0xff)
		++cycles;
	return indirection + y;
}

uint16_t MOS6502::Address_IndexedIndirectY_Write() {
	return GetWord(FetchByte()) + y;
}

uint16_t MOS6502::Address_Absolute() {
	return FetchWord();
}

uint16_t MOS6502::Address_AbsoluteXIndirect() {
	return GetWord(FetchWord() + x);
}

uint16_t MOS6502::Address_AbsoluteX_Read() {
	auto address = FetchWord();
	auto offset = (uint16_t)(address + x);
	if (LowByte(offset) == 0xff)
		++cycles;
	return offset;
}

uint16_t MOS6502::Address_AbsoluteX_Write() {
	return FetchWord() + x;
}

uint16_t MOS6502::Address_AbsoluteY_Read() {
	auto address = FetchWord();
	auto offset = (uint16_t)(address + y);
	if (LowByte(offset) == 0xff)
		++cycles;
	return offset;
}

uint16_t MOS6502::Address_AbsoluteY_Write() {
	return FetchWord() + y;
}

uint16_t MOS6502::Address_ZeroPageIndirect() {
	return GetWord(FetchByte());
}

////

uint8_t MOS6502::ReadByte_Immediate() {
	return FetchByte();
}

int8_t MOS6502::ReadByte_ImmediateDisplacement() {
	return FetchByte();
}

uint8_t MOS6502::ReadByte_ZeroPage() {
	return GetByte(Address_ZeroPage());
}

uint8_t MOS6502::ReadByte_ZeroPageX() {
	return GetByte(Address_ZeroPageX());
}

uint8_t MOS6502::ReadByte_ZeroPageY() {
	return GetByte(Address_ZeroPageY());
}

uint8_t MOS6502::ReadByte_Absolute() {
	return GetByte(Address_Absolute());
}

uint8_t MOS6502::ReadByte_AbsoluteX() {
	return GetByte(Address_AbsoluteX_Read());
}

uint8_t MOS6502::ReadByte_AbsoluteY() {
	return GetByte(Address_AbsoluteY_Read());
}

uint8_t MOS6502::ReadByte_IndexedIndirectX() {
	return GetByte(Address_IndexedIndirectX());
}

uint8_t MOS6502::ReadByte_IndirectIndexedY() {
	return GetByte(Address_IndexedIndirectY_Read());
}

uint8_t MOS6502::ReadByte_ZeroPageIndirect() {
	return GetByte(Address_ZeroPageIndirect());
}

////

void MOS6502::WriteByte_ZeroPage(uint8_t value) {
	SetByte(Address_ZeroPage(), value);
}

void MOS6502::WriteByte_Absolute(uint8_t value) {
	SetByte(Address_Absolute(), value);
}

void MOS6502::WriteByte_AbsoluteX(uint8_t value) {
	SetByte(Address_AbsoluteX_Write(), value);
}

void MOS6502::WriteByte_AbsoluteY(uint8_t value) {
	SetByte(Address_AbsoluteY_Write(), value);
}

void MOS6502::WriteByte_ZeroPageX(uint8_t value) {
	SetByte(Address_ZeroPageX(), value);
}

void MOS6502::WriteByte_ZeroPageY(uint8_t value) {
	SetByte(Address_ZeroPageY(), value);
}

void MOS6502::WriteByte_IndirectIndexedY(uint8_t value) {
	SetByte(Address_IndexedIndirectY_Write(), value);
}

void MOS6502::WriteByte_IndexedIndirectX(uint8_t value) {
	SetByte(Address_IndexedIndirectX(), value);
}

void MOS6502::WriteByte_ZeroPageIndirect(uint8_t value) {
	SetByte(Address_ZeroPageIndirect(), value);
}

////

void MOS6502::DEC(uint16_t offset) {
	auto content = GetByte(offset);
	SetByte(offset, --content);
	UpdateZeroNegativeFlags(content);
}

uint8_t MOS6502::ROR(uint8_t data) {
	auto carry = p.carry;

	p.carry = (data & 1) != 0;

	auto result = (uint8_t)(data >> 1);
	if (carry)
		result |= 0x80;

	UpdateZeroNegativeFlags(result);

	return result;
}

void MOS6502::ROR(uint16_t offset) {
	SetByte(offset, ROR(GetByte(offset)));
}

uint8_t MOS6502::LSR(uint8_t data) {
	p.carry = (data & 1) != 0;

	auto result = (uint8_t)(data >> 1);

	UpdateZeroNegativeFlags(result);

	return result;
}

void MOS6502::LSR(uint16_t offset) {
	SetByte(offset, LSR(GetByte(offset)));
}

void MOS6502::BIT_immediate(uint8_t data) {
	auto result = (uint8_t)(a & data);
	UpdateZeroFlag(result);
}

void MOS6502::BIT(uint8_t data) {
	BIT_immediate(data);
	p.negative = (data & 0x80) != 0;
	p.overflow = (data & 0x40) != 0;
}

void MOS6502::TSB(uint16_t address) {
	auto content = GetByte(address);
	BIT_immediate(content);
	uint8_t result = content | a;
	SetByte(address, result);
}

void MOS6502::TRB(uint16_t address) {
	auto content = GetByte(address);
	BIT_immediate(content);
	uint8_t result = content & ~a;
	SetByte(address, result);
}

void MOS6502::INC(uint16_t offset) {
	auto content = GetByte(offset);
	SetByte(offset, ++content);
	UpdateZeroNegativeFlags(content);
}

void MOS6502::ROL(uint16_t offset) {
	SetByte(offset, ROL(GetByte(offset)));
}

uint8_t MOS6502::ROL(uint8_t data) {
	auto carry = p.carry;

	p.carry = (data & 0x80) != 0;

	uint8_t result = data << 1;

	if (carry)
		result |= 1;

	UpdateZeroNegativeFlags(result);

	return result;
}

void MOS6502::ASL(uint16_t offset) {
	SetByte(offset, ASL(GetByte(offset)));
}

uint8_t MOS6502::ASL(uint8_t data) {
	uint8_t result = data << 1;
	UpdateZeroNegativeFlags(result);
	p.carry = (data & 0x80) != 0;
	return result;
}

void MOS6502::ORA(uint8_t data) {
	a |= data;
	UpdateZeroNegativeFlags(a);
}

void MOS6502::AND(uint8_t data) {
	a &= data;
	UpdateZeroNegativeFlags(a);
}

void MOS6502::SBC(uint8_t data) {
	if (p.decimal)
		SBC_d(data);
	else
		SBC_b(data);
}

void MOS6502::SBC_b(uint8_t data) {
	auto carry = p.carry ? 0 : 1;
	auto difference = a - data - carry;

	UpdateZeroNegativeFlags((uint8_t)difference);
	p.overflow = ((a ^ data) & (a ^ difference) & 0x80) != 0;
	p.carry = HighByte((uint16_t)difference) == 0;

	a = (uint8_t)difference;
}

void MOS6502::SBC_d(uint8_t data) {
	auto carry = p.carry ? 0 : 1;
	auto difference = a - data - carry;

	if (level < ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags((uint8_t)difference);

	p.overflow = ((a ^ data) & (a ^ difference) & 0x80) != 0;
	p.carry = HighByte((uint16_t)difference) == 0;

	auto low = (uint8_t)(LowNybble(a) - LowNybble(data) - carry);

	auto lowNegative = (int8_t)low < 0;
	if (lowNegative)
		low -= 6;

	uint8_t high = HighNybble(a) - HighNybble(data) - (lowNegative ? 1 : 0);

	if ((int8_t)high < 0)
		high -= 6;

	a = PromoteNybble(high) | LowNybble(low);
	if (level >= ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(a);
}

void MOS6502::EOR(uint8_t data) {
	a ^= data;
	UpdateZeroNegativeFlags(a);
}

void MOS6502::CPX(uint8_t data) {
	CMP(x, data);
}

void MOS6502::CPY(uint8_t data) {
	CMP(y, data);
}

void MOS6502::CMP(uint8_t data) {
	CMP(a, data);
}

void MOS6502::CMP(uint8_t first, uint8_t second) {
	uint16_t result = first - second;
	UpdateZeroNegativeFlags((uint8_t)result);
	p.carry = HighByte(result) == 0;
}

void MOS6502::LDA(uint8_t data) {
	a = data;
	UpdateZeroNegativeFlags(a);
}

void MOS6502::LDY(uint8_t data) {
	y = data;
	UpdateZeroNegativeFlags(y);
}

void MOS6502::LDX(uint8_t data) {
	x = data;
	UpdateZeroNegativeFlags(x);
}

void MOS6502::ADC(uint8_t data) {
	if (p.decimal)
		ADC_d(data);
	else
		ADC_b(data);
}

void MOS6502::ADC_b(uint8_t data) {
	auto carry = (uint8_t)(p.carry ? 1 : 0);
	auto sum = (uint16_t)(a + data + carry);

	UpdateZeroNegativeFlags((uint8_t)sum);
	p.overflow = (~(a ^ data) & (a ^ sum) & 0x80) != 0;
	p.carry = HighByte(sum) != 0;

	a = (uint8_t)sum;
}

void MOS6502::ADC_d(uint8_t data) {
	auto carry = (uint8_t)(p.carry ? 1 : 0);
	auto sum = (uint16_t)(a + data + carry);

	if (level < ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags((uint8_t)sum);

	auto low = (uint8_t)(LowNybble(a) + LowNybble(data) + carry);
	if (low > 9)
		low += 6;

	auto high = (uint8_t)(HighNybble(a) + HighNybble(data) + (low > 0xf ? 1 : 0));
	p.overflow = (~(a ^ data) & (a ^ PromoteNybble(high)) & 0x80) != 0;

	if (high > 9)
		high += 6;

	p.carry = high > 0xf;

	a = (uint8_t)(PromoteNybble(high) | LowNybble(low));
	if (level >= ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(a);
}

////

void MOS6502::RMB(uint16_t address, uint8_t flag) {
	auto data = GetByte(address);
	data &= (uint8_t)~flag;
	SetByte(address, data);
}

void MOS6502::SMB(uint16_t address, uint8_t flag) {
	auto data = GetByte(address);
	data |= flag;
	SetByte(address, data);
}

////

void MOS6502::Branch(int8_t displacement) {
	++cycles;
	auto oldPage = HighByte(pc);
	pc += (uint16_t)((short)displacement);
	auto newPage = HighByte(pc);
	if (oldPage != newPage)
		cycles += 2;
}

void MOS6502::Branch() {
	auto displacement = ReadByte_ImmediateDisplacement();
	Branch(displacement);
}

void MOS6502::Branch(bool flag) {
	auto displacement = ReadByte_ImmediateDisplacement();
	if (flag)
		Branch(displacement);
}

void MOS6502::BitBranch_Clear(uint8_t check) {
	auto zp = FetchByte();
	auto contents = GetByte(zp);
	auto displacement = FetchByte();
	if ((contents & check) == 0)
		pc += (uint16_t)displacement;
}

void MOS6502::BitBranch_Set(uint8_t check) {
	auto zp = FetchByte();
	auto contents = GetByte(zp);
	auto displacement = FetchByte();
	if ((contents & check) != 0)
		pc += (uint16_t)displacement;
}

//

void MOS6502::NOP_imp() {
}

void MOS6502::NOP2_imp() {
	FetchByte();
}

void MOS6502::NOP3_imp() {
	FetchWord();
}

//

void MOS6502::ORA_xind() {
	ORA(ReadByte_IndexedIndirectX());
}

void MOS6502::ORA_zp() {
	ORA(ReadByte_ZeroPage());
}

void MOS6502::ORA_imm() {
	ORA(ReadByte_Immediate());
}

void MOS6502::ORA_abs() {
	ORA(ReadByte_Absolute());
}

void MOS6502::ORA_absx() {
	ORA(ReadByte_AbsoluteX());
}

void MOS6502::ORA_absy() {
	ORA(ReadByte_AbsoluteY());
}

void MOS6502::ORA_zpx() {
	ORA(ReadByte_ZeroPageX());
}

void MOS6502::ORA_indy() {
	ORA(ReadByte_IndirectIndexedY());
}

void MOS6502::ORA_zpind() {
	ORA(ReadByte_ZeroPageIndirect());
}

//

void MOS6502::AND_zpx() {
	AND(ReadByte_ZeroPageX());
}

void MOS6502::AND_indy() {
	AND(ReadByte_IndirectIndexedY());
}

void MOS6502::AND_zp() {
	AND(ReadByte_ZeroPage());
}

void MOS6502::AND_absx() {
	AND(ReadByte_AbsoluteX());
}

void MOS6502::AND_absy() {
	AND(ReadByte_AbsoluteY());
}

void MOS6502::AND_imm() {
	AND(ReadByte_Immediate());
}

void MOS6502::AND_xind() {
	AND(ReadByte_IndexedIndirectX());
}

void MOS6502::AND_abs() {
	AND(ReadByte_Absolute());
}

void MOS6502::AND_zpind() {
	AND(ReadByte_ZeroPageIndirect());
}

//

void MOS6502::EOR_absx() {
	EOR(ReadByte_AbsoluteX());
}

void MOS6502::EOR_absy() {
	EOR(ReadByte_AbsoluteY());
}

void MOS6502::EOR_zpx() {
	EOR(ReadByte_ZeroPageX());
}

void MOS6502::EOR_indy() {
	EOR(ReadByte_IndirectIndexedY());
}

void MOS6502::EOR_abs() {
	EOR(ReadByte_Absolute());
}

void MOS6502::EOR_imm() {
	EOR(ReadByte_Immediate());
}

void MOS6502::EOR_zp() {
	EOR(ReadByte_ZeroPage());
}

void MOS6502::EOR_xind() {
	EOR(ReadByte_IndexedIndirectX());
}

void MOS6502::EOR_zpind() {
	EOR(ReadByte_ZeroPageIndirect());
}

//

void MOS6502::LDA_absx() {
	LDA(ReadByte_AbsoluteX());
}

void MOS6502::LDA_absy() {
	LDA(ReadByte_AbsoluteY());
}

void MOS6502::LDA_zpx() {
	LDA(ReadByte_ZeroPageX());
}

void MOS6502::LDA_indy() {
	LDA(ReadByte_IndirectIndexedY());
}

void MOS6502::LDA_abs() {
	LDA(ReadByte_Absolute());
}

void MOS6502::LDA_imm() {
	LDA(ReadByte_Immediate());
}

void MOS6502::LDA_zp() {
	LDA(ReadByte_ZeroPage());
}

void MOS6502::LDA_xind() {
	LDA(ReadByte_IndexedIndirectX());
}

void MOS6502::LDA_zpind() {
	LDA(ReadByte_ZeroPageIndirect());
}

//

void MOS6502::LDX_imm() {
	LDX(ReadByte_Immediate());
}

void MOS6502::LDX_zp() {
	LDX(ReadByte_ZeroPage());
}

void MOS6502::LDX_abs() {
	LDX(ReadByte_Absolute());
}

void MOS6502::LDX_zpy() {
	LDX(ReadByte_ZeroPageY());
}

void MOS6502::LDX_absy() {
	LDX(ReadByte_AbsoluteY());
}

//

void MOS6502::LDY_imm() {
	LDY(ReadByte_Immediate());
}

void MOS6502::LDY_zp() {
	LDY(ReadByte_ZeroPage());
}

void MOS6502::LDY_abs() {
	LDY(ReadByte_Absolute());
}

void MOS6502::LDY_zpx() {
	LDY(ReadByte_ZeroPageX());
}

void MOS6502::LDY_absx() {
	LDY(ReadByte_AbsoluteX());
}

//

void MOS6502::CMP_absx() {
	CMP(ReadByte_AbsoluteX());
}

void MOS6502::CMP_absy() {
	CMP(ReadByte_AbsoluteY());
}

void MOS6502::CMP_zpx() {
	CMP(ReadByte_ZeroPageX());
}

void MOS6502::CMP_indy() {
	CMP(ReadByte_IndirectIndexedY());
}

void MOS6502::CMP_abs() {
	CMP(ReadByte_Absolute());
}

void MOS6502::CMP_imm() {
	CMP(ReadByte_Immediate());
}

void MOS6502::CMP_zp() {
	CMP(ReadByte_ZeroPage());
}

void MOS6502::CMP_xind() {
	CMP(ReadByte_IndexedIndirectX());
}

void MOS6502::CMP_zpind() {
	CMP(ReadByte_ZeroPageIndirect());
}

//

void MOS6502::CPX_abs() {
	CPX(ReadByte_Absolute());
}

void MOS6502::CPX_zp() {
	CPX(ReadByte_ZeroPage());
}

void MOS6502::CPX_imm() {
	CPX(ReadByte_Immediate());
}

//

void MOS6502::CPY_imm() {
	CPY(ReadByte_Immediate());
}

void MOS6502::CPY_zp() {
	CPY(ReadByte_ZeroPage());
}

void MOS6502::CPY_abs() {
	CPY(ReadByte_Absolute());
}

//

void MOS6502::ADC_zp() {
	ADC(ReadByte_ZeroPage());
}

void MOS6502::ADC_xind() {
	ADC(ReadByte_IndexedIndirectX());
}

void MOS6502::ADC_imm() {
	ADC(ReadByte_Immediate());
}

void MOS6502::ADC_abs() {
	ADC(ReadByte_Absolute());
}

void MOS6502::ADC_zpx() {
	ADC(ReadByte_ZeroPageX());
}

void MOS6502::ADC_indy() {
	ADC(ReadByte_IndirectIndexedY());
}

void MOS6502::ADC_absx() {
	ADC(ReadByte_AbsoluteX());
}

void MOS6502::ADC_absy() {
	ADC(ReadByte_AbsoluteY());
}

void MOS6502::ADC_zpind() {
	ADC(ReadByte_ZeroPageIndirect());
}

//

void MOS6502::SBC_xind() {
	SBC(ReadByte_IndexedIndirectX());
}

void MOS6502::SBC_zp() {
	SBC(ReadByte_ZeroPage());
}

void MOS6502::SBC_imm() {
	SBC(ReadByte_Immediate());
}

void MOS6502::SBC_abs() {
	SBC(ReadByte_Absolute());
}

void MOS6502::SBC_zpx() {
	SBC(ReadByte_ZeroPageX());
}

void MOS6502::SBC_indy() {
	SBC(ReadByte_IndirectIndexedY());
}

void MOS6502::SBC_absx() {
	SBC(ReadByte_AbsoluteX());
}

void MOS6502::SBC_absy() {
	SBC(ReadByte_AbsoluteY());
}

void MOS6502::SBC_zpind() {
	SBC(ReadByte_ZeroPageIndirect());
}

//

void MOS6502::BIT_imm() {
	BIT_immediate(ReadByte_Immediate());
}

void MOS6502::BIT_zp() {
	BIT(ReadByte_ZeroPage());
}

void MOS6502::BIT_zpx() {
	BIT(ReadByte_ZeroPageX());
}

void MOS6502::BIT_abs() {
	BIT(ReadByte_Absolute());
}

void MOS6502::BIT_absx() {
	BIT(ReadByte_AbsoluteX());
}

//

void MOS6502::DEC_a() {
	UpdateZeroNegativeFlags(--a);
}

void MOS6502::DEC_absx() {
	DEC(Address_AbsoluteX_Write());
}

void MOS6502::DEC_zpx() {
	DEC(Address_ZeroPageX());
}

void MOS6502::DEC_abs() {
	DEC(Address_Absolute());
}

void MOS6502::DEC_zp() {
	DEC(Address_ZeroPage());
}

//

void MOS6502::DEX_imp() {
	UpdateZeroNegativeFlags(--x);
}

void MOS6502::DEY_imp() {
	UpdateZeroNegativeFlags(--y);
}

//

void MOS6502::INC_a() {
	UpdateZeroNegativeFlags(++a);
}

void MOS6502::INC_zp() {
	INC(Address_ZeroPage());
}

void MOS6502::INC_absx() {
	INC(Address_AbsoluteX_Write());
}

void MOS6502::INC_zpx() {
	INC(Address_ZeroPageX());
}

void MOS6502::INC_abs() {
	INC(Address_Absolute());
}

//

void MOS6502::INX_imp() {
	UpdateZeroNegativeFlags(++x);
}

void MOS6502::INY_imp() {
	UpdateZeroNegativeFlags(++y);
}

//

void MOS6502::STX_zpy() {
	WriteByte_ZeroPageY(x);
}

void MOS6502::STX_abs() {
	WriteByte_Absolute(x);
}

void MOS6502::STX_zp() {
	WriteByte_ZeroPage(x);
}

//

void MOS6502::STY_zpx() {
	WriteByte_ZeroPageX(y);
}

void MOS6502::STY_abs() {
	WriteByte_Absolute(y);
}

void MOS6502::STY_zp() {
	WriteByte_ZeroPage(y);
}

//

void MOS6502::STA_absx() {
	WriteByte_AbsoluteX(a);
}

void MOS6502::STA_absy() {
	WriteByte_AbsoluteY(a);
}

void MOS6502::STA_zpx() {
	WriteByte_ZeroPageX(a);
}

void MOS6502::STA_indy() {
	WriteByte_IndirectIndexedY(a);
}

void MOS6502::STA_abs() {
	WriteByte_Absolute(a);
}

void MOS6502::STA_zp() {
	WriteByte_ZeroPage(a);
}

void MOS6502::STA_xind() {
	WriteByte_IndexedIndirectX(a);
}

void MOS6502::STA_zpind() {
	WriteByte_ZeroPageIndirect(a);
}

//

void MOS6502::STZ_zp() {
	WriteByte_ZeroPage((uint8_t)0);
}

void MOS6502::STZ_zpx() {
	WriteByte_ZeroPageX((uint8_t)0);
}

void MOS6502::STZ_abs() {
	WriteByte_Absolute((uint8_t)0);
}

void MOS6502::STZ_absx() {
	WriteByte_AbsoluteX((uint8_t)0);
}

//

void MOS6502::TSX_imp() {
	x = s;
	UpdateZeroNegativeFlags(x);
}

void MOS6502::TAX_imp() {
	x = a;
	UpdateZeroNegativeFlags(x);
}

void MOS6502::TAY_imp() {
	y = a;
	UpdateZeroNegativeFlags(y);
}

void MOS6502::TXS_imp() {
	s = x;
}

void MOS6502::TYA_imp() {
	a = y;
	UpdateZeroNegativeFlags(a);
}

void MOS6502::TXA_imp() {
	a = x;
	UpdateZeroNegativeFlags(a);
}

//

void MOS6502::PHP_imp() {
	p.brk = true;
	PushByte(p);
}

void MOS6502::PLP_imp() {
	p = PopByte();
	p.reserved = true;
}

void MOS6502::PLA_imp() {
	a = PopByte();
	UpdateZeroNegativeFlags(a);
}

void MOS6502::PHA_imp() {
	PushByte(a);
}

void MOS6502::PHX_imp() {
	PushByte(x);
}

void MOS6502::PHY_imp() {
	PushByte(y);
}

void MOS6502::PLX_imp() {
	x = PopByte();
	UpdateZeroNegativeFlags(x);
}

void MOS6502::PLY_imp() {
	y = PopByte();
	UpdateZeroNegativeFlags(y);
}

//

void MOS6502::ASL_a() {
	a = ASL(a);
}

void MOS6502::ASL_zp() {
	ASL(Address_ZeroPage());
}

void MOS6502::ASL_abs() {
	ASL(Address_Absolute());
}

void MOS6502::ASL_absx() {
	ASL(Address_AbsoluteX_Write());
}

void MOS6502::ASL_zpx() {
	ASL(Address_ZeroPageX());
}

//

void MOS6502::LSR_absx() {
	LSR(Address_AbsoluteX_Write());
}

void MOS6502::LSR_zpx() {
	LSR(Address_ZeroPageX());
}

void MOS6502::LSR_abs() {
	LSR(Address_Absolute());
}

void MOS6502::LSR_a() {
	a = LSR(a);
}

void MOS6502::LSR_zp() {
	LSR(Address_ZeroPage());
}

//

void MOS6502::ROL_absx() {
	ROL(Address_AbsoluteX_Write());
}

void MOS6502::ROL_zpx() {
	ROL(Address_ZeroPageX());
}

void MOS6502::ROL_abs() {
	ROL(Address_Absolute());
}

void MOS6502::ROL_a() {
	a = ROL(a);
}

void MOS6502::ROL_zp() {
	ROL(Address_ZeroPage());
}

//

void MOS6502::ROR_absx() {
	ROR(Address_AbsoluteX_Write());
}

void MOS6502::ROR_zpx() {
	ROR(Address_ZeroPageX());
}

void MOS6502::ROR_abs() {
	ROR(Address_Absolute());
}

void MOS6502::ROR_a() {
	a = ROR(a);
}

void MOS6502::ROR_zp() {
	ROR(Address_ZeroPage());
}

//

void MOS6502::TSB_zp() {
	TSB(Address_ZeroPage());
}

void MOS6502::TSB_abs() {
	TSB(Address_Absolute());
}

//

void MOS6502::TRB_zp() {
	TRB(Address_ZeroPage());
}

void MOS6502::TRB_abs() {
	TRB(Address_Absolute());
}

//

void MOS6502::RMB0_zp() {
	RMB(Address_ZeroPage(), 1);
}

void MOS6502::RMB1_zp() {
	RMB(Address_ZeroPage(), 2);
}

void MOS6502::RMB2_zp() {
	RMB(Address_ZeroPage(), 4);
}

void MOS6502::RMB3_zp() {
	RMB(Address_ZeroPage(), 8);
}

void MOS6502::RMB4_zp() {
	RMB(Address_ZeroPage(), 0x10);
}

void MOS6502::RMB5_zp() {
	RMB(Address_ZeroPage(), 0x20);
}

void MOS6502::RMB6_zp() {
	RMB(Address_ZeroPage(), 0x40);
}

void MOS6502::RMB7_zp() {
	RMB(Address_ZeroPage(), 0x80);
}

//

void MOS6502::SMB0_zp() {
	SMB(Address_ZeroPage(), 1);
}

void MOS6502::SMB1_zp() {
	SMB(Address_ZeroPage(), 2);
}

void MOS6502::SMB2_zp() {
	SMB(Address_ZeroPage(), 4);
}

void MOS6502::SMB3_zp() {
	SMB(Address_ZeroPage(), 8);
}

void MOS6502::SMB4_zp() {
	SMB(Address_ZeroPage(), 0x10);
}

void MOS6502::SMB5_zp() {
	SMB(Address_ZeroPage(), 0x20);
}

void MOS6502::SMB6_zp() {
	SMB(Address_ZeroPage(), 0x40);
}

void MOS6502::SMB7_zp() {
	SMB(Address_ZeroPage(), 0x80);
}

//

void MOS6502::JSR_abs() {
	auto destination = Address_Absolute();
	PushWord((uint16_t)(pc - 1));
	pc = destination;
}

void MOS6502::RTI_imp() {
	PLP_imp();
	pc = PopWord();
}

void MOS6502::RTS_imp() {
	pc = (uint16_t)(PopWord() + 1);
}

void MOS6502::JMP_abs() {
	pc = Address_Absolute();
}

void MOS6502::JMP_ind() {
	pc = GetWord(Address_Absolute());
}

void MOS6502::JMP_absxind() {
	pc = Address_AbsoluteXIndirect();
}

void MOS6502::BRK_imp() {
	PushWord((uint16_t)(pc + 1));
	PHP_imp();
	p.interrupt = true;
	if (level >= ProcessorType::Cpu65SC02)
		p.decimal = false;

	pc = GetWord(IRQvector);
}

//

void MOS6502::WAI_imp() {
	throw std::runtime_error("Not implemented");
}

void MOS6502::STP_imp() {
	throw std::runtime_error("Not implemented");
}

//

void MOS6502::SED_imp() {
	p.decimal = true;
}

void MOS6502::CLD_imp() {
	p.decimal = false;
}

void MOS6502::CLV_imp() {
	p.overflow = false;
}

void MOS6502::SEI_imp() {
	p.interrupt = true;
}

void MOS6502::CLI_imp() {
	p.interrupt = false;
}

void MOS6502::CLC_imp() {
	p.carry = false;
}

void MOS6502::SEC_imp() {
	p.carry = true;
}

//

void MOS6502::BMI_rel() {
	Branch(p.negative);
}

void MOS6502::BPL_rel() {
	Branch(!p.negative);
}

void MOS6502::BVC_rel() {
	Branch(!p.overflow);
}

void MOS6502::BVS_rel() {
	Branch(p.overflow);
}

void MOS6502::BCC_rel() {
	Branch(!p.carry);
}

void MOS6502::BCS_rel() {
	Branch(p.carry);
}

void MOS6502::BNE_rel() {
	Branch(!p.zero);
}

void MOS6502::BEQ_rel() {
	Branch(p.zero);
}

void MOS6502::BRA_rel() {
	Branch();
}

//

void MOS6502::BBR0_zprel() {
	BitBranch_Clear(0x1);
}

void MOS6502::BBR1_zprel() {
	BitBranch_Clear(0x2);
}

void MOS6502::BBR2_zprel() {
	BitBranch_Clear(0x4);
}

void MOS6502::BBR3_zprel() {
	BitBranch_Clear(0x8);
}

void MOS6502::BBR4_zprel() {
	BitBranch_Clear(0x10);
}

void MOS6502::BBR5_zprel() {
	BitBranch_Clear(0x20);
}

void MOS6502::BBR6_zprel() {
	BitBranch_Clear(0x40);
}

void MOS6502::BBR7_zprel() {
	BitBranch_Clear(0x80);
}

void MOS6502::BBS0_zprel() {
	BitBranch_Set(0x1);
}

void MOS6502::BBS1_zprel() {
	BitBranch_Set(0x2);
}

void MOS6502::BBS2_zprel() {
	BitBranch_Set(0x4);
}

void MOS6502::BBS3_zprel() {
	BitBranch_Set(0x8);
}

void MOS6502::BBS4_zprel() {
	BitBranch_Set(0x10);
}

void MOS6502::BBS5_zprel() {
	BitBranch_Set(0x20);
}

void MOS6502::BBS6_zprel() {
	BitBranch_Set(0x40);
}

void MOS6502::BBS7_zprel() {
	BitBranch_Set(0x80);
}
