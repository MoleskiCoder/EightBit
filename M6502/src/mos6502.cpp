#include "stdafx.h"
#include "mos6502.h"

EightBit::MOS6502::MOS6502(Bus& bus)
: Processor(bus) {
	m_timings = {
		////	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
		/* 0 */	7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 0, 4, 4, 6, 6,
		/* 1 */	2, 5, 0, 7, 4, 4, 6, 6, 2, 4, 2, 6, 4, 4, 7, 6,
		/* 2 */	6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 0, 4, 4, 6, 6,
		/* 3 */	2, 5, 0, 7, 4, 4, 6, 6, 2, 4, 2, 6, 4, 4, 7, 6,
		/* 4 */	6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 0, 3, 4, 6, 6,
		/* 5 */	2, 5, 0, 7, 4, 4, 6, 6, 2, 4, 2, 6, 4, 4, 7, 6,
		/* 6 */	6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 0, 5, 4, 6, 6,
		/* 7 */	2, 5, 0, 7, 4, 4, 6, 6, 2, 4, 2, 6, 4, 4, 7, 6,
		/* 8 */	2, 6, 0, 6, 3, 3, 3, 3, 2, 0, 2, 0, 4, 4, 4, 4,
		/* 9 */	2, 6, 0, 0, 4, 4, 4, 4, 2, 5, 2, 0, 0, 5, 0, 0,
		/* A */	2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 0, 4, 4, 4, 4,
		/* B */	2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 0, 4, 4, 4, 4,
		/* C */	2, 6, 0, 8, 3, 3, 5, 5, 2, 2, 2, 0, 4, 4, 6, 6,
		/* D */	2, 5, 0, 7, 4, 4, 6, 6, 2, 4, 2, 6, 4, 4, 7, 6,
		/* E */	2, 6, 0, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
		/* F */	2, 5, 0, 7, 4, 4, 6, 6, 2, 4, 2, 6, 4, 4, 7, 6,
	};


	for (int i = 0; i < 0x100; ++i)
		m_decodedOpcodes[i] = i;

	X() = Bit7;
	Y() = 0;
	A() = 0;
	P() = RF;
	S() = Mask8;

	raise(SO());
}

int EightBit::MOS6502::step() {
	resetCycles();
	auto returned = 0;
	if (LIKELY(powered())) {
		ExecutingInstruction.fire(*this);
		if (UNLIKELY(lowered(SO()))) {
			P() |= VF;
			raise(SO());
		}
		if (UNLIKELY(lowered(NMI()))) {
			raise(NMI());
			interrupt(NMIvector);
			returned = 4;	// ?? TBC
		} else if (UNLIKELY(lowered(INT()))) {
			raise(INT());
			interrupt(IRQvector);
			returned = 4;	// ?? TBC
		} else if (UNLIKELY(lowered(HALT()))) {
			execute(0xea);	// NOP
			returned = 2;	//
		} else {
			returned = execute(fetchByte());
		}
		ExecutedInstruction.fire(*this);
	}
	return returned;
}

void EightBit::MOS6502::reset() {
	Processor::reset();
	getWord(0xff, RSTvector, PC());
}

void EightBit::MOS6502::getWord(uint8_t page, uint8_t offset, register16_t& output) {
	BUS().ADDRESS().low = offset;
	BUS().ADDRESS().high = page;
	output.low = getByte();
	BUS().ADDRESS().low++;
	output.high = getByte();
}

void EightBit::MOS6502::interrupt(uint8_t vector) {
	raise(HALT());
	pushWord(PC());
	push(P());
	setFlag(P(), IF);
	getWord(0xff, vector, PC());
}

int EightBit::MOS6502::execute(uint8_t cell) {

	addCycles(m_timings[cell]);

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
			case 0b001:	// DOP/NOP (0x04)
				AM_ZeroPage();
				break;
			case 0b010:	// PHP
				PHP();
				break;
			case 0b011:	// TOP/NOP (0b00001100, 0x0c)
				AM_Absolute();
				break;
			case 0b100:	// BPL
				Branch(!(P() & NF));
				break;
			case 0b101:	// DOP/NOP (0x14)
				AM_ZeroPageX();
				break;
			case 0b110:	// CLC
				clearFlag(P(), CF);
				break;
			case 0b111:	// TOP/NOP (0b‭00011100‬, 0x1c)
				AM_AbsoluteX();
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
			case 0b101:	// DOP/NOP (0x34)
				AM_ZeroPageX();
				break;
			case 0b110:	// SEC
				setFlag(P(), CF);
				break;
			case 0b111:	// TOP/NOP (0b‭00111100‬, 0x3c)
				AM_AbsoluteX();
				break;
			default:	// BIT
				BIT(AM_00(decoded.bbb));
				break;
			}
			break;
		case 0b010:
			switch (decoded.bbb) {
			case 0b000:	// RTI
				RTI();
				break;
			case 0b001:	// DOP/NOP (0x44)
				AM_ZeroPage();
				break;
			case 0b010:	// PHA
				push(A());
				break;
			case 0b011:	// JMP
				JMP_abs();
				break;
			case 0b100:	// BVC
				Branch(!(P() & VF));
				break;
			case 0b101:	// DOP/NOP (0x54)
				AM_ZeroPageX();
				break;
			case 0b110:	// CLI
				clearFlag(P(), IF);
				break;
			case 0b111:	// TOP/NOP (0b‭01011100‬, 0x5c)
				AM_AbsoluteX();
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
			case 0b001:	// DOP/NOP (0x64)
				AM_ZeroPage();
				break;
			case 0b010:	// PLA
				adjustNZ(A() = pop());
				break;
			case 0b011:	// JMP (abs)
				JMP_ind();
				break;
			case 0b100:	// BVS
				Branch((P() & VF) != 0);
				break;
			case 0b101:	// DOP/NOP (0x74)
				AM_ZeroPageX();
				break;
			case 0b110:	// SEI
				setFlag(P(), IF);
				break;
			case 0b111:	// TOP/NOP (0b‭01111100‬, 0x7c)
				AM_AbsoluteX();
				break;
			default:
				throw std::domain_error("Illegal addressing mode");
			}
			break;
		case 0b100:
			switch (decoded.bbb) {
			case 0b000:	// DOP/NOP (0x80)
				AM_Immediate();
				break;
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
				AM_00(decoded.bbb, Y());
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
			case 0b101:	// DOP/NOP (0xd4)
				AM_ZeroPageX();
				break;
			case 0b110:	// CLD
				clearFlag(P(), DF);
				break;
			case 0b111:	// TOP/NOP (0b‭11011100‬, 0xdc)
				AM_AbsoluteX();
				break;
			default:	// CPY
				CMP(Y(), AM_00(decoded.bbb));
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
			case 0b101:	// DOP/NOP (0xf4)
				AM_ZeroPageX();
				break;
			case 0b110:	// SED
				setFlag(P(), DF);
				break;
			case 0b111:	// TOP/NOP (0b‭11111100‬, 0xfc)
				AM_AbsoluteX();
				break;
			default:	// CPX
				CMP(X(), AM_00(decoded.bbb));
				break;
			}
			break;
		}
		break;
	case 0b01:
		switch (decoded.aaa) {
		case 0b000:		// ORA
			ORA(AM_01(decoded.bbb));
			break;
		case 0b001:		// AND
			ANDA(AM_01(decoded.bbb));
			break;
		case 0b010:		// EOR
			EORA(AM_01(decoded.bbb));
			break;
		case 0b011:		// ADC
			A() = ADC(A(), AM_01(decoded.bbb));
			break;
		case 0b100:		// STA
			AM_01(decoded.bbb, A());
			break;
		case 0b101:		// LDA
			adjustNZ(A() = AM_01(decoded.bbb));
			break;
		case 0b110:		// CMP
			CMP(A(), AM_01(decoded.bbb));
			break;
		case 0b111:		// SBC
			A() = SBC(A(), AM_01(decoded.bbb));
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 0b10:
		switch (decoded.aaa) {
		case 0b000:		// ASL
			switch (decoded.bbb) {
			case 0b110:
				break;	// *NOP (0x1a)
			default:
				ASL(decoded.bbb);
				break;
			}
			break;
		case 0b001:		// ROL
			switch (decoded.bbb) {
			case 0b110:
				break;	// *NOP (0x3a)
			default:
				ROL(decoded.bbb);
				break;
			}
			break;
		case 0b010:		// LSR
			switch (decoded.bbb) {
			case 0b110:
				break;	// *NOP (0x5a)
			default:
				LSR(decoded.bbb);
				break;
			}
			break;
		case 0b011:		// ROR (0x7a)
			switch (decoded.bbb) {
			case 0b110:
				break;	// *NOP
			default:
				ROR(decoded.bbb);
				break;
			}
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
				AM_10_x(decoded.bbb, X());
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
				break;
			}
			break;
		case 0b110:
			switch (decoded.bbb) {
			case 0b010:	// DEX
				adjustNZ(--X());
				break;
			case 0b110:	// *NOP (0xda)
				break;
			default:	// DEC
				DEC(decoded.bbb);
				break;
			}
			break;
		case 0b111:
			switch (decoded.bbb) {
			case 0b010:	// NOP
				break;
			case 0b110:	// *NOP (0xfa)
				break;
			default:	// INC
				INC(decoded.bbb);
				break;
			}
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 0b11:
		switch (decoded.aaa) {
		case 0b000:	// *SLO
			SLO(decoded.bbb);
			break;
		case 0b001:	// *RLA
			RLA(decoded.bbb);
			break;
		case 0b010:	// *SRE
			SRE(decoded.bbb);
			break;
		case 0b011:	// *RRA
			RRA(decoded.bbb);
			break;
		case 0b100: // *SAX
			AM_11(decoded.bbb, A() & X());
			break;
		case 0b101:	// *LAX
			adjustNZ(X() = A() = AM_11(decoded.bbb));
			break;
		case 0b110:	// *DCP
			DCP(decoded.bbb);
			break;
		case 0b111:	// *SBC
			switch (decoded.bbb) {
			case 0b010:
				A() = SBC(A(), AM_11(decoded.bbb));
				break;
			default:	// *ISB
				ISB(decoded.bbb);
				break;
			}
			break;
		default:
			throw std::domain_error("Illegal instruction group");
		}
		break;
	default:
		UNREACHABLE;
	}

	if (UNLIKELY(cycles() == 0))
		throw std::logic_error("Unhandled opcode");

	return cycles();
}

////

void EightBit::MOS6502::push(uint8_t value) {
	setByte(PageOne + S()--, value);
}

uint8_t EightBit::MOS6502::pop() {
	return getByte(PageOne + ++S());
}

////

void EightBit::MOS6502::ROR(uint8_t& output) {
	const auto carry = P() & CF;
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
	const uint8_t result = (output << 1) | (P() & CF);
	setFlag(P(), CF, output & Bit7);
	adjustNZ(output = result);
}

void EightBit::MOS6502::ASL(uint8_t& output) {
	setFlag(P(), CF, (output & Bit7) >> 7);
	adjustNZ(output <<= 1);
}

uint8_t EightBit::MOS6502::SBC(const uint8_t operand, const uint8_t data) {

	const auto returned = SUB(operand, data, ~P() & CF);

	const register16_t& difference = MEMPTR();
	adjustNZ(difference.low);
	setFlag(P(), VF, (operand ^ data) & (operand ^ difference.low) & NF);
	clearFlag(P(), CF, difference.high);

	return returned;
}

uint8_t EightBit::MOS6502::SUB(const uint8_t operand, const uint8_t data, const int borrow) {
	return P() & DF ? SUB_d(operand, data, borrow) : SUB_b(operand, data, borrow);
}

uint8_t EightBit::MOS6502::SUB_b(const uint8_t operand, const uint8_t data, const int borrow) {
	MEMPTR().word = operand - data - borrow;
	return MEMPTR().low;
}

uint8_t EightBit::MOS6502::SUB_d(const uint8_t operand, const uint8_t data, const int borrow) {
	MEMPTR().word = operand - data - borrow;

	uint8_t low = lowNibble(operand) - lowNibble(data) - borrow;
	const auto lowNegative = low & NF;
	if (lowNegative)
		low -= 6;

	uint8_t high = highNibble(operand) - highNibble(data) - (lowNegative >> 7);
	const auto highNegative = high & NF;
	if (highNegative)
		high -= 6;

	return promoteNibble(high) | lowNibble(low);
}

void EightBit::MOS6502::CMP(uint8_t first, uint8_t second) {
	register16_t result;
	result.word = first - second;
	adjustNZ(result.low);
	clearFlag(P(), CF, result.high);
}

uint8_t EightBit::MOS6502::ADC(const uint8_t operand, const uint8_t data) {
	const auto returned = ADD(operand, data, P() & CF);
	adjustNZ(MEMPTR().low);
	return returned;
}

uint8_t EightBit::MOS6502::ADD(uint8_t operand, uint8_t data, int carry) {
	return P() & DF ? ADD_d(operand, data, carry) : ADD_b(operand, data, carry);
}

uint8_t EightBit::MOS6502::ADD_b(uint8_t operand, uint8_t data, int carry) {
	MEMPTR().word = operand + data + carry;

	setFlag(P(), VF, ~(operand ^ data) & (operand ^ MEMPTR().low) & NF);
	setFlag(P(), CF, MEMPTR().high & CF);

	return MEMPTR().low;
}

uint8_t EightBit::MOS6502::ADD_d(uint8_t operand, uint8_t data, int carry) {

	MEMPTR().word = operand + data + carry;

	uint8_t low = lowNibble(operand) + lowNibble(data) + carry;
	if (low > 9)
		low += 6;

	uint8_t high = highNibble(operand) + highNibble(data) + (low > 0xf ? 1 : 0);
	setFlag(P(), VF, ~(operand ^ data) & (operand ^ promoteNibble(high)) & NF);

	if (high > 9)
		high += 6;

	setFlag(P(), CF, high > 0xf);

	return promoteNibble(high) | lowNibble(low);
}

////

void EightBit::MOS6502::Branch(int8_t displacement) {
	const auto page = PC().high;
	PC().word += displacement;
	if (UNLIKELY(PC().high != page))
		addCycle();
	addCycle();
}

void EightBit::MOS6502::Branch(bool flag) {
	const int8_t displacement = AM_Immediate();
	if (flag)
		Branch(displacement);
}

//

void EightBit::MOS6502::PHP() {
	push(P() | BF);
}

void EightBit::MOS6502::PLP() {
	P() = (pop() | RF) & ~BF;
}

//

void EightBit::MOS6502::JSR_abs() {
	Address_Absolute();
	PC().word--;
	call();
}

void EightBit::MOS6502::RTI() {
	PLP();
	ret();
}

void EightBit::MOS6502::RTS() {
	ret();
	PC().word++;
}

void EightBit::MOS6502::JMP_abs() {
	Address_Absolute();
	jump();
}

void EightBit::MOS6502::JMP_ind() {
	Address_Indirect();
	jump();
}

void EightBit::MOS6502::BRK() {
	PC().word++;
	pushWord(PC());
	PHP();
	setFlag(P(), IF);
	getWord(0xff, IRQvector, PC());
}
