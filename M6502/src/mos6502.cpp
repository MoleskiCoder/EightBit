#include "stdafx.h"
#include "mos6502.h"

EightBit::MOS6502::MOS6502(Memory& memory, ProcessorType cpuLevel)
: Processor(memory),
  level(cpuLevel) {
	Install6502Instructions();
	Install65sc02Instructions();
	Install65c02Instructions();
}

void EightBit::MOS6502::initialise() {

	Processor::initialise();

	PC().word = 0;
	x = 0x80;
	y = 0;
	a = 0;

	p = 0;
	p.reserved = true;

	s = 0xff;
}

int EightBit::MOS6502::step() {
	ExecutingInstruction.fire(*this);
	auto returned = Execute(FetchByte());
	ExecutedInstruction.fire(*this);
	return returned;
}

void EightBit::MOS6502::Reset() {
	GetWord(RSTvector, PC());
}

void EightBit::MOS6502::TriggerIRQ() {
	Interrupt(IRQvector);
}

void EightBit::MOS6502::TriggerNMI() {
	Interrupt(NMIvector);
}

void EightBit::MOS6502::GetWord(register16_t& output) {
	output.low = GetByte();
	m_memory.ADDRESS().word++;
	output.high = GetByte();
}

void EightBit::MOS6502::GetWord(uint16_t offset, register16_t& output) {
	m_memory.ADDRESS().word = offset;
	GetWord(output);
}

void EightBit::MOS6502::Interrupt(uint16_t vector) {
	PushWord(PC());
	PushByte(p);
	p.interrupt = true;
	GetWord(vector, PC());
}

int EightBit::MOS6502::Execute(uint8_t cell) {
	cycles = 0;
	const auto& instruction = instructions[cell];
	const auto& method = instruction.vector;
	method();
	cycles += instruction.count;
	return cycles;
}

void EightBit::MOS6502::___() {
	if (level >= ProcessorType::Cpu65SC02) {
		// Generally, missing instructions act as a one byte,
		// one cycle NOP instruction on 65c02 (ish) processors.
		NOP_imp();
		cycles++;
	} else {
		throw new std::domain_error("Whoops: Invalid instruction.");
	}
}

EightBit::MOS6502::Instruction EightBit::MOS6502::INS(instruction_t method, int cycles, AddressingMode addressing, std::string display) {
	MOS6502::Instruction returnValue;
	returnValue.vector = method;
	returnValue.count = cycles;
	returnValue.mode = addressing;
	returnValue.display = display;
	return returnValue;
}

////

#define BIND(method)	std::bind(&MOS6502:: method, this)

void EightBit::MOS6502::Install6502Instructions() {
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

void EightBit::MOS6502::Install65sc02Instructions() {
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

void EightBit::MOS6502::Install65c02Instructions() {
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

void EightBit::MOS6502::InstallInstructionSet(const std::array<Instruction, 0x100>& basis) {
	OverlayInstructionSet(basis, true);
}

void EightBit::MOS6502::OverlayInstructionSet(const std::array<Instruction, 0x100>& overlay) {
	OverlayInstructionSet(overlay, false);
}

void EightBit::MOS6502::OverlayInstructionSet(const std::array<Instruction, 0x100>& overlay, bool includeIllegal) {
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

void EightBit::MOS6502::PushByte(uint8_t value) {
	SetByte(PageOne + s--, value);
}

uint8_t EightBit::MOS6502::PopByte() {
	return GetByte(PageOne + ++s);
}

void EightBit::MOS6502::PushWord(register16_t value) {
	PushByte(value.high);
	PushByte(value.low);
}

void EightBit::MOS6502::PopWord(register16_t& output) {
	output.low = PopByte();
	output.high = PopByte();
}

uint8_t EightBit::MOS6502::FetchByte() {
	m_memory.ADDRESS().word = PC().word++;
	return GetByte();
}

void EightBit::MOS6502::FetchWord(register16_t& output) {
	m_memory.ADDRESS().word = PC().word++;
	GetWord(output);
	PC().word++;
}

////

void EightBit::MOS6502::Address_ZeroPage(register16_t& output) {
	output.low = FetchByte();
	output.high = 0;
}

void EightBit::MOS6502::Address_ZeroPageX(register16_t& output) {
	output.low = FetchByte() + x;
	output.high = 0;
}

void EightBit::MOS6502::Address_ZeroPageY(register16_t& output) {
	output.low = FetchByte() + y;
	output.high = 0;
}

void EightBit::MOS6502::Address_IndexedIndirectX(register16_t& output) {
	register16_t zp;
	Address_ZeroPageX(zp);
	return GetWord(zp.word, output);
}

void EightBit::MOS6502::Address_IndexedIndirectY_Read(register16_t& output) {
	GetWord(FetchByte(), output);
	if (output.low == 0xff)
		++cycles;
	output.word += y;
}

void EightBit::MOS6502::Address_IndexedIndirectY_Write(register16_t& output) {
	GetWord(FetchByte(), output);
	output.word += y;
}

void EightBit::MOS6502::Address_Absolute(register16_t& output) {
	FetchWord(output);
}

void EightBit::MOS6502::Address_AbsoluteXIndirect(register16_t& output) {
	register16_t address;
	FetchWord(address);
	GetWord(address.word + x, output);
}

void EightBit::MOS6502::Address_AbsoluteX_Read(register16_t& output) {
	FetchWord(output);
	output.word += x;
	if (output.low == 0xff)
		++cycles;
}

void EightBit::MOS6502::Address_AbsoluteX_Write(register16_t& output) {
	FetchWord(output);
	output.word += x;
}

void EightBit::MOS6502::Address_AbsoluteY_Read(register16_t& output) {
	FetchWord(output);
	output.word += y;
	if (output.low == 0xff)
		++cycles;
}

void EightBit::MOS6502::Address_AbsoluteY_Write(register16_t& output) {
	FetchWord(output);
	output.word += y;
}

void EightBit::MOS6502::Address_ZeroPageIndirect(register16_t& output) {
	GetWord(FetchByte(), output);
}

////

uint8_t EightBit::MOS6502::ReadByte_Immediate() {
	return FetchByte();
}

int8_t EightBit::MOS6502::ReadByte_ImmediateDisplacement() {
	return FetchByte();
}

uint8_t EightBit::MOS6502::ReadByte_ZeroPage() {
	register16_t address;
	Address_ZeroPage(address);
	return GetByte(address.word);
}

uint8_t EightBit::MOS6502::ReadByte_ZeroPageX() {
	register16_t address;
	Address_ZeroPageX(address);
	return GetByte(address.word);
}

uint8_t EightBit::MOS6502::ReadByte_ZeroPageY() {
	register16_t address;
	Address_ZeroPageY(address);
	return GetByte(address.word);
}

uint8_t EightBit::MOS6502::ReadByte_Absolute() {
	register16_t address;
	Address_Absolute(address);
	return GetByte(address.word);
}

uint8_t EightBit::MOS6502::ReadByte_AbsoluteX() {
	register16_t address;
	Address_AbsoluteX_Read(address);
	return GetByte(address.word);
}

uint8_t EightBit::MOS6502::ReadByte_AbsoluteY() {
	register16_t address;
	Address_AbsoluteY_Read(address);
	return GetByte(address.word);
}

uint8_t EightBit::MOS6502::ReadByte_IndexedIndirectX() {
	register16_t address;
	Address_IndexedIndirectX(address);
	return GetByte(address.word);
}

uint8_t EightBit::MOS6502::ReadByte_IndirectIndexedY() {
	register16_t address;
	Address_IndexedIndirectY_Read(address);
	return GetByte(address.word);
}

uint8_t EightBit::MOS6502::ReadByte_ZeroPageIndirect() {
	register16_t address;
	Address_ZeroPageIndirect(address);
	return GetByte(address.word);
}

////

void EightBit::MOS6502::WriteByte_ZeroPage(uint8_t value) {
	register16_t address;
	Address_ZeroPage(address);
	SetByte(address.word, value);
}

void EightBit::MOS6502::WriteByte_Absolute(uint8_t value) {
	register16_t address;
	Address_Absolute(address);
	SetByte(address.word, value);
}

void EightBit::MOS6502::WriteByte_AbsoluteX(uint8_t value) {
	register16_t address;
	Address_AbsoluteX_Write(address);
	SetByte(address.word, value);
}

void EightBit::MOS6502::WriteByte_AbsoluteY(uint8_t value) {
	register16_t address;
	Address_AbsoluteY_Write(address);
	SetByte(address.word, value);
}

void EightBit::MOS6502::WriteByte_ZeroPageX(uint8_t value) {
	register16_t address;
	Address_ZeroPageX(address);
	SetByte(address.word, value);
}

void EightBit::MOS6502::WriteByte_ZeroPageY(uint8_t value) {
	register16_t address;
	Address_ZeroPageY(address);
	SetByte(address.word, value);
}

void EightBit::MOS6502::WriteByte_IndirectIndexedY(uint8_t value) {
	register16_t address;
	Address_IndexedIndirectY_Write(address);
	SetByte(address.word, value);
}

void EightBit::MOS6502::WriteByte_IndexedIndirectX(uint8_t value) {
	register16_t address;
	Address_IndexedIndirectX(address);
	SetByte(address.word, value);
}

void EightBit::MOS6502::WriteByte_ZeroPageIndirect(uint8_t value) {
	register16_t address;
	Address_ZeroPageIndirect(address);
	SetByte(address.word, value);
}

////

void EightBit::MOS6502::DEC(uint16_t offset) {
	auto content = GetByte(offset);
	SetByte(offset, --content);
	UpdateZeroNegativeFlags(content);
}

uint8_t EightBit::MOS6502::ROR(uint8_t data) {
	auto carry = p.carry;

	p.carry = (data & 1) != 0;

	auto result = (uint8_t)(data >> 1);
	if (carry)
		result |= 0x80;

	UpdateZeroNegativeFlags(result);

	return result;
}

void EightBit::MOS6502::ROR(uint16_t offset) {
	SetByte(offset, ROR(GetByte(offset)));
}

uint8_t EightBit::MOS6502::LSR(uint8_t data) {
	p.carry = (data & 1) != 0;

	auto result = (uint8_t)(data >> 1);

	UpdateZeroNegativeFlags(result);

	return result;
}

void EightBit::MOS6502::LSR(uint16_t offset) {
	SetByte(offset, LSR(GetByte(offset)));
}

void EightBit::MOS6502::BIT_immediate(uint8_t data) {
	auto result = (uint8_t)(a & data);
	UpdateZeroFlag(result);
}

void EightBit::MOS6502::BIT(uint8_t data) {
	BIT_immediate(data);
	p.negative = (data & 0x80) != 0;
	p.overflow = (data & 0x40) != 0;
}

void EightBit::MOS6502::TSB(uint16_t address) {
	auto content = GetByte(address);
	BIT_immediate(content);
	uint8_t result = content | a;
	SetByte(address, result);
}

void EightBit::MOS6502::TRB(uint16_t address) {
	auto content = GetByte(address);
	BIT_immediate(content);
	uint8_t result = content & ~a;
	SetByte(address, result);
}

void EightBit::MOS6502::INC(uint16_t offset) {
	auto content = GetByte(offset);
	SetByte(offset, ++content);
	UpdateZeroNegativeFlags(content);
}

void EightBit::MOS6502::ROL(uint16_t offset) {
	SetByte(offset, ROL(GetByte(offset)));
}

uint8_t EightBit::MOS6502::ROL(uint8_t data) {
	auto carry = p.carry;

	p.carry = (data & 0x80) != 0;

	uint8_t result = data << 1;

	if (carry)
		result |= 1;

	UpdateZeroNegativeFlags(result);

	return result;
}

void EightBit::MOS6502::ASL(uint16_t offset) {
	SetByte(offset, ASL(GetByte(offset)));
}

uint8_t EightBit::MOS6502::ASL(uint8_t data) {
	uint8_t result = data << 1;
	UpdateZeroNegativeFlags(result);
	p.carry = (data & 0x80) != 0;
	return result;
}

void EightBit::MOS6502::ORA(uint8_t data) {
	a |= data;
	UpdateZeroNegativeFlags(a);
}

void EightBit::MOS6502::AND(uint8_t data) {
	a &= data;
	UpdateZeroNegativeFlags(a);
}

void EightBit::MOS6502::SBC(uint8_t data) {
	if (p.decimal)
		SBC_d(data);
	else
		SBC_b(data);
}

void EightBit::MOS6502::SBC_b(uint8_t data) {
	auto carry = p.carry ? 0 : 1;

	register16_t difference;
	difference.word = a - data - carry;

	UpdateZeroNegativeFlags(difference.low);
	p.overflow = ((a ^ data) & (a ^ difference.low) & 0x80) != 0;
	p.carry = difference.high == 0;

	a = difference.low;
}

void EightBit::MOS6502::SBC_d(uint8_t data) {
	auto carry = p.carry ? 0 : 1;

	register16_t difference;
	difference.word = a - data - carry;

	if (level < ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(difference.low);

	p.overflow = ((a ^ data) & (a ^ difference.low) & 0x80) != 0;
	p.carry = difference.high == 0;

	auto low = (uint8_t)(lowNibble(a) - lowNibble(data) - carry);

	auto lowNegative = (int8_t)low < 0;
	if (lowNegative)
		low -= 6;

	uint8_t high = highNibble(a) - highNibble(data) - (lowNegative ? 1 : 0);

	if ((int8_t)high < 0)
		high -= 6;

	a = promoteNibble(high) | lowNibble(low);
	if (level >= ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(a);
}

void EightBit::MOS6502::EOR(uint8_t data) {
	a ^= data;
	UpdateZeroNegativeFlags(a);
}

void EightBit::MOS6502::CPX(uint8_t data) {
	CMP(x, data);
}

void EightBit::MOS6502::CPY(uint8_t data) {
	CMP(y, data);
}

void EightBit::MOS6502::CMP(uint8_t data) {
	CMP(a, data);
}

void EightBit::MOS6502::CMP(uint8_t first, uint8_t second) {
	register16_t result;
	result.word = first - second;
	UpdateZeroNegativeFlags(result.low);
	p.carry = result.high == 0;
}

void EightBit::MOS6502::LDA(uint8_t data) {
	a = data;
	UpdateZeroNegativeFlags(a);
}

void EightBit::MOS6502::LDY(uint8_t data) {
	y = data;
	UpdateZeroNegativeFlags(y);
}

void EightBit::MOS6502::LDX(uint8_t data) {
	x = data;
	UpdateZeroNegativeFlags(x);
}

void EightBit::MOS6502::ADC(uint8_t data) {
	if (p.decimal)
		ADC_d(data);
	else
		ADC_b(data);
}

void EightBit::MOS6502::ADC_b(uint8_t data) {
	auto carry = (uint8_t)(p.carry ? 1 : 0);

	register16_t sum;
	sum.word = a + data + carry;

	UpdateZeroNegativeFlags(sum.low);
	p.overflow = (~(a ^ data) & (a ^ sum.low) & 0x80) != 0;
	p.carry = sum.high != 0;

	a = sum.low;
}

void EightBit::MOS6502::ADC_d(uint8_t data) {
	auto carry = (uint8_t)(p.carry ? 1 : 0);

	register16_t sum;
	sum.word = a + data + carry;

	if (level < ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(sum.low);

	auto low = (uint8_t)(lowNibble(a) + lowNibble(data) + carry);
	if (low > 9)
		low += 6;

	auto high = (uint8_t)(highNibble(a) + highNibble(data) + (low > 0xf ? 1 : 0));
	p.overflow = (~(a ^ data) & (a ^ promoteNibble(high)) & 0x80) != 0;

	if (high > 9)
		high += 6;

	p.carry = high > 0xf;

	a = (uint8_t)(promoteNibble(high) | lowNibble(low));
	if (level >= ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(a);
}

////

void EightBit::MOS6502::RMB(uint16_t address, uint8_t flag) {
	auto data = GetByte(address);
	data &= (uint8_t)~flag;
	SetByte(address, data);
}

void EightBit::MOS6502::SMB(uint16_t address, uint8_t flag) {
	auto data = GetByte(address);
	data |= flag;
	SetByte(address, data);
}

////

void EightBit::MOS6502::Branch(int8_t displacement) {
	++cycles;
	auto oldPage = PC().high;
	PC().word += displacement;
	auto newPage = PC().high;
	if (oldPage != newPage)
		cycles += 2;
}

void EightBit::MOS6502::Branch() {
	auto displacement = ReadByte_ImmediateDisplacement();
	Branch(displacement);
}

void EightBit::MOS6502::Branch(bool flag) {
	auto displacement = ReadByte_ImmediateDisplacement();
	if (flag)
		Branch(displacement);
}

void EightBit::MOS6502::BitBranch_Clear(uint8_t check) {
	auto zp = FetchByte();
	auto contents = GetByte(zp);
	auto displacement = FetchByte();
	if ((contents & check) == 0)
		PC().word += displacement;
}

void EightBit::MOS6502::BitBranch_Set(uint8_t check) {
	auto zp = FetchByte();
	auto contents = GetByte(zp);
	auto displacement = FetchByte();
	if ((contents & check) != 0)
		PC().word += displacement;
}

//

void EightBit::MOS6502::NOP_imp() {
}

void EightBit::MOS6502::NOP2_imp() {
	FetchByte();
}

void EightBit::MOS6502::NOP3_imp() {
	register16_t discarded;
	FetchWord(discarded);
}

//

void EightBit::MOS6502::ORA_xind() {
	ORA(ReadByte_IndexedIndirectX());
}

void EightBit::MOS6502::ORA_zp() {
	ORA(ReadByte_ZeroPage());
}

void EightBit::MOS6502::ORA_imm() {
	ORA(ReadByte_Immediate());
}

void EightBit::MOS6502::ORA_abs() {
	ORA(ReadByte_Absolute());
}

void EightBit::MOS6502::ORA_absx() {
	ORA(ReadByte_AbsoluteX());
}

void EightBit::MOS6502::ORA_absy() {
	ORA(ReadByte_AbsoluteY());
}

void EightBit::MOS6502::ORA_zpx() {
	ORA(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::ORA_indy() {
	ORA(ReadByte_IndirectIndexedY());
}

void EightBit::MOS6502::ORA_zpind() {
	ORA(ReadByte_ZeroPageIndirect());
}

//

void EightBit::MOS6502::AND_zpx() {
	AND(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::AND_indy() {
	AND(ReadByte_IndirectIndexedY());
}

void EightBit::MOS6502::AND_zp() {
	AND(ReadByte_ZeroPage());
}

void EightBit::MOS6502::AND_absx() {
	AND(ReadByte_AbsoluteX());
}

void EightBit::MOS6502::AND_absy() {
	AND(ReadByte_AbsoluteY());
}

void EightBit::MOS6502::AND_imm() {
	AND(ReadByte_Immediate());
}

void EightBit::MOS6502::AND_xind() {
	AND(ReadByte_IndexedIndirectX());
}

void EightBit::MOS6502::AND_abs() {
	AND(ReadByte_Absolute());
}

void EightBit::MOS6502::AND_zpind() {
	AND(ReadByte_ZeroPageIndirect());
}

//

void EightBit::MOS6502::EOR_absx() {
	EOR(ReadByte_AbsoluteX());
}

void EightBit::MOS6502::EOR_absy() {
	EOR(ReadByte_AbsoluteY());
}

void EightBit::MOS6502::EOR_zpx() {
	EOR(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::EOR_indy() {
	EOR(ReadByte_IndirectIndexedY());
}

void EightBit::MOS6502::EOR_abs() {
	EOR(ReadByte_Absolute());
}

void EightBit::MOS6502::EOR_imm() {
	EOR(ReadByte_Immediate());
}

void EightBit::MOS6502::EOR_zp() {
	EOR(ReadByte_ZeroPage());
}

void EightBit::MOS6502::EOR_xind() {
	EOR(ReadByte_IndexedIndirectX());
}

void EightBit::MOS6502::EOR_zpind() {
	EOR(ReadByte_ZeroPageIndirect());
}

//

void EightBit::MOS6502::LDA_absx() {
	LDA(ReadByte_AbsoluteX());
}

void EightBit::MOS6502::LDA_absy() {
	LDA(ReadByte_AbsoluteY());
}

void EightBit::MOS6502::LDA_zpx() {
	LDA(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::LDA_indy() {
	LDA(ReadByte_IndirectIndexedY());
}

void EightBit::MOS6502::LDA_abs() {
	LDA(ReadByte_Absolute());
}

void EightBit::MOS6502::LDA_imm() {
	LDA(ReadByte_Immediate());
}

void EightBit::MOS6502::LDA_zp() {
	LDA(ReadByte_ZeroPage());
}

void EightBit::MOS6502::LDA_xind() {
	LDA(ReadByte_IndexedIndirectX());
}

void EightBit::MOS6502::LDA_zpind() {
	LDA(ReadByte_ZeroPageIndirect());
}

//

void EightBit::MOS6502::LDX_imm() {
	LDX(ReadByte_Immediate());
}

void EightBit::MOS6502::LDX_zp() {
	LDX(ReadByte_ZeroPage());
}

void EightBit::MOS6502::LDX_abs() {
	LDX(ReadByte_Absolute());
}

void EightBit::MOS6502::LDX_zpy() {
	LDX(ReadByte_ZeroPageY());
}

void EightBit::MOS6502::LDX_absy() {
	LDX(ReadByte_AbsoluteY());
}

//

void EightBit::MOS6502::LDY_imm() {
	LDY(ReadByte_Immediate());
}

void EightBit::MOS6502::LDY_zp() {
	LDY(ReadByte_ZeroPage());
}

void EightBit::MOS6502::LDY_abs() {
	LDY(ReadByte_Absolute());
}

void EightBit::MOS6502::LDY_zpx() {
	LDY(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::LDY_absx() {
	LDY(ReadByte_AbsoluteX());
}

//

void EightBit::MOS6502::CMP_absx() {
	CMP(ReadByte_AbsoluteX());
}

void EightBit::MOS6502::CMP_absy() {
	CMP(ReadByte_AbsoluteY());
}

void EightBit::MOS6502::CMP_zpx() {
	CMP(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::CMP_indy() {
	CMP(ReadByte_IndirectIndexedY());
}

void EightBit::MOS6502::CMP_abs() {
	CMP(ReadByte_Absolute());
}

void EightBit::MOS6502::CMP_imm() {
	CMP(ReadByte_Immediate());
}

void EightBit::MOS6502::CMP_zp() {
	CMP(ReadByte_ZeroPage());
}

void EightBit::MOS6502::CMP_xind() {
	CMP(ReadByte_IndexedIndirectX());
}

void EightBit::MOS6502::CMP_zpind() {
	CMP(ReadByte_ZeroPageIndirect());
}

//

void EightBit::MOS6502::CPX_abs() {
	CPX(ReadByte_Absolute());
}

void EightBit::MOS6502::CPX_zp() {
	CPX(ReadByte_ZeroPage());
}

void EightBit::MOS6502::CPX_imm() {
	CPX(ReadByte_Immediate());
}

//

void EightBit::MOS6502::CPY_imm() {
	CPY(ReadByte_Immediate());
}

void EightBit::MOS6502::CPY_zp() {
	CPY(ReadByte_ZeroPage());
}

void EightBit::MOS6502::CPY_abs() {
	CPY(ReadByte_Absolute());
}

//

void EightBit::MOS6502::ADC_zp() {
	ADC(ReadByte_ZeroPage());
}

void EightBit::MOS6502::ADC_xind() {
	ADC(ReadByte_IndexedIndirectX());
}

void EightBit::MOS6502::ADC_imm() {
	ADC(ReadByte_Immediate());
}

void EightBit::MOS6502::ADC_abs() {
	ADC(ReadByte_Absolute());
}

void EightBit::MOS6502::ADC_zpx() {
	ADC(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::ADC_indy() {
	ADC(ReadByte_IndirectIndexedY());
}

void EightBit::MOS6502::ADC_absx() {
	ADC(ReadByte_AbsoluteX());
}

void EightBit::MOS6502::ADC_absy() {
	ADC(ReadByte_AbsoluteY());
}

void EightBit::MOS6502::ADC_zpind() {
	ADC(ReadByte_ZeroPageIndirect());
}

//

void EightBit::MOS6502::SBC_xind() {
	SBC(ReadByte_IndexedIndirectX());
}

void EightBit::MOS6502::SBC_zp() {
	SBC(ReadByte_ZeroPage());
}

void EightBit::MOS6502::SBC_imm() {
	SBC(ReadByte_Immediate());
}

void EightBit::MOS6502::SBC_abs() {
	SBC(ReadByte_Absolute());
}

void EightBit::MOS6502::SBC_zpx() {
	SBC(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::SBC_indy() {
	SBC(ReadByte_IndirectIndexedY());
}

void EightBit::MOS6502::SBC_absx() {
	SBC(ReadByte_AbsoluteX());
}

void EightBit::MOS6502::SBC_absy() {
	SBC(ReadByte_AbsoluteY());
}

void EightBit::MOS6502::SBC_zpind() {
	SBC(ReadByte_ZeroPageIndirect());
}

//

void EightBit::MOS6502::BIT_imm() {
	BIT_immediate(ReadByte_Immediate());
}

void EightBit::MOS6502::BIT_zp() {
	BIT(ReadByte_ZeroPage());
}

void EightBit::MOS6502::BIT_zpx() {
	BIT(ReadByte_ZeroPageX());
}

void EightBit::MOS6502::BIT_abs() {
	BIT(ReadByte_Absolute());
}

void EightBit::MOS6502::BIT_absx() {
	BIT(ReadByte_AbsoluteX());
}

//

void EightBit::MOS6502::DEC_a() {
	UpdateZeroNegativeFlags(--a);
}

void EightBit::MOS6502::DEC_absx() {
	register16_t address;
	Address_AbsoluteX_Write(address);
	DEC(address.word);
}

void EightBit::MOS6502::DEC_zpx() {
	register16_t address;
	Address_ZeroPageX(address);
	DEC(address.word);
}

void EightBit::MOS6502::DEC_abs() {
	register16_t address;
	Address_Absolute(address);
	DEC(address.word);
}

void EightBit::MOS6502::DEC_zp() {
	register16_t address;
	Address_ZeroPage(address);
	DEC(address.word);
}

//

void EightBit::MOS6502::DEX_imp() {
	UpdateZeroNegativeFlags(--x);
}

void EightBit::MOS6502::DEY_imp() {
	UpdateZeroNegativeFlags(--y);
}

//

void EightBit::MOS6502::INC_a() {
	UpdateZeroNegativeFlags(++a);
}

void EightBit::MOS6502::INC_zp() {
	register16_t address;
	Address_ZeroPage(address);
	INC(address.word);
}

void EightBit::MOS6502::INC_absx() {
	register16_t address;
	Address_AbsoluteX_Write(address);
	INC(address.word);
}

void EightBit::MOS6502::INC_zpx() {
	register16_t address;
	Address_ZeroPageX(address);
	INC(address.word);
}

void EightBit::MOS6502::INC_abs() {
	register16_t address;
	Address_Absolute(address);
	INC(address.word);
}

//

void EightBit::MOS6502::INX_imp() {
	UpdateZeroNegativeFlags(++x);
}

void EightBit::MOS6502::INY_imp() {
	UpdateZeroNegativeFlags(++y);
}

//

void EightBit::MOS6502::STX_zpy() {
	WriteByte_ZeroPageY(x);
}

void EightBit::MOS6502::STX_abs() {
	WriteByte_Absolute(x);
}

void EightBit::MOS6502::STX_zp() {
	WriteByte_ZeroPage(x);
}

//

void EightBit::MOS6502::STY_zpx() {
	WriteByte_ZeroPageX(y);
}

void EightBit::MOS6502::STY_abs() {
	WriteByte_Absolute(y);
}

void EightBit::MOS6502::STY_zp() {
	WriteByte_ZeroPage(y);
}

//

void EightBit::MOS6502::STA_absx() {
	WriteByte_AbsoluteX(a);
}

void EightBit::MOS6502::STA_absy() {
	WriteByte_AbsoluteY(a);
}

void EightBit::MOS6502::STA_zpx() {
	WriteByte_ZeroPageX(a);
}

void EightBit::MOS6502::STA_indy() {
	WriteByte_IndirectIndexedY(a);
}

void EightBit::MOS6502::STA_abs() {
	WriteByte_Absolute(a);
}

void EightBit::MOS6502::STA_zp() {
	WriteByte_ZeroPage(a);
}

void EightBit::MOS6502::STA_xind() {
	WriteByte_IndexedIndirectX(a);
}

void EightBit::MOS6502::STA_zpind() {
	WriteByte_ZeroPageIndirect(a);
}

//

void EightBit::MOS6502::STZ_zp() {
	WriteByte_ZeroPage((uint8_t)0);
}

void EightBit::MOS6502::STZ_zpx() {
	WriteByte_ZeroPageX((uint8_t)0);
}

void EightBit::MOS6502::STZ_abs() {
	WriteByte_Absolute((uint8_t)0);
}

void EightBit::MOS6502::STZ_absx() {
	WriteByte_AbsoluteX((uint8_t)0);
}

//

void EightBit::MOS6502::TSX_imp() {
	x = s;
	UpdateZeroNegativeFlags(x);
}

void EightBit::MOS6502::TAX_imp() {
	x = a;
	UpdateZeroNegativeFlags(x);
}

void EightBit::MOS6502::TAY_imp() {
	y = a;
	UpdateZeroNegativeFlags(y);
}

void EightBit::MOS6502::TXS_imp() {
	s = x;
}

void EightBit::MOS6502::TYA_imp() {
	a = y;
	UpdateZeroNegativeFlags(a);
}

void EightBit::MOS6502::TXA_imp() {
	a = x;
	UpdateZeroNegativeFlags(a);
}

//

void EightBit::MOS6502::PHP_imp() {
	p.brk = true;
	PushByte(p);
}

void EightBit::MOS6502::PLP_imp() {
	p = PopByte();
	p.reserved = true;
}

void EightBit::MOS6502::PLA_imp() {
	a = PopByte();
	UpdateZeroNegativeFlags(a);
}

void EightBit::MOS6502::PHA_imp() {
	PushByte(a);
}

void EightBit::MOS6502::PHX_imp() {
	PushByte(x);
}

void EightBit::MOS6502::PHY_imp() {
	PushByte(y);
}

void EightBit::MOS6502::PLX_imp() {
	x = PopByte();
	UpdateZeroNegativeFlags(x);
}

void EightBit::MOS6502::PLY_imp() {
	y = PopByte();
	UpdateZeroNegativeFlags(y);
}

//

void EightBit::MOS6502::ASL_a() {
	a = ASL(a);
}

void EightBit::MOS6502::ASL_zp() {
	register16_t address;
	Address_ZeroPage(address);
	ASL(address.word);
}

void EightBit::MOS6502::ASL_abs() {
	register16_t address;
	Address_Absolute(address);
	ASL(address.word);
}

void EightBit::MOS6502::ASL_absx() {
	register16_t address;
	Address_AbsoluteX_Write(address);
	ASL(address.word);
}

void EightBit::MOS6502::ASL_zpx() {
	register16_t address;
	Address_ZeroPageX(address);
	ASL(address.word);
}

//

void EightBit::MOS6502::LSR_absx() {
	register16_t address;
	Address_AbsoluteX_Write(address);
	LSR(address.word);
}

void EightBit::MOS6502::LSR_zpx() {
	register16_t address;
	Address_ZeroPageX(address);
	LSR(address.word);
}

void EightBit::MOS6502::LSR_abs() {
	register16_t address;
	Address_Absolute(address);
	LSR(address.word);
}

void EightBit::MOS6502::LSR_a() {
	a = LSR(a);
}

void EightBit::MOS6502::LSR_zp() {
	register16_t address;
	Address_ZeroPage(address);
	LSR(address.word);
}

//

void EightBit::MOS6502::ROL_absx() {
	register16_t address;
	Address_AbsoluteX_Write(address);
	ROL(address.word);
}

void EightBit::MOS6502::ROL_zpx() {
	register16_t address;
	Address_ZeroPageX(address);
	ROL(address.word);
}

void EightBit::MOS6502::ROL_abs() {
	register16_t address;
	Address_Absolute(address);
	ROL(address.word);
}

void EightBit::MOS6502::ROL_a() {
	a = ROL(a);
}

void EightBit::MOS6502::ROL_zp() {
	register16_t address;
	Address_ZeroPage(address);
	ROL(address.word);
}

//

void EightBit::MOS6502::ROR_absx() {
	register16_t address;
	Address_AbsoluteX_Write(address);
	ROR(address.word);
}

void EightBit::MOS6502::ROR_zpx() {
	register16_t address;
	Address_ZeroPageX(address);
	ROR(address.word);
}

void EightBit::MOS6502::ROR_abs() {
	register16_t address;
	Address_Absolute(address);
	ROR(address.word);
}

void EightBit::MOS6502::ROR_a() {
	a = ROR(a);
}

void EightBit::MOS6502::ROR_zp() {
	register16_t address;
	Address_ZeroPage(address);
	ROR(address.word);
}

//

void EightBit::MOS6502::TSB_zp() {
	register16_t address;
	Address_ZeroPage(address);
	TSB(address.word);
}

void EightBit::MOS6502::TSB_abs() {
	register16_t address;
	Address_Absolute(address);
	TSB(address.word);
}

//

void EightBit::MOS6502::TRB_zp() {
	register16_t address;
	Address_ZeroPage(address);
	TRB(address.word);
}

void EightBit::MOS6502::TRB_abs() {
	register16_t address;
	Address_Absolute(address);
	TRB(address.word);
}

//

void EightBit::MOS6502::RMB0_zp() {
	register16_t address;
	Address_ZeroPage(address);
	RMB(address.word, 1);
}

void EightBit::MOS6502::RMB1_zp() {
	register16_t address;
	Address_ZeroPage(address);
	RMB(address.word, 2);
}

void EightBit::MOS6502::RMB2_zp() {
	register16_t address;
	Address_ZeroPage(address);
	RMB(address.word, 4);
}

void EightBit::MOS6502::RMB3_zp() {
	register16_t address;
	Address_ZeroPage(address);
	RMB(address.word, 8);
}

void EightBit::MOS6502::RMB4_zp() {
	register16_t address;
	Address_ZeroPage(address);
	RMB(address.word, 0x10);
}

void EightBit::MOS6502::RMB5_zp() {
	register16_t address;
	Address_ZeroPage(address);
	RMB(address.word, 0x20);
}

void EightBit::MOS6502::RMB6_zp() {
	register16_t address;
	Address_ZeroPage(address);
	RMB(address.word, 0x40);
}

void EightBit::MOS6502::RMB7_zp() {
	register16_t address;
	Address_ZeroPage(address);
	RMB(address.word, 0x80);
}

//

void EightBit::MOS6502::SMB0_zp() {
	register16_t address;
	Address_ZeroPage(address);
	SMB(address.word, 1);
}

void EightBit::MOS6502::SMB1_zp() {
	register16_t address;
	Address_ZeroPage(address);
	SMB(address.word, 2);
}

void EightBit::MOS6502::SMB2_zp() {
	register16_t address;
	Address_ZeroPage(address);
	SMB(address.word, 4);
}

void EightBit::MOS6502::SMB3_zp() {
	register16_t address;
	Address_ZeroPage(address);
	SMB(address.word, 8);
}

void EightBit::MOS6502::SMB4_zp() {
	register16_t address;
	Address_ZeroPage(address);
	SMB(address.word, 0x10);
}

void EightBit::MOS6502::SMB5_zp() {
	register16_t address;
	Address_ZeroPage(address);
	SMB(address.word, 0x20);
}

void EightBit::MOS6502::SMB6_zp() {
	register16_t address;
	Address_ZeroPage(address);
	SMB(address.word, 0x40);
}

void EightBit::MOS6502::SMB7_zp() {
	register16_t address;
	Address_ZeroPage(address);
	SMB(address.word, 0x80);
}

//

void EightBit::MOS6502::JSR_abs() {
	register16_t destination;
	Address_Absolute(destination);
	PC().word--;
	PushWord(PC());
	PC() = destination;
}

void EightBit::MOS6502::RTI_imp() {
	PLP_imp();
	PopWord(PC());
}

void EightBit::MOS6502::RTS_imp() {
	PopWord(PC());
	PC().word++;
}

void EightBit::MOS6502::JMP_abs() {
	register16_t address;
	Address_Absolute(address);
	PC() = address;
}

void EightBit::MOS6502::JMP_ind() {
	register16_t address;
	Address_Absolute(address);
	GetWord(address.word, PC());
}

void EightBit::MOS6502::JMP_absxind() {
	register16_t address;
	Address_AbsoluteXIndirect(address);
	PC() = address;
}

void EightBit::MOS6502::BRK_imp() {
	PC().word++;
	PushWord(PC());
	PHP_imp();
	p.interrupt = true;
	if (level >= ProcessorType::Cpu65SC02)
		p.decimal = false;

	 GetWord(IRQvector, PC());
}

//

void EightBit::MOS6502::WAI_imp() {
	throw std::runtime_error("Not implemented");
}

void EightBit::MOS6502::STP_imp() {
	throw std::runtime_error("Not implemented");
}

//

void EightBit::MOS6502::SED_imp() {
	p.decimal = true;
}

void EightBit::MOS6502::CLD_imp() {
	p.decimal = false;
}

void EightBit::MOS6502::CLV_imp() {
	p.overflow = false;
}

void EightBit::MOS6502::SEI_imp() {
	p.interrupt = true;
}

void EightBit::MOS6502::CLI_imp() {
	p.interrupt = false;
}

void EightBit::MOS6502::CLC_imp() {
	p.carry = false;
}

void EightBit::MOS6502::SEC_imp() {
	p.carry = true;
}

//

void EightBit::MOS6502::BMI_rel() {
	Branch(p.negative);
}

void EightBit::MOS6502::BPL_rel() {
	Branch(!p.negative);
}

void EightBit::MOS6502::BVC_rel() {
	Branch(!p.overflow);
}

void EightBit::MOS6502::BVS_rel() {
	Branch(p.overflow);
}

void EightBit::MOS6502::BCC_rel() {
	Branch(!p.carry);
}

void EightBit::MOS6502::BCS_rel() {
	Branch(p.carry);
}

void EightBit::MOS6502::BNE_rel() {
	Branch(!p.zero);
}

void EightBit::MOS6502::BEQ_rel() {
	Branch(p.zero);
}

void EightBit::MOS6502::BRA_rel() {
	Branch();
}

//

void EightBit::MOS6502::BBR0_zprel() {
	BitBranch_Clear(0x1);
}

void EightBit::MOS6502::BBR1_zprel() {
	BitBranch_Clear(0x2);
}

void EightBit::MOS6502::BBR2_zprel() {
	BitBranch_Clear(0x4);
}

void EightBit::MOS6502::BBR3_zprel() {
	BitBranch_Clear(0x8);
}

void EightBit::MOS6502::BBR4_zprel() {
	BitBranch_Clear(0x10);
}

void EightBit::MOS6502::BBR5_zprel() {
	BitBranch_Clear(0x20);
}

void EightBit::MOS6502::BBR6_zprel() {
	BitBranch_Clear(0x40);
}

void EightBit::MOS6502::BBR7_zprel() {
	BitBranch_Clear(0x80);
}

void EightBit::MOS6502::BBS0_zprel() {
	BitBranch_Set(0x1);
}

void EightBit::MOS6502::BBS1_zprel() {
	BitBranch_Set(0x2);
}

void EightBit::MOS6502::BBS2_zprel() {
	BitBranch_Set(0x4);
}

void EightBit::MOS6502::BBS3_zprel() {
	BitBranch_Set(0x8);
}

void EightBit::MOS6502::BBS4_zprel() {
	BitBranch_Set(0x10);
}

void EightBit::MOS6502::BBS5_zprel() {
	BitBranch_Set(0x20);
}

void EightBit::MOS6502::BBS6_zprel() {
	BitBranch_Set(0x40);
}

void EightBit::MOS6502::BBS7_zprel() {
	BitBranch_Set(0x80);
}
