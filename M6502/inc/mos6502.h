#pragma once

#include <cstdint>
#include <utility>

#include <LittleEndianProcessor.h>
#include <Register.h>
#include <Signal.h>
#include <EventArgs.h>

namespace EightBit {

	class Bus;

	class MOS6502 : public LittleEndianProcessor {
	private:
		using base = LittleEndianProcessor;

	private:
		uint8_t m_x = 0;		// index register X
		uint8_t m_y = 0;		// index register Y
		uint8_t m_a = 0;		// accumulator
		uint8_t m_s = 0;		// stack pointer
		uint8_t m_p = 0;		// processor status

		register16_t m_intermediate;

		uint8_t m_fixedPage = 0;
		uint8_t m_unfixedPage = 0;

		bool m_immediateInstruction = false;

	public:
		DECLARE_PIN_INPUT(NMI)
		DECLARE_PIN_INPUT(SO)
		DECLARE_PIN_OUTPUT(SYNC)
		DECLARE_PIN_INPUT(RDY)
		DECLARE_PIN_OUTPUT(RW)

	public:
		enum StatusBits {
			NF = Bit7,	// Negative
			VF = Bit6,	// Overflow
			RF = Bit5,	// reserved
			BF = Bit4,	// Brk
			DF = Bit3,	// D (use BCD for arithmetic)
			IF = Bit2,	// I (IRQ disable)
			ZF = Bit1,	// Zero
			CF = Bit0,	// Carry
		};

		MOS6502(Bus& bus) noexcept;

		void execute() noexcept final;
		void poweredStep() noexcept final;

		[[nodiscard]] constexpr auto& X() noexcept { return m_x; }
		[[nodiscard]] constexpr auto& Y() noexcept { return m_y; }
		[[nodiscard]] constexpr auto& A() noexcept { return m_a; }
		[[nodiscard]] constexpr auto& S() noexcept { return m_s; }

		[[nodiscard]] constexpr auto& P() noexcept { return m_p; }
		[[nodiscard]] constexpr const auto& P() const noexcept { return m_p; }

	protected:
		void handleRESET() noexcept final;
		void handleINT() noexcept final;

		void memoryWrite() noexcept final;
		void memoryRead() noexcept final;

		void getPagedInto(uint8_t page, uint8_t offset, register16_t& into) { Processor::getPagedInto(page, offset, into); }

	private:
		const uint8_t _vectorIRQ = 0xfe;		// IRQ vector
		const uint8_t _vectorRST = 0xfc;		// RST vector
		const uint8_t _vectorNMI = 0xfa;		// NMI vector

		void handleNMI() noexcept;
		void handleSO() noexcept;

		void adjustInterruptFlags();

		void reset();
		void interruptMaskable();
		void interruptNonMaskable();
		void BRK() noexcept;

		constexpr void updateStack(uint8_t position) noexcept { BUS().ADDRESS() = { position, 1 }; }
		constexpr void lowerStack() noexcept { updateStack(S()--); }
		constexpr void raiseStack() noexcept { updateStack(++S()); }

		void push(uint8_t value) noexcept final;
		void pop() noexcept final;

		// Dummy stack push, used during RESET
		void dummyPush() noexcept;

		void readFromMemory() noexcept;
		void writeToMemory() noexcept;

		void modifyWrite(uint8_t data) noexcept;

		// Read the opcode within the existing cycle
		[[nodiscard]] uint8_t fetchInstruction() noexcept final;

		#pragma region Addressing modes

		#pragma region Address page fixup

		[[nodiscard]] constexpr auto fixedPage() const noexcept { return m_fixedPage; }
		[[nodiscard]] constexpr auto unfixedPage() const noexcept { return m_unfixedPage; }
		[[nodiscard]] constexpr auto fixed() const noexcept { return fixedPage() != unfixedPage(); }

		void maybeFixup() noexcept;
		void fixup() noexcept;
		void maybeFixupRead() noexcept;
		void fixupRead() noexcept;

		constexpr void noteFixedAddress(int address) noexcept {
			noteFixedAddress((uint16_t)address);
		}

		constexpr void noteFixedAddress(uint16_t address) noexcept {
			m_unfixedPage = BUS().ADDRESS().high;
			intermediate() = address;
			m_fixedPage = intermediate().high;
			BUS().ADDRESS().low = intermediate().low;
		}

		#pragma endregion

		#pragma region Address resolution

		void getAddressPaged() noexcept;
		void absoluteAddress() noexcept;
		void zeroPageAddress() noexcept;
		void zeroPageIndirectAddress() noexcept;
		void indirectAddress() noexcept;
		void zeroPageWithIndexAddress(uint8_t index) noexcept;
		void zeroPageXAddress() noexcept;
		void zeroPageYAddress() noexcept;
		void absoluteWithIndexAddress(uint8_t index) noexcept;
		void absoluteXAddress() noexcept;
		void absoluteYAddress() noexcept;
		void indexedIndirectXAddress() noexcept;
		void indirectIndexedYAddress() noexcept;

		#pragma endregion

		#pragma region Address and read

		void immediate() noexcept;
		void absolute() noexcept;
		void zeroPage() noexcept;
		void zeroPageX() noexcept;
		void zeroPageY() noexcept;
		void indexedIndirectX() noexcept;
		void absoluteX() noexcept;
		void absoluteY() noexcept;
		void indirectIndexedY() noexcept;

		#pragma endregion

		#pragma endregion

		// Flag checking

		[[nodiscard]] constexpr auto interruptMasked() const noexcept { return P() & IF; }
		[[nodiscard]] constexpr auto denary() const noexcept { return P() & DF; }

		[[nodiscard]] static constexpr auto negativeTest(uint8_t data) noexcept { return data & NF; }
		[[nodiscard]] static constexpr auto zeroTest(uint8_t data) noexcept { return data & ZF; }
		[[nodiscard]] static constexpr auto carryTest(uint8_t data) noexcept { return data & CF; }
		[[nodiscard]] static constexpr auto overflowTest(uint8_t data) noexcept { return data & VF; }

		[[nodiscard]] constexpr auto negative() const noexcept { return negativeTest(P()); }
		[[nodiscard]] constexpr auto zero() const noexcept { return zeroTest(P()); }
		[[nodiscard]] constexpr auto overflow() const noexcept { return overflowTest(P()); }
		[[nodiscard]] constexpr auto carry() const noexcept { return carryTest(P()); }

		// Flag adjustment

		constexpr void adjustZero(const uint8_t datum) noexcept { resetFlag(ZF, datum); }
		constexpr void adjustNegative(const uint8_t datum) noexcept { setFlag(NF, negativeTest(datum)); }

		constexpr void adjustNZ(const uint8_t datum) noexcept {
			adjustZero(datum);
			adjustNegative(datum);
		}

		// Miscellaneous

		[[nodiscard]] constexpr auto through(const uint8_t data) noexcept {
			adjustNZ(data);
			return data;
		}

		[[nodiscard]] constexpr auto through() noexcept {
			return through(BUS().DATA());
		}

		// Status flag operations

		constexpr static void setFlag(uint8_t& f, int which, int condition) noexcept { f = setBit(f, which, condition); }
		constexpr void setFlag(int which, int condition) noexcept { setFlag(P(), which, condition); }
		constexpr void setFlag(int which) noexcept { P() = setBit(P(), which); }

		constexpr static void resetFlag(uint8_t& f, int which, int condition) noexcept { f = clearBit(f, which, condition); }
		constexpr void resetFlag(int which, int condition) noexcept { resetFlag(P(), which, condition); }
		constexpr void resetFlag(int which) noexcept { P() = clearBit(P(), which); }

		// Chew up a cycle
		void swallowRead() noexcept;
		void swallowPop() noexcept;
		void swallowFetch() noexcept;

		#pragma region Instruction implementations

		#pragma region Miscellaneous

		static constexpr void NOP() noexcept {
			// No operation!
		}

		void JMP() noexcept;

		#pragma endregion

		#pragma region Register to register transfer

		constexpr void TAX() noexcept { X() = through(A()); }
		constexpr void TXA() noexcept { A() = through(X()); }
		constexpr void TAY() noexcept { Y() = through(A()); }
		constexpr void TYA() noexcept { A() = through(Y()); }
		constexpr void TXS() noexcept { S() = X(); }
		constexpr void TSX() noexcept { X() = through(S()); }

		#pragma endregion

		#pragma region Load and store

		constexpr void LDA() noexcept { A() = through(); }
		void STA() noexcept;

		constexpr void LDX() noexcept { X() = through(); }
		void STX() noexcept;

		void LDY() noexcept { Y() = through(); }
		void STY() noexcept;

		#pragma endregion

		#pragma region Branching

		void fixupBranch(int8_t relative) noexcept;
		void branch(bool condition) noexcept;

		void branchNot(int condition) noexcept;
		void branch(int condition) noexcept;
			
		void BCS() noexcept;
		void BCC() noexcept;
		void BVC() noexcept;
		void BMI() noexcept;
		void BPL() noexcept;
		void BVS() noexcept;
		void BEQ() noexcept;
		void BNE() noexcept;

		#pragma endregion

		#pragma region Status flag operations

		constexpr void SEI() noexcept { setFlag(IF); }
		constexpr void CLI() noexcept { resetFlag(IF); }
		constexpr void SEV() noexcept { setFlag(VF); }
		constexpr void CLV() noexcept { resetFlag(VF); }
		constexpr void SEC() noexcept { setFlag(CF); }
		constexpr void CLC() noexcept { resetFlag(CF); }
		constexpr void SED() noexcept { setFlag(DF); }
		constexpr void CLD() noexcept { resetFlag(DF); }

		#pragma endregion

		#pragma region Instructions with BCD effects

		#pragma region Addition / subtraction

		#pragma region Subtraction

		constexpr void adjustOverflowSubtract(uint8_t operand) noexcept {
			const auto data = BUS().DATA();
			const auto result = intermediate().low;
			setFlag(VF, negativeTest((operand ^ data) & (operand ^ result)));
		}

		constexpr void SBC() noexcept {
			const auto operand = A();
			A() = SUB(operand, carryTest(~P()));
			PostSUB(operand);
		}

		constexpr uint8_t SUB(uint8_t operand, int borrow) noexcept {
			return denary() != 0 ? decimalSUB(operand, borrow) : binarySUB(operand, borrow);
		}

		constexpr uint8_t binarySUB(uint8_t operand, int borrow = 0) noexcept {
			const auto data = BUS().DATA();
			intermediate() = operand - data - borrow;

			const auto result = intermediate().low;
			adjustNZ(result);

			return result;
		}

		constexpr uint8_t decimalSUB(uint8_t operand, int borrow) noexcept {

			binarySUB(operand, borrow);

			const auto data = BUS().DATA();
			auto low = lowNibble(operand) - lowNibble(data) - borrow;
			const auto lowNegative = negativeTest(low);
			if (lowNegative != 0)
				low -= 6;

			auto high = highNibble(operand) - highNibble(data) - (lowNegative >> 7);
			const auto highNegative = negativeTest(high);
			if (highNegative != 0)
				high -= 6;

			return promoteNibble(high) | lowNibble(low);
		}

		constexpr void PostSUB(uint8_t operand) noexcept {
			adjustOverflowSubtract(operand);
			resetFlag(CF, intermediate().high);
		}

		#pragma endregion

		#pragma region Addition

		constexpr void adjustOverflowAdd(uint8_t operand) noexcept {
			const auto data = BUS().DATA();
			const auto result = intermediate().low;
			setFlag(VF, negativeTest(~(operand ^ data) & (operand ^ result)));
		}

		constexpr void ADC() noexcept { ADC(BUS().DATA()); }

		constexpr void ADC(uint8_t data) noexcept { A() = denary() != 0 ? decimalADC(data) : binaryADC(data); }

		[[nodiscard]] constexpr uint8_t binaryADC(uint8_t data) noexcept {
			const auto operand = A();
			intermediate() = operand + data + carry();
			adjustOverflowAdd(operand);
			setFlag(CF, carryTest(intermediate().high));

			const auto result = intermediate().low;
			adjustNZ(result);

			return result;
		}

		[[nodiscard]] constexpr uint8_t decimalADC(uint8_t data) noexcept {

			const auto operand = A();

			auto low = (uint16_t)(lowerNibble(operand) + lowerNibble(data) + carry());
			intermediate() = higherNibble(operand) + higherNibble(data);

			adjustZero(lowByte((uint16_t)(low + intermediate().joined)));

			if (low > 0x09)
			{
				intermediate() += 0x10;
				low += 0x06;
			}

			adjustNegative(intermediate().low);
			adjustOverflowAdd(operand);

			if (intermediate().joined > 0x90)
				intermediate() += 0x60;

			setFlag(CF, intermediate().high);

			return (uint8_t)(lowerNibble(lowByte((uint16_t)(low + intermediate().joined))) | higherNibble(intermediate().low));
		}

		#pragma endregion

		#pragma endregion

		#pragma endregion

		#pragma region Bitwise operations

		constexpr void ORA() noexcept { ORA(BUS().DATA()); }
		constexpr void ORA(uint8_t data) noexcept { A() = through(A() | data); }

		constexpr void AND() noexcept { AND(BUS().DATA()); }
		constexpr void AND(uint8_t data) noexcept { A() = through(A() & data); }

		constexpr void EOR() noexcept { EOR(BUS().DATA()); }
		constexpr void EOR(uint8_t data) noexcept { A() = through(A() ^ data); }

		constexpr void bitSet(uint8_t mask) noexcept { adjustZero(A() & mask); }

		constexpr void BIT() noexcept {
			const auto data = BUS().DATA();
			setFlag(VF, overflowTest(data));
			bitSet(data);
			adjustNegative(data);
		}

		#pragma endregion

		#pragma region Comparison operations

		constexpr void CMP(uint8_t first, uint8_t second) noexcept {
			intermediate() = first - second;
			adjustNZ(intermediate().low);
			resetFlag(CF, intermediate().high);
		}

		constexpr void CMP() noexcept { CMP(A(), BUS().DATA()); }

		constexpr void CPX() noexcept { CMP(X(), BUS().DATA()); }

		constexpr void CPY() noexcept { CMP(Y(), BUS().DATA()); }

		#pragma endregion

		#pragma region Increment/decrement

		[[nodiscard]] constexpr auto DEC(uint8_t value) noexcept { return through(value - 1); }
		constexpr void DEX() noexcept { X() = DEC(X()); }
		constexpr void DEY() noexcept { Y() = DEC(Y()); }
		void DEC();

		[[nodiscard]] constexpr auto INC(uint8_t value) noexcept { return through(value + 1); }
		constexpr void INX() noexcept { X() = INC(X()); }
		constexpr void INY() noexcept { Y() = INC(Y()); }
		void INC();

		#pragma endregion

		#pragma region Stack operations

		void JSR();

		void PHA();
		void PLA();
		void PHP();
		void PLP();

		void RTI();
		void RTS();

		#pragma endregion

		#pragma region Shift/rotate operations

		#pragma region Shift

		void ASL();
		constexpr void ASLA() noexcept { A() = ASL(A()); }
		constexpr uint8_t ASL(uint8_t value) noexcept {
			setFlag(CF, negativeTest(value));
			return through(value << 1);
		}

		void LSR();
		constexpr void LSRA() noexcept { A() = LSR(A()); }
		constexpr uint8_t LSR(uint8_t value) noexcept {
			setFlag(CF, carryTest(value));
			return through(value >> 1);
		}

		#pragma endregion

		#pragma region Rotate

		void ROL();
		constexpr void ROLA() noexcept { A() = ROL(A()); }
		constexpr uint8_t ROL(uint8_t value) noexcept {
			const auto carryIn = carry();
			return through(ASL(value) | carryIn);
		}

		void ROR();
		constexpr void RORA() noexcept { A() = ROR(A()); }
		constexpr uint8_t ROR(uint8_t value) noexcept {
			const auto carryIn = carry();
			return through(LSR(value) | (carryIn << 7));
		}

		#pragma endregion

		#pragma endregion

		#pragma region Undocumented instructions

		#pragma region Undocumented instructions with BCD effects

		void ARR();
		uint8_t coreARR();
		void decimalARR();
		void binaryARR();

		#pragma endregion

		#pragma region Undocumented instructions with fixup effects

		void storeFixupEffect(uint8_t data);

		void SHA();
		void SYA();
		void SXA();

		#pragma endregion

		void SAX();
		void LAX();
		void ANC();
		void AXS();
		void JAM();
		void TAS();
		void LAS();
		void ANE();
		void ATX();
		void ASR();
		void ISB();
		void RLA();
		void RRA();
		void SLO();
		void SRE();
		void DCP();

		#pragma endregion

		#pragma endregion
	};
}