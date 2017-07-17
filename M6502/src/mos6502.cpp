#include "stdafx.h"
#include "mos6502.h"

EightBit::MOS6502::MOS6502(Memory& memory)
: Processor(memory) {
	m_timings = {
		////	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
		/* 0 */	7, 6, 0, 0, 0, 4, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,
		/* 1 */	2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		/* 2 */	6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
		/* 3 */	2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		/* 4 */	6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
		/* 5 */	2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		/* 6 */	6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
		/* 7 */	2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		/* 8 */	0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,
		/* 9 */	2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,
		/* A */	2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
		/* B */	2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
		/* C */	2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
		/* D */	2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
		/* E */	2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
		/* F */	2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
	};
}

EightBit::MOS6502::~MOS6502() {
}

void EightBit::MOS6502::initialise() {

	Processor::initialise();

	for (int i = 0; i < 0x100; ++i) {
		m_decodedOpcodes[i] = i;
	}

	PC().word = 0;
	X() = Bit7;
	Y() = 0;
	A() = 0;

	P() = 0;
	setFlag(P(), RF);

	S() = Mask8;

	MEMPTR().word = 0;
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

	cycles = m_timings[cell];

	// http://www.llx.com/~nparker/a2/opcodes.html

	// Most instructions that explicitly reference memory
	// locations have bit patterns of the form aaabbbcc.
	// The aaa and cc bits determine the opcode, and the bbb
	// bits determine the addressing mode.

	const auto& decoded = m_decodedOpcodes[cell];

	switch (decoded.cc) {
	case 0b00:
		switch (decoded.aaa) {
		case 0b000:
			switch (decoded.bbb) {
			case 0b000:	// BRK
				BRK();
				break;
			case 0b010:	// PHP
				PHP();
				break;
			case 0b100:	// BPL
				Branch(!(P() & NF));
				break;
			case 0b110:	// CLC
				clearFlag(P(), CF);
				break;
			default:
				throw std::domain_error("Illegal instruction");
			}
			break;
		case 0b001:
			switch (decoded.bbb) {
			case 0b000:	// JSR
				JSR_abs();
				break;
			case 0b010:	// PLP
				PLP();
				break;
			case 0b100:	// BMI
				Branch((P() & NF) != 0);
				break;
			case 0b110:	// SEC
				setFlag(P(), CF);
				break;
			default:	// BIT
				BIT(AM_00(decoded.bbb));
				assert(m_busRW);
				m_memory.read();
				break;
			}
			break;
		case 0b010:
			switch (decoded.bbb) {
			case 0b000:	// RTI
				RTI();
				break;
			case 0b010:	// PHA
				PushByte(A());
				break;
			case 0b011:	// JMP
				JMP_abs();
				break;
			case 0b100:	// BVC
				Branch(!(P() & VF));
				break;
			case 0b110:	// CLI
				clearFlag(P(), IF);
				break;
			default:
				throw std::domain_error("Illegal addressing mode");
			}
			break;
		case 0b011:
			switch (decoded.bbb) {
			case 0b000:	// RTS
				RTS();
				break;
			case 0b010:	// PLA
				adjustNZ(A() = PopByte());
				break;
			case 0b011:	// JMP (abs)
				JMP_ind();
				break;
			case 0b100:	// BVS
				Branch((P() & VF) != 0);
				break;
			case 0b110:	// SEI
				setFlag(P(), IF);
				break;
			default:
				throw std::domain_error("Illegal addressing mode");
			}
			break;
		case 0b100:
			switch (decoded.bbb) {
			case 0b010:	// DEY
				adjustNZ(--Y());
				break;
			case 0b100:	// BCC
				Branch(!(P() & CF));
				break;
			case 0b110:	// TYA
				adjustNZ(A() = Y());
				break;
			default:	// STY
				AM_00(decoded.bbb, false);
				assert(m_busRW);
				m_memory.write(Y());
				break;
			}
			break;
		case 0b101:
			switch (decoded.bbb) {
			case 0b010:	// TAY
				adjustNZ(Y() = A());
				break;
			case 0b100:	// BCS
				Branch((P() & CF) != 0);
				break;
			case 0b110:	// CLV
				clearFlag(P(), VF);
				break;
			default:	// LDY
				adjustNZ(Y() = AM_00(decoded.bbb));
				if (m_busRW)
					m_memory.read();
				break;
			}
			break;
		case 0b110:
			switch (decoded.bbb) {
			case 0b010:	// INY
				adjustNZ(++Y());
				break;
			case 0b100:	// BNE
				Branch(!(P() & ZF));
				break;
			case 0b110:	// CLD
				clearFlag(P(), DF);
				break;
			default:	// CPY
				CMP(Y(), AM_00(decoded.bbb));
				if (m_busRW)
					m_memory.read();
				break;
			}
			break;
		case 0b111:
			switch (decoded.bbb) {
			case 0b010:	// INX
				adjustNZ(++X());
				break;
			case 0b100:	// BEQ
				Branch((P() & ZF) != 0);
				break;
			case 0b110:	// SED
				setFlag(P(), DF);
				break;
			default:	// CPX
				CMP(X(), AM_00(decoded.bbb));
				if (m_busRW)
					m_memory.read();
				break;
			}
			break;
		}
		break;
	case 0b01:
		switch (decoded.aaa) {
		case 0b000:		// ORA
			adjustNZ(A() |= AM_01(decoded.bbb));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b001:		// AND
			adjustNZ(A() &= AM_01(decoded.bbb));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b010:		// EOR
			adjustNZ(A() ^= AM_01(decoded.bbb));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b011:		// ADC
			ADC(AM_01(decoded.bbb));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b100:		// STA
			AM_01(decoded.bbb, false);
			assert(m_busRW);
			m_memory.write(A());
			break;
		case 0b101:		// LDA
			adjustNZ(A() = AM_01(decoded.bbb));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b110:		// CMP
			CMP(A(), AM_01(decoded.bbb));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b111:		// SBC
			SBC(AM_01(decoded.bbb));
			if (m_busRW)
				m_memory.read();
			break;
		default:
			__assume(0);
		}
		break;
	case 0b10:
		switch (decoded.aaa) {
		case 0b000:		// ASL
			ASL(AM_10(decoded.bbb, false));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b001:		// ROL
			ROL(AM_10(decoded.bbb, false));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b010:		// LSR
			LSR(AM_10(decoded.bbb, false));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b011:		// ROR
			ROR(AM_10(decoded.bbb, false));
			if (m_busRW)
				m_memory.read();
			break;
		case 0b100:
			switch (decoded.bbb) {
			case 0b010:	// TXA
				adjustNZ(A() = X());
				break;
			case 0b110:	// TXS
				S() = X();
				break;
			default:	// STX
				AM_10_x(decoded.bbb, false);
				assert(m_busRW);
				m_memory.write(X());
				break;
			}
			break;
		case 0b101:
			switch (decoded.bbb) {
			case 0b110:	// TSX
				adjustNZ(X() = S());
				break;
			default:	// LDX
				adjustNZ(X() = AM_10_x(decoded.bbb));
				if (m_busRW)
					m_memory.read();
				break;
			}
			break;
		case 0b110:
			switch (decoded.bbb) {
			case 0b010:	// DEX
				adjustNZ(--X());
				break;
			default:	// DEC
				adjustNZ(--AM_10(decoded.bbb, false));
				if (m_busRW)
					m_memory.read();
				break;
			}
			break;
		case 0b111:
			switch (decoded.bbb) {
			case 0b010:	// NOP
				break;
			default:	// INC
				adjustNZ(++AM_10(decoded.bbb, false));
				if (m_busRW)
					m_memory.read();
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

void EightBit::MOS6502::ROR(uint8_t& output) {
	auto carry = P() & CF;
	setFlag(P(), CF, output & CF);
	output = (output >> 1) | (carry << 7);
	adjustNZ(output);
}

void EightBit::MOS6502::LSR(uint8_t& output) {
	setFlag(P(), CF, output & CF);
	adjustNZ(output >>= 1);
}

void EightBit::MOS6502::BIT(uint8_t data) {
	adjustZero(A() & data);
	adjustNegative(data);
	setFlag(P(), VF, data & VF);
}

void EightBit::MOS6502::ROL(uint8_t& output) {
	uint8_t result = (output << 1) | (P() & CF);
	setFlag(P(), CF, output & Bit7);
	adjustNZ(output = result);
}

void EightBit::MOS6502::ASL(uint8_t& output) {
	setFlag(P(), CF, (output & Bit7) >> 7);
	adjustNZ(output <<= 1);
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

	adjustNZ(difference.low);
	setFlag(P(), VF, (A() ^ data) & (A() ^ difference.low) & NF);
	clearFlag(P(), CF, difference.high);

	A() = difference.low;
}

void EightBit::MOS6502::SBC_d(uint8_t data) {
	auto carry = ~P() & CF;

	register16_t difference;
	difference.word = A() - data - carry;

	adjustNZ(difference.low);

	setFlag(P(), VF, (A() ^ data) & (A() ^ difference.low) & NF);
	clearFlag(P(), CF, difference.high);

	auto low = (uint8_t)(lowNibble(A()) - lowNibble(data) - carry);

	auto lowNegative = (int8_t)low < 0;
	if (lowNegative)
		low -= 6;

	uint8_t high = highNibble(A()) - highNibble(data) - (lowNegative ? 1 : 0);

	if ((int8_t)high < 0)
		high -= 6;

	A() = promoteNibble(high) | lowNibble(low);
}

void EightBit::MOS6502::CMP(uint8_t first, uint8_t second) {
	register16_t result;
	result.word = first - second;
	adjustNZ(result.low);
	clearFlag(P(), CF, result.high);
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

	adjustNZ(sum.low);
	setFlag(P(), VF, ~(A() ^ data) & (A() ^ sum.low) & NF);
	setFlag(P(), CF, sum.high & CF);

	A() = sum.low;
}

void EightBit::MOS6502::ADC_d(uint8_t data) {
	auto carry = P() & CF;

	register16_t sum;
	sum.word = A() + data + carry;

	adjustNZ(sum.low);

	auto low = (uint8_t)(lowNibble(A()) + lowNibble(data) + carry);
	if (low > 9)
		low += 6;

	auto high = (uint8_t)(highNibble(A()) + highNibble(data) + (low > 0xf ? 1 : 0));
	setFlag(P(), VF, ~(A() ^ data) & (A() ^ promoteNibble(high)) & NF);

	if (high > 9)
		high += 6;

	setFlag(P(), CF, high > 0xf);

	A() = (uint8_t)(promoteNibble(high) | lowNibble(low));
}

////

void EightBit::MOS6502::Branch(int8_t displacement) {
	auto page = PC().high;
	PC().word += displacement;
	if (PC().high != page)
		cycles++;
	cycles++;
}

void EightBit::MOS6502::Branch(bool flag) {
	int8_t displacement = AM_Immediate();
	if (flag)
		Branch(displacement);
}

//

void EightBit::MOS6502::PHP() {
	setFlag(P(), BF);
	PushByte(P());
}

void EightBit::MOS6502::PLP() {
	P() = PopByte();
	setFlag(P(), RF);
}

//

void EightBit::MOS6502::JSR_abs() {
	Address_Absolute();
	PC().word--;
	PushWord(PC());
	PC() = MEMPTR();
}

void EightBit::MOS6502::RTI() {
	PLP();
	PopWord(PC());
}

void EightBit::MOS6502::RTS() {
	PopWord(PC());
	PC().word++;
}

void EightBit::MOS6502::JMP_abs() {
	Address_Absolute();
	PC() = MEMPTR();
}

void EightBit::MOS6502::JMP_ind() {
	Address_Indirect();
	PC() = MEMPTR();
}

void EightBit::MOS6502::BRK() {
	PC().word++;
	PushWord(PC());
	PHP();
	setFlag(P(), IF);
	GetWord(IRQvector, PC());
}
