#pragma once

// Uses some information from:
// http://www.cpu-world.com/Arch/6809.html

#include <cstdint>
#include <functional>

#include <Bus.h>
#include <BigEndianProcessor.h>
#include <Signal.h>
#include <Register.h>

namespace EightBit {
	class mc6809 : public BigEndianProcessor {
	private:
		using base = BigEndianProcessor;

	public:
		DECLARE_PIN_INPUT(NMI)
		DECLARE_PIN_INPUT(FIRQ)
		DECLARE_PIN_INPUT(HALT)
		DECLARE_PIN_OUTPUT(BA)
		DECLARE_PIN_OUTPUT(BS)
		DECLARE_PIN_OUTPUT(RW)

	public:
		enum StatusBits {

			// Entire flag: set if the complete machine state was saved in the stack.
			// If this bit is not set then only program counter and condition code
			// registers were saved in the stack. This bit is used by interrupt
			// handling routines only.
			// The bit is cleared by fast interrupts, and set by all other interrupts.
			EF = Bit7,

			// Fast interrupt mask: set if the FIRQ interrupt is disabled.
			FF = Bit6,

			// Half carry: set if there was a carry from bit 3 to bit 4 of the result
			// during the last add operation.
			HF = Bit5,

			// Interrupt mask: set if the IRQ interrupt is disabled.
			IF = Bit4,

			// Negative: set if the most significant bit of the result is set.
			// This bit can be set not only by arithmetic and logical operations,
			// but also by load / store operations.
			NF = Bit3,

			// Zero: set if the result is zero. Like the N bit, this bit can be
			// set not only by arithmetic and logical operations, but also
			// by load / store operations.
			ZF = Bit2,

			// Overflow: set if there was an overflow during last result calculation.
			// Logical, load and store operations clear this bit.
			VF = Bit1,

			// Carry: set if there was a carry from the bit 7 during last add
			// operation, or if there was a borrow from last subtract operation,
			// or if bit 7 of the A register was set during last MUL operation.
			CF = Bit0,
		};

		mc6809(Bus& bus);

		void execute() noexcept final;
		void poweredStep() noexcept final;

		[[nodiscard]] constexpr auto& D() { return m_d; }
		[[nodiscard]] constexpr auto& A() { return D().high; }
		[[nodiscard]] constexpr auto& B() { return D().low; }

		[[nodiscard]] constexpr auto& X() { return m_x; }
		[[nodiscard]] constexpr auto& Y() { return m_y; }
		[[nodiscard]] constexpr auto& U() { return m_u; }
		[[nodiscard]] constexpr auto& S() { return m_s; }

		[[nodiscard]] constexpr auto& DP() { return m_dp; }
		[[nodiscard]] constexpr auto& CC() { return m_cc; }
		[[nodiscard]] constexpr const auto& CC() const { return m_cc; }

		// Flag checking

		[[nodiscard]] constexpr auto fastInterruptMaskedFlag() const { return CC() & FF; }
		[[nodiscard]] constexpr auto fastInterruptMasked() const { return fastInterruptMaskedFlag() != 0; }
		[[nodiscard]] constexpr auto interruptMaskedFlag() const { return CC() & IF; }
		[[nodiscard]] constexpr auto interruptMasked() const { return interruptMaskedFlag() != 0; }

		[[nodiscard]] constexpr auto entireRegisterSetFlag() const { return CC() & EF; }
		[[nodiscard]] constexpr auto entireRegisterSet() const { return entireRegisterSetFlag() != 0; }
		[[nodiscard]] constexpr auto entireFlag() const { return entireRegisterSet(); }

		[[nodiscard]] constexpr auto negativeFlag() const { return CC() & NF; }
		[[nodiscard]] constexpr auto negative() const { return negativeFlag() != 0; }

		[[nodiscard]] constexpr auto zeroFlag() const { return CC() & ZF; }
		[[nodiscard]] constexpr auto zero() const { return zeroFlag() != 0; }

		[[nodiscard]] constexpr auto overflowFlag() const { return CC() & VF; }
		[[nodiscard]] constexpr auto overflow() const { return overflowFlag() != 0; }

		[[nodiscard]] constexpr auto carryFlag() const { return CC() & CF; }
		[[nodiscard]] constexpr auto carry() const { return carryFlag() != 0; }

		[[nodiscard]] constexpr auto halfCarryFlag() const { return CC() & HF; }
		[[nodiscard]] constexpr auto halfCarry() const { return halfCarryFlag() != 0; }

		//	|---------------|-----------------------------------|
		//	|	MPU State	|									|
		//	|_______________|	MPU State Definition			|
		//	|	BA  |	BS	|									|
		//	|_______|_______|___________________________________|
		//	|	0	|	0	|	Normal (running)				|
		//	|	0	|	1	|	Interrupt or RESET Acknowledge	|
		//	|	1	|	0	|	SYNC Acknowledge				|
		//	|	1	|	1	|	HALT Acknowledge				|
		//	|-------|-------|-----------------------------------|

		[[nodiscard]] auto halted() { return lowered(HALT()); }
		void halt() { lowerHALT(); }
		void proceed() { raiseHALT(); }

	protected:
		[[nodiscard]] constexpr auto& EA() { return m_ea; }

		// Default push/pop handlers

		void push(uint8_t value) noexcept final;
		void pop() noexcept final;

		// Interrupt (etc.) handlers

		void handleRESET() noexcept final;
		void handleINT() noexcept final;

		// Bus reader/writers

		void memoryRead() noexcept override;
		void memoryWrite() noexcept override;

		void fetchByte() noexcept override;

	private:
		const uint8_t _vectorRESET = 0xfe;		// RESET vector
		const uint8_t _vectorNMI = 0xfc;		// NMI vector
		const uint8_t _vectorSWI = 0xfa;		// SWI vector
		const uint8_t _vectorIRQ = 0xf8;		// IRQ vector
		const uint8_t _vectorFIRQ = 0xf6;		// FIRQ vector
		const uint8_t _vectorSWI2 = 0xf4;		// SWI2 vector
		const uint8_t _vectorSWI3 = 0xf2;		// SWI3 vector
		const uint8_t _vectorRESERVED = 0xf0;	// RESERVED vector

		void swallowRead(int ticks = 1);
		void swallowCurrent(int ticks = 1);
		void swallowPop(register16_t stack);
		void swallowEffectiveAddress();
		void swallowSpin(int ticks = 1);

		// Stack manipulation

		void push(register16_t& stack, uint8_t value);
		void pushS(uint8_t value) noexcept;
		void push(register16_t& stack, register16_t value);
		void pop(register16_t& stack) noexcept;
		void popS() noexcept;
		register16_t popWord(register16_t& stack);

		// Interrupt (etc.) handlers

		void handleHALT() noexcept;
		void handleNMI() noexcept;
		void handleFIRQ() noexcept;

		// Execution helpers

		void executeUnprefixed();
		void execute10();
		void execute11();

		void prefix10();
		void prefix11();

		#pragma region Addressing modes

		register16_t& RR(int which);

		void immediateAddress() noexcept override;
		void relativeByteAddress();
		void relativeWordAddress();
		void directAddress();
		void extendedAddress();
		void indexedAddress();

		void immediateByte();
		void directByte();
		void indexedByte();
		void extendedByte();

		void immediateShort();
		void directShort();
		void indexedShort();
		void extendedShort();

		#pragma endregion

		// Flag adjustment

		template<class T> [[nodiscard]] constexpr auto adjustZero(const T datum) { return clearBit(CC(), ZF, datum); }
		[[nodiscard]] constexpr auto adjustZero(const register16_t datum) { return clearBit(CC(), ZF, datum.joined); }
		[[nodiscard]] constexpr auto adjustNegative(const uint8_t datum) { return setBit(CC(), NF, datum & Bit7); }
		[[nodiscard]] constexpr auto adjustNegative(const register16_t datum) { return adjustNegative(datum.high); }
		[[nodiscard]] constexpr auto adjustNegative(const uint16_t datum) { return adjustNegative(register16_t(datum)); }

		template<class T> [[nodiscard]] constexpr auto adjustNZ(const T datum) {
			CC() = adjustZero(datum);
			return adjustNegative(datum);
		}

		[[nodiscard]] constexpr auto adjustCarry(const uint16_t datum) { return setBit(CC(), CF, datum & Bit8); }		// 8-bit addition
		[[nodiscard]] constexpr auto adjustCarry(const uint32_t datum) { return setBit(CC(), CF, datum & Bit16); }		// 16-bit addition
		[[nodiscard]] constexpr auto adjustCarry(const register16_t datum) { return adjustCarry(datum.joined); }

		[[nodiscard]] constexpr auto adjustBorrow(const uint16_t datum) { return clearBit(CC(), CF, datum & Bit8); }	// 8-bit subtraction
		[[nodiscard]] constexpr auto adjustBorrow(const uint32_t datum) { return clearBit(CC(), CF, datum & Bit16); }	// 16-bit subtraction
		[[nodiscard]] constexpr auto adjustBorrow(const register16_t datum) { return adjustBorrow(datum.joined); }

		[[nodiscard]] constexpr auto adjustOverflow(const uint8_t before, const uint8_t data, const register16_t after) {
			const uint8_t lowAfter = after.low;
			const uint8_t highAfter = after.high;
			return setBit(CC(), VF, (before ^ data ^ lowAfter ^ (highAfter << 7)) & Bit7);
		}

		[[nodiscard]] constexpr auto adjustOverflow(const uint16_t before, const uint16_t data, const uint32_t after) {
			const uint16_t lowAfter = after & Mask16;
			const uint16_t highAfter = after >> 16;
			return setBit(CC(), VF, (before ^ data ^ lowAfter ^ (highAfter << 15)) & Bit15);
		}

		[[nodiscard]] constexpr auto adjustOverflow(const register16_t before, const register16_t data, const register16_t after) {
			return adjustOverflow(before.joined, data.joined, after.joined);
		}

		[[nodiscard]] constexpr auto adjustHalfCarry(const uint8_t before, const uint8_t data, const uint8_t after) {
			return setBit(CC(), HF, (before ^ data ^ after) & Bit4);
		}

		[[nodiscard]] constexpr auto adjustAddition(const uint8_t before, const uint8_t data, const register16_t after) {
			const auto result = after.low;
			CC() = adjustNZ(result);
			CC() = adjustCarry(after);
			CC() = adjustOverflow(before, data, after);
			return adjustHalfCarry(before, data, result);
		}

		[[nodiscard]] constexpr auto adjustAddition(const uint16_t before, const uint16_t data, const uint32_t after) {
			const register16_t result = after & Mask16;
			CC() = adjustNZ(result);
			CC() = adjustCarry(after);
			return adjustOverflow(before, data, after);
		}

		[[nodiscard]] constexpr auto adjustAddition(const register16_t before, const register16_t data, const uint32_t after) {
			return adjustAddition(before.joined, data.joined, after);
		}

		[[nodiscard]] constexpr auto adjustSubtraction(const uint8_t before, const uint8_t data, const register16_t after) {
			const auto result = after.low;
			CC() = adjustNZ(result);
			CC() = adjustCarry(after);
			return adjustOverflow(before, data, after);
		}

		[[nodiscard]] constexpr auto adjustSubtraction(const uint16_t before, const uint16_t data, const uint32_t after) {
			const register16_t result = after & Mask16;
			CC() = adjustNZ(result);
			CC() = adjustCarry(after);
			return adjustOverflow(before, data, after);
		}

		[[nodiscard]] constexpr auto adjustSubtraction(const register16_t before, const register16_t data, const uint32_t after) {
			return adjustSubtraction(before.joined, data.joined, after);
		}

		[[nodiscard]] constexpr auto LS() const { return carry() || zero(); }						// (C OR Z)
		[[nodiscard]] constexpr auto HI() const { return !LS(); }									// !(C OR Z)
		[[nodiscard]] constexpr auto LT() const { return (negative() >> 3) ^ (overflow() >> 1); }	// (N XOR V)
		[[nodiscard]] constexpr auto GE() const { return !LT(); }									// !(N XOR V)
		[[nodiscard]] constexpr auto LE() const { return zero() || LT(); }							// (Z OR (N XOR V))
		[[nodiscard]] constexpr auto GT() const { return !LE(); }									// !(Z OR (N XOR V))

		#pragma region Load / store 8 or 16 - bit data

		[[nodiscard]] uint8_t through(uint8_t data);
		void assign(uint8_t& destination);

		void LDA();
		void LDB();

		[[nodiscard]] uint16_t through(uint16_t data);
		[[nodiscard]] register16_t through(register16_t data);
		void assign(register16_t destination);

		void LDD();
		void LDS();
		void LDU();
		void LDX();
		void LDY();

		void store(uint8_t data);

		void STA();
		void STB();

		void store(register16_t data);

		void STD();
		void STU();
		void STS();
		void STX();
		void STY();

		#pragma endregion

		#pragma region Branching

		void LBSR();
		void BSR();

		void BRA();
		void BRN();
		void BHI();
		void BLS();
		void BCC();
		void BCS();
		void BNE();
		void BEQ();
		void BVC();
		void BVS();
		void BPL();
		void BMI();
		void BGE();
		void BLT();
		void BGT();
		void BLE();

		void LBRA();
		void LBRN();
		void LBHI();
		void LBLS();
		void LBCC();
		void LBCS();
		void LBNE();
		void LBEQ();
		void LBVC();
		void LBVS();
		void LBPL();
		void LBMI();
		void LBGE();
		void LBLT();
		void LBGT();
		void LBLE();

		#pragma endregion

		#pragma region Miscellaneous instruction implementations

		void SYNC();

		static void NOP();

		void ABX();

		[[nodiscard]] uint8_t addWithCarry(uint8_t operand);
		void ADCA();
		void ADCB();

		[[nodiscard]] uint8_t add(uint8_t operand);
 		[[nodiscard]] uint8_t add(uint8_t operand, uint8_t data, int carry = 0);
		void ADDA();
		void ADDB();

		[[nodiscard]] register16_t add(register16_t operand, register16_t data, int carry);
		[[nodiscard]] register16_t add(register16_t operand, register16_t data);
		void ADDD();

		void ANDCC();

		[[nodiscard]] uint8_t _and(uint8_t operand, uint8_t data);
		[[nodiscard]] uint8_t _and(uint8_t operand);
		[[nodiscard]] uint16_t _and(uint16_t operand, uint16_t data);
		[[nodiscard]] register16_t _and(register16_t operand, register16_t data);

		void ANDA();
		void ANDB();

		[[nodiscard]] uint8_t arithmeticShiftLeft(uint8_t operand);
		void ASLA();
		void ASLB();
		void ASL();

		[[nodiscard]] uint8_t arithmeticShiftRight(uint8_t operand);
		void ASRA();
		void ASRB();
		void ASR();

		void bit(uint8_t operand, uint8_t data);
		void BITA();
		void BITB();

		[[nodiscard]] uint8_t clear();
		void CLRA();
		void CLRB();
		void CLR();

		void compare(uint8_t operand, uint8_t data);
		void CMPA();
		void CMPB();

		void compare(uint16_t operand, uint16_t data);
		void compare(register16_t operand, register16_t data);
		void compare(register16_t operand);
		void CMPU();
		void CMPS();
		void CMPD();
		void CMPX();
		void CMPY();

		[[nodiscard]] uint8_t complement(uint8_t operand);
		void COMA();
		void COMB();
		void COM();

		void CWAI();

		void DAA();

		[[nodiscard]] uint16_t exclusiveOr(uint16_t operand, uint16_t data);
		[[nodiscard]] register16_t exclusiveOr(register16_t operand, register16_t data);
		[[nodiscard]] uint8_t exclusiveOr(uint8_t operand, uint8_t data);
		[[nodiscard]] uint8_t exclusiveOr(uint8_t operand);
		void EORA();
		void EORB();

		[[nodiscard]] uint8_t decrement(uint8_t operand);
		void DECA();
		void DECB();
		void DEC();

		[[nodiscard]] uint8_t increment(uint8_t operand);
		void INCA();
		void INCB();
		void INC();

		void JMP();
		void JSR();

		[[nodiscard]] uint8_t logicalShiftRight(uint8_t operand);
		void LSRA();
		void LSRB();
		void LSR();

		void MUL();

		[[nodiscard]] uint8_t negate(uint8_t operand);
		void NEGA();
		void NEGB();
		void NEG();

		void ORCC();

		[[nodiscard]] uint16_t _or(uint16_t operand, uint16_t data);
		[[nodiscard]] register16_t _or(register16_t operand, register16_t data);
		[[nodiscard]] uint8_t _or(uint8_t operand, uint8_t data);
		[[nodiscard]] uint8_t _or(uint8_t operand);
		void ORA();
		void ORB();

		[[nodiscard]] uint8_t rotateLeft(uint8_t operand);
		void ROLA();
		void ROLB();
		void ROL();

		[[nodiscard]] uint8_t rotateRight(uint8_t operand);
		void RORA();
		void RORB();
		void ROR();

		void RTI();
		void RTS();

		[[nodiscard]] uint8_t subtractWithCarry(uint8_t operand);
		void SBCA();
		void SBCB();

		[[nodiscard]] uint8_t subtract(uint8_t operand, uint8_t data, int carry = 0);
		[[nodiscard]] uint8_t subtract(uint8_t operand);
		void SUBA();
		void SUBB();

		[[nodiscard]] uint16_t subtract(uint16_t operand, uint16_t data, int carry = 0);
		[[nodiscard]] register16_t subtract(register16_t operand, register16_t data, int carry);
		[[nodiscard]] register16_t subtract(register16_t operand, register16_t data);
		void SUBD();

		[[nodiscard]] uint8_t SEX(uint8_t from);
		void SEX();

		void SWI();
		void SWI2();
		void SWI3();

		void test(uint8_t data);
		void TSTA();
		void TSTB();
		void TST();

		void LEA(register16_t& destination);
		void LEAX();
		void LEAY();
		void LEAS();
		void LEAU();

		bool branch(register16_t destination, bool condition);
		void branchShort(bool condition);
		void branchLong(bool condition);

		#pragma region Save / restore register state

		void saveEntireRegisterState();
		void savePartialRegisterState();
		void saveRegisterState();
		void restoreRegisterState();

		void PSH(register16_t& stack, uint8_t control);
		void PSH(register16_t& stack);
		void PSHS();
		void PSHU();

		void PUL(register16_t& stack, uint8_t control);
		void PUL(register16_t& stack);
		void PULU();
		void PULS();

		#pragma endregion

		#pragma region 8 - bit register transfers

		[[nodiscard]] uint8_t& referenceTransfer8(int specifier);
		[[nodiscard]] register16_t& referenceTransfer16(int specifier);
		void EXG();
		void TFR();

		#pragma endregion

		#pragma endregion

		register16_t m_d;
		register16_t m_x;
		register16_t m_y;
		register16_t m_u;
		register16_t m_s;

		register16_t m_ea;

		uint8_t m_dp = 0;
		uint8_t m_cc = 0;

		bool m_prefix10 = false;
		bool m_prefix11 = false;
	};
}