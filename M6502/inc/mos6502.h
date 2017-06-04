#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <functional>

#include "ProcessorType.h"
#include "StatusFlags.h"
#include "AddressingMode.h"

class MOS6502 {
public:
	typedef std::function<void()> instruction_t;

	struct Instruction {
		instruction_t vector = nullptr;
		uint64_t count = 0;
		AddressingMode mode = AddressingMode::Illegal;
		std::string display = "";
	};

	MOS6502(ProcessorType level);

	ProcessorType getLevel() const	{ return level;		}
	uint64_t getCycles() const		{ return cycles;	}

	bool getProceed() const		{ return proceed;	}
	void setProceed(bool value)	{ proceed = value;	}

	uint16_t getPC() const { return pc; }
	uint8_t getX() const { return x; }
	uint8_t getY() const { return y; }
	uint8_t getA() const { return a; }
	uint8_t getS() const { return s; }

	const StatusFlags& getP() const { return p; }

	const Instruction& getInstruction(uint8_t code) const {
		return instructions[code];
	}

	virtual void Initialise();

	virtual void Start(uint16_t address);
	virtual void Run();
	virtual void Step();

	virtual void Reset();

	virtual void TriggerIRQ();
	virtual void TriggerNMI();

	uint16_t GetWord(uint16_t offset) const;

	virtual uint8_t GetByte(uint16_t offset) const = 0;
	virtual void SetByte(uint16_t offset, uint8_t value) = 0;

protected:
	virtual void Interrupt(uint16_t vector);

	virtual void Execute(uint8_t cell);

	void ___();

	void ResetRegisters();

private:
	static Instruction INS(instruction_t method, uint64_t cycles, AddressingMode addressing, std::string display);

	static uint8_t LowNybble(uint8_t value);
	static uint8_t HighNybble(uint8_t value);
	static uint8_t PromoteNybble(uint8_t value);
	static uint8_t DemoteNybble(uint8_t value);
	static uint8_t LowByte(uint16_t value);
	static uint8_t HighByte(uint16_t value);
	static uint16_t MakeWord(uint8_t low, uint8_t high);

	void Install6502Instructions();
	void Install65sc02Instructions();
	void Install65c02Instructions();

	void InstallInstructionSet(std::array<Instruction, 0x100> basis);
	void OverlayInstructionSet(std::array<Instruction, 0x100> overlay);
	void OverlayInstructionSet(std::array<Instruction, 0x100> overlay, bool includeIllegal);

	bool UpdateZeroFlag(uint8_t datum);
	bool UpdateNegativeFlag(int8_t datum);
	void UpdateZeroNegativeFlags(uint8_t datum);

	void PushByte(uint8_t value);
	uint8_t PopByte();
	void PushWord(uint16_t value);
	uint16_t PopWord();

	uint8_t FetchByte();
	uint16_t FetchWord();

	uint16_t Address_ZeroPage();
	uint16_t Address_ZeroPageX();
	uint16_t Address_ZeroPageY();
	uint16_t Address_IndexedIndirectX();
	uint16_t Address_IndexedIndirectY_Read();
	uint16_t Address_IndexedIndirectY_Write();
	uint16_t Address_Absolute();
	uint16_t Address_AbsoluteXIndirect();
	uint16_t Address_AbsoluteX_Read();
	uint16_t Address_AbsoluteX_Write();
	uint16_t Address_AbsoluteY_Read();
	uint16_t Address_AbsoluteY_Write();
	uint16_t Address_ZeroPageIndirect();

	uint8_t ReadByte_Immediate();
	int8_t ReadByte_ImmediateDisplacement();
	uint8_t ReadByte_ZeroPage();
	uint8_t ReadByte_ZeroPageX();
	uint8_t ReadByte_ZeroPageY();
	uint8_t ReadByte_Absolute();
	uint8_t ReadByte_AbsoluteX();
	uint8_t ReadByte_AbsoluteY();
	uint8_t ReadByte_IndexedIndirectX();
	uint8_t ReadByte_IndirectIndexedY();
	uint8_t ReadByte_ZeroPageIndirect();

	void WriteByte_ZeroPage(uint8_t value);
	void WriteByte_Absolute(uint8_t value);
	void WriteByte_AbsoluteX(uint8_t value);
	void WriteByte_AbsoluteY(uint8_t value);
	void WriteByte_ZeroPageX(uint8_t value);
	void WriteByte_ZeroPageY(uint8_t value);
	void WriteByte_IndirectIndexedY(uint8_t value);
	void WriteByte_IndexedIndirectX(uint8_t value);
	void WriteByte_ZeroPageIndirect(uint8_t value);

	void DEC(uint16_t offset);

	uint8_t ROR(uint8_t data);
	void ROR(uint16_t offset);

	uint8_t LSR(uint8_t data);
	void LSR(uint16_t offset);

	void BIT_immediate(uint8_t data);
	void BIT(uint8_t data);

	void TSB(uint16_t address);
	void TRB(uint16_t address);

	void INC(uint16_t offset);

	void ROL(uint16_t offset);
	uint8_t ROL(uint8_t data);

	void ASL(uint16_t offset);
	uint8_t ASL(uint8_t data);

	void ORA(uint8_t data);

	void AND(uint8_t data);

	void SBC(uint8_t data);
	void SBC_b(uint8_t data);
	void SBC_d(uint8_t data);

	void EOR(uint8_t data);

	void CPX(uint8_t data);
	void CPY(uint8_t data);
	void CMP(uint8_t data);
	void CMP(uint8_t first, uint8_t second);

	void LDA(uint8_t data);
	void LDY(uint8_t data);
	void LDX(uint8_t data);

	void ADC(uint8_t data);
	void ADC_b(uint8_t data);
	void ADC_d(uint8_t data);

	void RMB(uint16_t address, uint8_t flag);
	void SMB(uint16_t address, uint8_t flag);

	void Branch(int8_t displacement);
	void Branch();
	void Branch(bool flag);
	void BitBranch_Clear(uint8_t check);
	void BitBranch_Set(uint8_t check);

	void NOP_imp();
	void NOP2_imp();
	void NOP3_imp();

	void ORA_xind();
	void ORA_zp();
	void ORA_imm();
	void ORA_abs();
	void ORA_absx();
	void ORA_absy();
	void ORA_zpx();
	void ORA_indy();
	void ORA_zpind();

	void AND_zpx();
	void AND_indy();
	void AND_zp();
	void AND_absx();
	void AND_absy();
	void AND_imm();
	void AND_xind();
	void AND_abs();
	void AND_zpind();

	void EOR_absx();
	void EOR_absy();
	void EOR_zpx();
	void EOR_indy();
	void EOR_abs();
	void EOR_imm();
	void EOR_zp();
	void EOR_xind();
	void EOR_zpind();

	void LDA_absx();
	void LDA_absy();
	void LDA_zpx();
	void LDA_indy();
	void LDA_abs();
	void LDA_imm();
	void LDA_zp();
	void LDA_xind();
	void LDA_zpind();

	void LDX_imm();
	void LDX_zp();
	void LDX_abs();
	void LDX_zpy();
	void LDX_absy();

	void LDY_imm();
	void LDY_zp();
	void LDY_abs();
	void LDY_zpx();
	void LDY_absx();

	void CMP_absx();
	void CMP_absy();
	void CMP_zpx();
	void CMP_indy();
	void CMP_abs();
	void CMP_imm();
	void CMP_zp();
	void CMP_xind();
	void CMP_zpind();

	void CPX_abs();
	void CPX_zp();
	void CPX_imm();

	void CPY_imm();
	void CPY_zp();
	void CPY_abs();

	void ADC_zp();
	void ADC_xind();
	void ADC_imm();
	void ADC_abs();
	void ADC_zpx();
	void ADC_indy();
	void ADC_absx();
	void ADC_absy();
	void ADC_zpind();

	void SBC_xind();
	void SBC_zp();
	void SBC_imm();
	void SBC_abs();
	void SBC_zpx();
	void SBC_indy();
	void SBC_absx();
	void SBC_absy();
	void SBC_zpind();

	void BIT_imm();
	void BIT_zp();
	void BIT_zpx();
	void BIT_abs();
	void BIT_absx();

	void DEC_a();
	void DEC_absx();
	void DEC_zpx();
	void DEC_abs();
	void DEC_zp();

	void DEX_imp();
	void DEY_imp();

	void INC_a();
	void INC_zp();
	void INC_absx();
	void INC_zpx();
	void INC_abs();

	void INX_imp();
	void INY_imp();

	void STX_zpy();
	void STX_abs();
	void STX_zp();

	void STY_zpx();
	void STY_abs();
	void STY_zp();

	void STA_absx();
	void STA_absy();
	void STA_zpx();
	void STA_indy();
	void STA_abs();
	void STA_zp();
	void STA_xind();
	void STA_zpind();

	void STZ_zp();
	void STZ_zpx();
	void STZ_abs();
	void STZ_absx();

	void TSX_imp();
	void TAX_imp();
	void TAY_imp();
	void TXS_imp();
	void TYA_imp();
	void TXA_imp();

	void PHP_imp();
	void PLP_imp();
	void PLA_imp();
	void PHA_imp();
	void PHX_imp();
	void PHY_imp();
	void PLX_imp();
	void PLY_imp();

	void ASL_a();
	void ASL_zp();
	void ASL_abs();
	void ASL_absx();
	void ASL_zpx();

	void LSR_absx();
	void LSR_zpx();
	void LSR_abs();
	void LSR_a();
	void LSR_zp();

	void ROL_absx();
	void ROL_zpx();
	void ROL_abs();
	void ROL_a();
	void ROL_zp();

	void ROR_absx();
	void ROR_zpx();
	void ROR_abs();
	void ROR_a();
	void ROR_zp();

	void TSB_zp();
	void TSB_abs();

	void TRB_zp();
	void TRB_abs();

	void RMB0_zp();
	void RMB1_zp();
	void RMB2_zp();
	void RMB3_zp();
	void RMB4_zp();
	void RMB5_zp();
	void RMB6_zp();
	void RMB7_zp();

	void SMB0_zp();
	void SMB1_zp();
	void SMB2_zp();
	void SMB3_zp();
	void SMB4_zp();
	void SMB5_zp();
	void SMB6_zp();
	void SMB7_zp();

	void JSR_abs();
	void RTI_imp();
	void RTS_imp();
	void JMP_abs();
	void JMP_ind();
	void JMP_absxind();
	void BRK_imp();

	void WAI_imp();
	void STP_imp();

	void SED_imp();
	void CLD_imp();
	void CLV_imp();
	void SEI_imp();
	void CLI_imp();
	void CLC_imp();
	void SEC_imp();

	void BMI_rel();
	void BPL_rel();
	void BVC_rel();
	void BVS_rel();
	void BCC_rel();
	void BCS_rel();
	void BNE_rel();
	void BEQ_rel();
	void BRA_rel();

	void BBR0_zprel();
	void BBR1_zprel();
	void BBR2_zprel();
	void BBR3_zprel();
	void BBR4_zprel();
	void BBR5_zprel();
	void BBR6_zprel();
	void BBR7_zprel();

	void BBS0_zprel();
	void BBS1_zprel();
	void BBS2_zprel();
	void BBS3_zprel();
	void BBS4_zprel();
	void BBS5_zprel();
	void BBS6_zprel();
	void BBS7_zprel();

	const uint16_t PageOne = 0x100;
	const uint16_t IRQvector = 0xfffe;
	const uint16_t RSTvector = 0xfffc;
	const uint16_t NMIvector = 0xfffa;

	uint16_t pc;	// program counter
	uint8_t x;		// index register X
	uint8_t y;		// index register Y
	uint8_t a;		// accumulator
	uint8_t s;		// stack pointer

	StatusFlags p = 0;	// processor status

	uint64_t cycles;

	bool proceed = true;

	std::array<Instruction, 0x100> instructions;

	ProcessorType level;

	std::array<Instruction, 0x100> overlay6502;
	std::array<Instruction, 0x100> overlay65sc02;
	std::array<Instruction, 0x100> overlay65c02;
};