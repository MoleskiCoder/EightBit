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
	X() = 0x80;
	Y() = 0;
	A() = 0;

	P() = 0;
	setFlag(P(), RF);

	S() = 0xff;

	m_memptr.word = 0;
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
	PushByte(P());
	setFlag(P(), IF);
	GetWord(vector, PC());
}

int EightBit::MOS6502::Execute(uint8_t cell) {

	cycles = 0;

	// http://www.llx.com/~nparker/a2/opcodes.html

	// Most instructions that explicitly reference memory
	// locations have bit patterns of the form aaabbbcc.
	// The aaa and cc bits determine the opcode, and the bbb
	// bits determine the addressing mode.

	auto aaa =	(cell & 0b11100000) >> 5;
	auto bbb =	(cell & 0b00011100) >> 2;
	auto cc =	(cell & 0b00000011);

	switch (cc) {
	case 0b00:
		switch (aaa) {
		case 0b000:
			switch (bbb) {
			case 0b000:	// BRK
				BRK_imp();
				break;
			case 0b010:	// PHP
				PHP_imp();
				break;
			case 0b100:	// BPL
				BPL_rel();
				break;
			case 0b110:	// CLC
				CLC_imp();
				break;
			default:
				throw std::domain_error("Illegal instruction");
			}
			break;
		case 0b001:
			switch (bbb) {
			case 0b000:	// JSR
				JSR_abs();
				break;
			case 0b010:	// PLP
				PLP_imp();
				break;
			case 0b100:	// BMI
				BMI_rel();
				break;
			case 0b110:	// SEC
				SEC_imp();
				break;
			default:	// BIT
				BIT(AM_00(bbb));
				break;
			}
			break;
		case 0b010:
			switch (bbb) {
			case 0b000:	// RTI
				RTI_imp();
				break;
			case 0b010:	// PHA
				PHA_imp();
				break;
			case 0b011:	// JMP
				JMP_abs();
				break;
			case 0b100:	// BVC
				BVC_rel();
				break;
			case 0b110:	// BVC
				CLI_imp();
				break;
			default:
				throw std::domain_error("Illegal addressing mode");
			}
			break;
		case 0b011:
			switch (bbb) {
			case 0b000:	// RTS
				RTS_imp();
				break;
			case 0b010:	// PLA
				PLA_imp();
				break;
			case 0b011:	// JMP (abs)
				JMP_ind();
				break;
			case 0b100:	// BVS
				BVS_rel();
				break;
			case 0b110:	// SEI
				SEI_imp();
				break;
			default:
				throw std::domain_error("Illegal addressing mode");
			}
			break;
		case 0b100:
			switch (bbb) {
			case 0b010:	// DEY
				DEY_imp();
				break;
			case 0b100:	// BCC
				BCC_rel();
				break;
			case 0b110:	// TYA
				TYA_imp();
				break;
			default:	// STY
				AM_00(bbb) = Y();
				break;
			}
			break;
		case 0b101:
			switch (bbb) {
			case 0b010:	// TAY
				TAY_imp();
				break;
			case 0b100:	// BCS
				BCS_rel();
				break;
			case 0b110:	// CLV
				CLV_imp();
				break;
			default:	// LDY
				LDY(AM_00(bbb));
				break;
			}
			break;
		case 0b110:
			switch (bbb) {
			case 0b010:	// INY
				INY_imp();
				break;
			case 0b100:	// BNE
				BNE_rel();
				break;
			case 0b110:	// CLD
				CLD_imp();
				break;
			default:	// CPY
				CPY(AM_00(bbb));
				break;
			}
			break;
		case 0b111:
			switch (bbb) {
			case 0b010:	// INX
				INX_imp();
				break;
			case 0b100:	// BEQ
				BEQ_rel();
				break;
			case 0b110:	// SED
				SED_imp();
				break;
			default:	// CPX
				CPX(AM_00(bbb));
				break;
			}
			break;
		}
		break;
	case 0b01:
		switch (aaa) {
		case 0b000:		// ORA
			ORA(AM_01(bbb));
			break;
		case 0b001:		// AND
			AND(AM_01(bbb));
			break;
		case 0b010:		// EOR
			EOR(AM_01(bbb));
			break;
		case 0b011:		// ADC
			ADC(AM_01(bbb));
			break;
		case 0b100:		// STA
			AM_01(bbb) = A();
			break;
		case 0b101:		// LDA
			LDA(AM_01(bbb));
			break;
		case 0b110:		// CMP
			CMP(AM_01(bbb));
			break;
		case 0b111:		// SBC
			SBC(AM_01(bbb));
			break;
		default:
			__assume(0);
		}
		break;
	case 0b10:
		switch (aaa) {
		case 0b000:		// ASL
			ASL(AM_10(bbb));
			break;
		case 0b001:		// ROL
			ROL(AM_10(bbb));
			break;
		case 0b010:		// LSR
			LSR(AM_10(bbb));
			break;
		case 0b011:		// ROR
			ROR(AM_10(bbb));
			break;
		case 0b100:
			switch (bbb) {
			case 0b010:	// TXA
				TXA_imp();
				break;
			case 0b110:	// TXS
				TXS_imp();
				break;
			default:	// STX
				AM_10_x(bbb) = X();
				break;
			}
			break;
		case 0b101:
			switch (bbb) {
			case 0b110:	// TSX
				TSX_imp();
				break;
			default:	// LDX
				LDX(AM_10_x(bbb));
				break;
			}
			break;
		case 0b110:
			switch (bbb) {
			case 0b010:	// DEX
				DEX_imp();
				break;
			default:	// DEC
				DEC(AM_10(bbb));
				break;
			}
			break;
		case 0b111:
			switch (bbb) {
			case 0b010:	// NOP
				NOP_imp();
				break;
			default:	// INC
				INC(AM_10(bbb));
				break;
			}
			break;
		default:
			__assume(0);
		}
		break;
	case 0b11:
		throw std::domain_error("Illegal instruction group");
	default:
		__assume(0);
	}

	return cycles;
}

void EightBit::MOS6502::___() {
	if (level >= ProcessorType::Cpu65SC02) {
		// Generally, missing instructions act as a one byte,
		// one cycle NOP instruction on 65c02 (ish) processors.
		NOP_imp();
		cycles++;
	} else {
		throw std::domain_error("Whoops: Invalid instruction.");
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
				throw std::domain_error("Whoops: replacing a non-missing instruction.");
			}

			instructions[i] = newInstruction;
		}
	}
}

////

void EightBit::MOS6502::PushByte(uint8_t value) {
	SetByte(PageOne + S()--, value);
}

uint8_t EightBit::MOS6502::PopByte() {
	return GetByte(PageOne + ++S());
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

void EightBit::MOS6502::DEC(uint8_t& target) {
	UpdateZeroNegativeFlags(--target);
}

void EightBit::MOS6502::ROR(uint8_t& output) {
	auto carry = P() & CF;
	setFlag(P(), CF, output & CF);
	output = (output >> 1) | (carry << 7);
	UpdateZeroNegativeFlags(output);
}

void EightBit::MOS6502::LSR(uint8_t& output) {
	setFlag(P(), CF, output & CF);
	UpdateZeroNegativeFlags(output >>= 1);
}

void EightBit::MOS6502::BIT(uint8_t data) {
	UpdateZeroFlag(A() & data);
	UpdateNegativeFlag(data);
	setFlag(P(), VF, data & VF);
}

void EightBit::MOS6502::TSB(uint8_t& output) {
	UpdateZeroFlag(A() & output);
	output |= A();
}

void EightBit::MOS6502::TRB(uint8_t& output) {
	UpdateZeroFlag(A() & output);
	output &= ~A();
}

void EightBit::MOS6502::INC(uint8_t& output) {
	UpdateZeroNegativeFlags(++output);
}

void EightBit::MOS6502::ROL(uint8_t& output) {
	uint8_t result = (output << 1) | (P() & CF);
	setFlag(P(), CF, output & Bit7);
	UpdateZeroNegativeFlags(output = result);
}

void EightBit::MOS6502::ASL(uint8_t& output) {
	setFlag(P(), CF, (output & 0x80) >> 7);
	UpdateZeroNegativeFlags(output <<= 1);
}

void EightBit::MOS6502::ORA(uint8_t data) {
	UpdateZeroNegativeFlags(A() |= data);
}

void EightBit::MOS6502::AND(uint8_t data) {
	UpdateZeroNegativeFlags(A() &= data);
}

void EightBit::MOS6502::SBC(uint8_t data) {
	if (P() & DF)
		SBC_d(data);
	else
		SBC_b(data);
}

void EightBit::MOS6502::SBC_b(uint8_t data) {
	register16_t difference;
	difference.word = A() - data - (~P() & CF);

	UpdateZeroNegativeFlags(difference.low);
	setFlag(P(), VF, (A() ^ data) & (A() ^ difference.low) & 0x80);
	clearFlag(P(), CF, difference.high);

	A() = difference.low;
}

void EightBit::MOS6502::SBC_d(uint8_t data) {
	auto carry = ~P() & CF;

	register16_t difference;
	difference.word = A() - data - carry;

	if (level < ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(difference.low);

	setFlag(P(), VF, (A() ^ data) & (A() ^ difference.low) & 0x80);
	clearFlag(P(), CF, difference.high);

	auto low = (uint8_t)(lowNibble(A()) - lowNibble(data) - carry);

	auto lowNegative = (int8_t)low < 0;
	if (lowNegative)
		low -= 6;

	uint8_t high = highNibble(A()) - highNibble(data) - (lowNegative ? 1 : 0);

	if ((int8_t)high < 0)
		high -= 6;

	A() = promoteNibble(high) | lowNibble(low);
	if (level >= ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(A());
}

void EightBit::MOS6502::EOR(uint8_t data) {
	A() ^= data;
	UpdateZeroNegativeFlags(A());
}

void EightBit::MOS6502::CPX(uint8_t data) {
	CMP(X(), data);
}

void EightBit::MOS6502::CPY(uint8_t data) {
	CMP(Y(), data);
}

void EightBit::MOS6502::CMP(uint8_t data) {
	CMP(A(), data);
}

void EightBit::MOS6502::CMP(uint8_t first, uint8_t second) {
	register16_t result;
	result.word = first - second;
	UpdateZeroNegativeFlags(result.low);
	clearFlag(P(), CF, result.high);
}

void EightBit::MOS6502::LDA(uint8_t data) {
	UpdateZeroNegativeFlags(A() = data);
}

void EightBit::MOS6502::LDY(uint8_t data) {
	UpdateZeroNegativeFlags(Y() = data);
}

void EightBit::MOS6502::LDX(uint8_t data) {
	UpdateZeroNegativeFlags(X() = data);
}

void EightBit::MOS6502::ADC(uint8_t data) {
	if (P() & DF)
		ADC_d(data);
	else
		ADC_b(data);
}

void EightBit::MOS6502::ADC_b(uint8_t data) {

	register16_t sum;
	sum.word = A() + data + (P() & CF);

	UpdateZeroNegativeFlags(sum.low);
	setFlag(P(), VF, ~(A() ^ data) & (A() ^ sum.low) & 0x80);
	setFlag(P(), CF, sum.high & CF);

	A() = sum.low;
}

void EightBit::MOS6502::ADC_d(uint8_t data) {
	auto carry = P() & CF;

	register16_t sum;
	sum.word = A() + data + carry;

	if (level < ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(sum.low);

	auto low = (uint8_t)(lowNibble(A()) + lowNibble(data) + carry);
	if (low > 9)
		low += 6;

	auto high = (uint8_t)(highNibble(A()) + highNibble(data) + (low > 0xf ? 1 : 0));
	setFlag(P(), VF, ~(A() ^ data) & (A() ^ promoteNibble(high)) & 0x80);

	if (high > 9)
		high += 6;

	setFlag(P(), CF, high > 0xf);

	A() = (uint8_t)(promoteNibble(high) | lowNibble(low));
	if (level >= ProcessorType::Cpu65SC02)
		UpdateZeroNegativeFlags(A());
}

////

void EightBit::MOS6502::RMB(uint8_t& output, uint8_t flag) {
	output &= ~flag;
}
	
void EightBit::MOS6502::SMB(uint8_t& output, uint8_t flag) {
	output |= flag;
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
	int8_t displacement = AM_Immediate();
	Branch(displacement);
}

void EightBit::MOS6502::Branch(bool flag) {
	int8_t displacement = AM_Immediate();
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
	ORA(AM_IndexedIndirectX());
}

void EightBit::MOS6502::ORA_zp() {
	ORA(AM_ZeroPage());
}

void EightBit::MOS6502::ORA_imm() {
	ORA(AM_Immediate());
}

void EightBit::MOS6502::ORA_abs() {
	ORA(AM_Absolute());
}

void EightBit::MOS6502::ORA_absx() {
	ORA(AM_AbsoluteX());
}

void EightBit::MOS6502::ORA_absy() {
	ORA(AM_AbsoluteY());
}

void EightBit::MOS6502::ORA_zpx() {
	ORA(AM_ZeroPageX());
}

void EightBit::MOS6502::ORA_indy() {
	ORA(AM_IndirectIndexedY());
}

void EightBit::MOS6502::ORA_zpind() {
	ORA(AM_ZeroPageIndirect());
}

//

void EightBit::MOS6502::AND_zpx() {
	AND(AM_ZeroPageX());
}

void EightBit::MOS6502::AND_indy() {
	AND(AM_IndirectIndexedY());
}

void EightBit::MOS6502::AND_zp() {
	AND(AM_ZeroPage());
}

void EightBit::MOS6502::AND_absx() {
	AND(AM_AbsoluteX());
}

void EightBit::MOS6502::AND_absy() {
	AND(AM_AbsoluteY());
}

void EightBit::MOS6502::AND_imm() {
	AND(AM_Immediate());
}

void EightBit::MOS6502::AND_xind() {
	AND(AM_IndexedIndirectX());
}

void EightBit::MOS6502::AND_abs() {
	AND(AM_Absolute());
}

void EightBit::MOS6502::AND_zpind() {
	AND(AM_ZeroPageIndirect());
}

//

void EightBit::MOS6502::EOR_absx() {
	EOR(AM_AbsoluteX());
}

void EightBit::MOS6502::EOR_absy() {
	EOR(AM_AbsoluteY());
}

void EightBit::MOS6502::EOR_zpx() {
	EOR(AM_ZeroPageX());
}

void EightBit::MOS6502::EOR_indy() {
	EOR(AM_IndirectIndexedY());
}

void EightBit::MOS6502::EOR_abs() {
	EOR(AM_Absolute());
}

void EightBit::MOS6502::EOR_imm() {
	EOR(AM_Immediate());
}

void EightBit::MOS6502::EOR_zp() {
	EOR(AM_ZeroPage());
}

void EightBit::MOS6502::EOR_xind() {
	EOR(AM_IndexedIndirectX());
}

void EightBit::MOS6502::EOR_zpind() {
	EOR(AM_ZeroPageIndirect());
}

//

void EightBit::MOS6502::LDA_absx() {
	LDA(AM_AbsoluteX());
}

void EightBit::MOS6502::LDA_absy() {
	LDA(AM_AbsoluteY());
}

void EightBit::MOS6502::LDA_zpx() {
	LDA(AM_ZeroPageX());
}

void EightBit::MOS6502::LDA_indy() {
	LDA(AM_IndirectIndexedY());
}

void EightBit::MOS6502::LDA_abs() {
	LDA(AM_Absolute());
}

void EightBit::MOS6502::LDA_imm() {
	LDA(AM_Immediate());
}

void EightBit::MOS6502::LDA_zp() {
	LDA(AM_ZeroPage());
}

void EightBit::MOS6502::LDA_xind() {
	LDA(AM_IndexedIndirectX());
}

void EightBit::MOS6502::LDA_zpind() {
	LDA(AM_ZeroPageIndirect());
}

//

void EightBit::MOS6502::LDX_imm() {
	LDX(AM_Immediate());
}

void EightBit::MOS6502::LDX_zp() {
	LDX(AM_ZeroPage());
}

void EightBit::MOS6502::LDX_abs() {
	LDX(AM_Absolute());
}

void EightBit::MOS6502::LDX_zpy() {
	LDX(AM_ZeroPageY());
}

void EightBit::MOS6502::LDX_absy() {
	LDX(AM_AbsoluteY());
}

//

void EightBit::MOS6502::LDY_imm() {
	LDY(AM_Immediate());
}

void EightBit::MOS6502::LDY_zp() {
	LDY(AM_ZeroPage());
}

void EightBit::MOS6502::LDY_abs() {
	LDY(AM_Absolute());
}

void EightBit::MOS6502::LDY_zpx() {
	LDY(AM_ZeroPageX());
}

void EightBit::MOS6502::LDY_absx() {
	LDY(AM_AbsoluteX());
}

//

void EightBit::MOS6502::CMP_absx() {
	CMP(AM_AbsoluteX());
}

void EightBit::MOS6502::CMP_absy() {
	CMP(AM_AbsoluteY());
}

void EightBit::MOS6502::CMP_zpx() {
	CMP(AM_ZeroPageX());
}

void EightBit::MOS6502::CMP_indy() {
	CMP(AM_IndirectIndexedY());
}

void EightBit::MOS6502::CMP_abs() {
	CMP(AM_Absolute());
}

void EightBit::MOS6502::CMP_imm() {
	CMP(AM_Immediate());
}

void EightBit::MOS6502::CMP_zp() {
	CMP(AM_ZeroPage());
}

void EightBit::MOS6502::CMP_xind() {
	CMP(AM_IndexedIndirectX());
}

void EightBit::MOS6502::CMP_zpind() {
	CMP(AM_ZeroPageIndirect());
}

//

void EightBit::MOS6502::CPX_abs() {
	CPX(AM_Absolute());
}

void EightBit::MOS6502::CPX_zp() {
	CPX(AM_ZeroPage());
}

void EightBit::MOS6502::CPX_imm() {
	CPX(AM_Immediate());
}

//

void EightBit::MOS6502::CPY_imm() {
	CPY(AM_Immediate());
}

void EightBit::MOS6502::CPY_zp() {
	CPY(AM_ZeroPage());
}

void EightBit::MOS6502::CPY_abs() {
	CPY(AM_Absolute());
}

//

void EightBit::MOS6502::ADC_zp() {
	ADC(AM_ZeroPage());
}

void EightBit::MOS6502::ADC_xind() {
	ADC(AM_IndexedIndirectX());
}

void EightBit::MOS6502::ADC_imm() {
	ADC(AM_Immediate());
}

void EightBit::MOS6502::ADC_abs() {
	ADC(AM_Absolute());
}

void EightBit::MOS6502::ADC_zpx() {
	ADC(AM_ZeroPageX());
}

void EightBit::MOS6502::ADC_indy() {
	ADC(AM_IndirectIndexedY());
}

void EightBit::MOS6502::ADC_absx() {
	ADC(AM_AbsoluteX());
}

void EightBit::MOS6502::ADC_absy() {
	ADC(AM_AbsoluteY());
}

void EightBit::MOS6502::ADC_zpind() {
	ADC(AM_ZeroPageIndirect());
}

//

void EightBit::MOS6502::SBC_xind() {
	SBC(AM_IndexedIndirectX());
}

void EightBit::MOS6502::SBC_zp() {
	SBC(AM_ZeroPage());
}

void EightBit::MOS6502::SBC_imm() {
	SBC(AM_Immediate());
}

void EightBit::MOS6502::SBC_abs() {
	SBC(AM_Absolute());
}

void EightBit::MOS6502::SBC_zpx() {
	SBC(AM_ZeroPageX());
}

void EightBit::MOS6502::SBC_indy() {
	SBC(AM_IndirectIndexedY());
}

void EightBit::MOS6502::SBC_absx() {
	SBC(AM_AbsoluteX());
}

void EightBit::MOS6502::SBC_absy() {
	SBC(AM_AbsoluteY());
}

void EightBit::MOS6502::SBC_zpind() {
	SBC(AM_ZeroPageIndirect());
}

//

void EightBit::MOS6502::BIT_imm() {
	BIT(AM_Immediate());
}

void EightBit::MOS6502::BIT_zp() {
	BIT(AM_ZeroPage());
}

void EightBit::MOS6502::BIT_zpx() {
	BIT(AM_ZeroPageX());
}

void EightBit::MOS6502::BIT_abs() {
	BIT(AM_Absolute());
}

void EightBit::MOS6502::BIT_absx() {
	BIT(AM_AbsoluteX());
}

//

void EightBit::MOS6502::DEC_a() {
	DEC(AM_A());
}

void EightBit::MOS6502::DEC_absx() {
	DEC(AM_AbsoluteX());
}

void EightBit::MOS6502::DEC_zpx() {
	DEC(AM_ZeroPageX());
}

void EightBit::MOS6502::DEC_abs() {
	DEC(AM_Absolute());
}

void EightBit::MOS6502::DEC_zp() {
	DEC(AM_ZeroPage());
}

//

void EightBit::MOS6502::DEX_imp() {
	DEC(AM_X());
}

void EightBit::MOS6502::DEY_imp() {
	DEC(AM_Y());
}

//

void EightBit::MOS6502::INC_a() {
	INC(AM_A());
}

void EightBit::MOS6502::INC_zp() {
	INC(AM_ZeroPage());
}

void EightBit::MOS6502::INC_absx() {
	INC(AM_AbsoluteX());
}

void EightBit::MOS6502::INC_zpx() {
	INC(AM_ZeroPageX());
}

void EightBit::MOS6502::INC_abs() {
	INC(AM_Absolute());
}

//

void EightBit::MOS6502::INX_imp() {
	INC(AM_X());
}

void EightBit::MOS6502::INY_imp() {
	INC(AM_Y());
}

//

void EightBit::MOS6502::STX_zpy() {
	AM_ZeroPageY() = X();
}

void EightBit::MOS6502::STX_abs() {
	AM_Absolute() = X();
}

void EightBit::MOS6502::STX_zp() {
	AM_ZeroPage() = X();
}

//

void EightBit::MOS6502::STY_zpx() {
	AM_ZeroPageX() = Y();
}

void EightBit::MOS6502::STY_abs() {
	AM_Absolute() = Y();
}

void EightBit::MOS6502::STY_zp() {
	AM_ZeroPage() = Y();
}

//

void EightBit::MOS6502::STA_absx() {
	AM_AbsoluteX() = A();
}

void EightBit::MOS6502::STA_absy() {
	AM_AbsoluteY() = A();
}

void EightBit::MOS6502::STA_zpx() {
	AM_ZeroPageX() = A();
}

void EightBit::MOS6502::STA_indy() {
	AM_IndirectIndexedY() = A();
}

void EightBit::MOS6502::STA_abs() {
	AM_Absolute() = A();
}

void EightBit::MOS6502::STA_zp() {
	AM_ZeroPage() = A();
}

void EightBit::MOS6502::STA_xind() {
	AM_IndexedIndirectX() = A();
}

void EightBit::MOS6502::STA_zpind() {
	AM_ZeroPageIndirect() = A();
}

//

void EightBit::MOS6502::STZ_zp() {
	AM_ZeroPage() = 0;
}

void EightBit::MOS6502::STZ_zpx() {
	AM_ZeroPageX() = 0;
}

void EightBit::MOS6502::STZ_abs() {
	AM_Absolute() = 0;
}

void EightBit::MOS6502::STZ_absx() {
	AM_AbsoluteX() = 0;
}

//

void EightBit::MOS6502::TSX_imp() {
	UpdateZeroNegativeFlags(X() = S());
}

void EightBit::MOS6502::TAX_imp() {
	UpdateZeroNegativeFlags(X() = A());
}

void EightBit::MOS6502::TAY_imp() {
	UpdateZeroNegativeFlags(Y() = A());
}

void EightBit::MOS6502::TXS_imp() {
	S() = X();
}

void EightBit::MOS6502::TYA_imp() {
	UpdateZeroNegativeFlags(A() = Y());
}

void EightBit::MOS6502::TXA_imp() {
	UpdateZeroNegativeFlags(A() = X());
}

//

void EightBit::MOS6502::PHP_imp() {
	setFlag(P(), BF);
	PushByte(P());
}

void EightBit::MOS6502::PLP_imp() {
	P() = PopByte();
	setFlag(P(), RF);
}

void EightBit::MOS6502::PLA_imp() {
	A() = PopByte();
	UpdateZeroNegativeFlags(A());
}

void EightBit::MOS6502::PHA_imp() {
	PushByte(A());
}

void EightBit::MOS6502::PHX_imp() {
	PushByte(X());
}

void EightBit::MOS6502::PHY_imp() {
	PushByte(Y());
}

void EightBit::MOS6502::PLX_imp() {
	UpdateZeroNegativeFlags(X() = PopByte());
}

void EightBit::MOS6502::PLY_imp() {
	UpdateZeroNegativeFlags(Y() = PopByte());
}

//

void EightBit::MOS6502::ASL_a() {
	ASL(AM_A());
}

void EightBit::MOS6502::ASL_zp() {
	ASL(AM_ZeroPage());
}

void EightBit::MOS6502::ASL_abs() {
	ASL(AM_Absolute());
}

void EightBit::MOS6502::ASL_absx() {
	ASL(AM_AbsoluteX());
}

void EightBit::MOS6502::ASL_zpx() {
	ASL(AM_ZeroPageX());
}

//

void EightBit::MOS6502::LSR_absx() {
	LSR(AM_AbsoluteX());
}

void EightBit::MOS6502::LSR_zpx() {
	LSR(AM_ZeroPageX());
}

void EightBit::MOS6502::LSR_abs() {
	LSR(AM_Absolute());
}

void EightBit::MOS6502::LSR_a() {
	LSR(AM_A());
}

void EightBit::MOS6502::LSR_zp() {
	LSR(AM_ZeroPage());
}

//

void EightBit::MOS6502::ROL_absx() {
	ROL(AM_AbsoluteX());
}

void EightBit::MOS6502::ROL_zpx() {
	ROL(AM_ZeroPageX());
}

void EightBit::MOS6502::ROL_abs() {
	ROL(AM_Absolute());
}

void EightBit::MOS6502::ROL_a() {
	ROL(AM_A());
}

void EightBit::MOS6502::ROL_zp() {
	ROL(AM_ZeroPage());
}

//

void EightBit::MOS6502::ROR_absx() {
	ROR(AM_AbsoluteX());
}

void EightBit::MOS6502::ROR_zpx() {
	ROR(AM_ZeroPageX());
}

void EightBit::MOS6502::ROR_abs() {
	ROR(AM_Absolute());
}

void EightBit::MOS6502::ROR_a() {
	ROR(AM_A());
}

void EightBit::MOS6502::ROR_zp() {
	ROR(AM_ZeroPage());
}

//

void EightBit::MOS6502::TSB_zp() {
	TSB(AM_ZeroPage());
}

void EightBit::MOS6502::TSB_abs() {
	TSB(AM_Absolute());
}

//

void EightBit::MOS6502::TRB_zp() {
	TRB(AM_ZeroPage());
}

void EightBit::MOS6502::TRB_abs() {
	TRB(AM_Absolute());
}

//

void EightBit::MOS6502::RMB0_zp() {
	RMB(AM_ZeroPage(), 1);
}

void EightBit::MOS6502::RMB1_zp() {
	RMB(AM_ZeroPage(), 2);
}

void EightBit::MOS6502::RMB2_zp() {
	RMB(AM_ZeroPage(), 4);
}

void EightBit::MOS6502::RMB3_zp() {
	RMB(AM_ZeroPage(), 8);
}

void EightBit::MOS6502::RMB4_zp() {
	RMB(AM_ZeroPage(), 0x10);
}

void EightBit::MOS6502::RMB5_zp() {
	RMB(AM_ZeroPage(), 0x20);
}

void EightBit::MOS6502::RMB6_zp() {
	RMB(AM_ZeroPage(), 0x40);
}

void EightBit::MOS6502::RMB7_zp() {
	RMB(AM_ZeroPage(), 0x80);
}

//

void EightBit::MOS6502::SMB0_zp() {
	SMB(AM_ZeroPage(), 1);
}

void EightBit::MOS6502::SMB1_zp() {
	SMB(AM_ZeroPage(), 2);
}

void EightBit::MOS6502::SMB2_zp() {
	SMB(AM_ZeroPage(), 4);
}

void EightBit::MOS6502::SMB3_zp() {
	SMB(AM_ZeroPage(), 8);
}

void EightBit::MOS6502::SMB4_zp() {
	SMB(AM_ZeroPage(), 0x10);
}

void EightBit::MOS6502::SMB5_zp() {
	SMB(AM_ZeroPage(), 0x20);
}

void EightBit::MOS6502::SMB6_zp() {
	SMB(AM_ZeroPage(), 0x40);
}

void EightBit::MOS6502::SMB7_zp() {
	SMB(AM_ZeroPage(), 0x80);
}

//

void EightBit::MOS6502::JSR_abs() {
	Address_Absolute();
	PC().word--;
	PushWord(PC());
	PC() = m_memptr;
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
	Address_Absolute();
	PC() = m_memptr;
}

void EightBit::MOS6502::JMP_ind() {
	Address_Indirect();
	PC() = m_memptr;
}

void EightBit::MOS6502::JMP_absxind() {
	Address_IndirectX();
	PC() = m_memptr;
}

void EightBit::MOS6502::BRK_imp() {
	PC().word++;
	PushWord(PC());
	PHP_imp();
	setFlag(P(), IF);
	if (level >= ProcessorType::Cpu65SC02)
		clearFlag(P(), DF);

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
	setFlag(P(), DF);
}

void EightBit::MOS6502::CLD_imp() {
	clearFlag(P(), DF);
}

void EightBit::MOS6502::CLV_imp() {
	clearFlag(P(), VF);
}

void EightBit::MOS6502::SEI_imp() {
	setFlag(P(), IF);
}

void EightBit::MOS6502::CLI_imp() {
	clearFlag(P(), IF);
}

void EightBit::MOS6502::CLC_imp() {
	clearFlag(P(), CF);
}

void EightBit::MOS6502::SEC_imp() {
	setFlag(P(), CF);
}

//

void EightBit::MOS6502::BMI_rel() {
	Branch((P() & NF) != 0);
}

void EightBit::MOS6502::BPL_rel() {
	Branch(!(P() & NF));
}

void EightBit::MOS6502::BVC_rel() {
	Branch(!(P() & VF));
}

void EightBit::MOS6502::BVS_rel() {
	Branch((P() & VF) != 0);
}

void EightBit::MOS6502::BCC_rel() {
	Branch(!(P() & CF));
}

void EightBit::MOS6502::BCS_rel() {
	Branch((P() & CF) != 0);
}

void EightBit::MOS6502::BNE_rel() {
	Branch(!(P() & ZF));
}

void EightBit::MOS6502::BEQ_rel() {
	Branch((P() & ZF) != 0);
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
