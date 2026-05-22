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

		Signal<mc6809> ExecutingInstruction;
		Signal<mc6809> ExecutedInstruction;

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

		[[nodiscard]] constexpr auto fastInterruptMasked() const { return CC() & FF; }
		[[nodiscard]] constexpr auto interruptMasked() const { return CC() & IF; }

		[[nodiscard]] constexpr auto negative() const { return CC() & NF; }
		[[nodiscard]] constexpr auto zero() const { return CC() & ZF; }
		[[nodiscard]] constexpr auto overflow() const { return CC() & VF; }
		[[nodiscard]] constexpr auto carry() const { return CC() & CF; }
		[[nodiscard]] constexpr auto halfCarry() const { return CC() & HF; }

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
		void halt() { --PC();  lowerHALT(); }
		void proceed() { ++PC(); raiseHALT(); }

	protected:
		// Default push/pop handlers

		void push(uint8_t value) noexcept final;
		[[nodiscard]] uint8_t pop() noexcept final;

		// Interrupt (etc.) handlers

		void handleRESET() noexcept final;
		void handleINT() noexcept final;

		// Bus reader/writers

		void busWrite() noexcept final;
		uint8_t busRead() noexcept final;

		void call(register16_t destination) noexcept final;
		void ret() noexcept final;

	private:
		const uint8_t RESETvector = 0xfe;		// RESET vector
		const uint8_t NMIvector = 0xfc;			// NMI vector
		const uint8_t SWIvector = 0xfa;			// SWI vector
		const uint8_t IRQvector = 0xf8;			// IRQ vector
		const uint8_t FIRQvector = 0xf6;		// FIRQ vector
		const uint8_t SWI2vector = 0xf4;		// SWI2 vector
		const uint8_t SWI3vector = 0xf2;		// SWI3 vector
		const uint8_t RESERVEDvector = 0xf0;	// RESERVED vector

		void eat(int cycles = 1) {
			for (int cycle = 0; cycle < cycles; ++cycle)
				memoryRead(Mask16);
		}

		// Read/Modify/Write

		typedef std::function<uint8_t(void)> accessor_t;
		typedef std::function<uint8_t(uint8_t)> operation_t;

		// C++ 11 version: looks great, but verbose to use
		// rmw([this]() { return AM_direct_byte(); }, [this](uint8_t data) { return asl(data); });
		void rmw(accessor_t accessor, operation_t operation) {
			const auto data = accessor();
			const auto address = BUS().ADDRESS();
			const auto result = operation(data);
			eat();
			memoryWrite(address, result);
		}

		// So define a helper macro
		#define RMW(ACCESSOR, OPERATION) \
			rmw([this]() { return ACCESSOR(); }, [this](uint8_t data) { return OPERATION(data); });

		// Stack manipulation

		void push(register16_t& stack, uint8_t value) noexcept;
		void pushS(const uint8_t value) noexcept { push(S(), value); }
		void pushU(const uint8_t value) noexcept { push(U(), value); }

		void pushWord(register16_t& stack, const register16_t value) noexcept {
			push(stack, value.low);
			push(stack, value.high);
		}

		void pushWordS(const register16_t value) noexcept { pushWord(S(), value); }
		void pushWordU(const register16_t value) noexcept { pushWord(U(), value); }

		[[nodiscard]] uint8_t pop(register16_t& stack) noexcept;
		[[nodiscard]] uint8_t popS() noexcept { return pop(S()); }
		[[nodiscard]] uint8_t popU() noexcept { return pop(U()); }

		[[nodiscard]] register16_t popWord(register16_t& stack) {
			const auto high = pop(stack);
			const auto low = pop(stack);
			return register16_t(low, high);
		}

		[[nodiscard]] auto popWordS() { return popWord(S()); }
		[[nodiscard]] auto popWordU() { return popWord(U()); }

		// Interrupt (etc.) handlers

		void handleHALT() noexcept;
		void handleNMI() noexcept;
		void handleFIRQ() noexcept;

		// Execution helpers

		void executeUnprefixed();
		void execute10();
		void execute11();

		// Register selection for "indexed"
		[[nodiscard]] register16_t& RR(int which);

		// Register selection for 8-bit transfer/exchange
		[[nodiscard]] uint8_t& referenceTransfer8(int specifier);

		// Register selection for 16-bit transfer/exchange
		[[nodiscard]] register16_t& referenceTransfer16(int specifier);

		// Addressing modes

		[[nodiscard]] register16_t Address_direct();	// DP + fetched offset
		[[nodiscard]] register16_t Address_indexed();	// Indexed address, complicated!
		[[nodiscard]] register16_t Address_extended();	// Fetched address
		register16_t Address_relative_byte();			// PC + fetched byte offset
		register16_t Address_relative_word();			// PC + fetched word offset

		// Addressing mode readers

		// Single byte readers

		[[nodiscard]] uint8_t AM_immediate_byte();
		[[nodiscard]] uint8_t AM_direct_byte();
		[[nodiscard]] uint8_t AM_indexed_byte();
		[[nodiscard]] uint8_t AM_extended_byte();

		// Word readers

		[[nodiscard]] register16_t AM_immediate_word();
		[[nodiscard]] register16_t AM_direct_word();
		[[nodiscard]] register16_t AM_indexed_word();
		[[nodiscard]] register16_t AM_extended_word();

		// Flag adjustment

		template<class T> constexpr void adjustZero(const T datum) { CC() = clearBit(CC(), ZF, datum); }
		constexpr void adjustZero(const register16_t datum) { CC() = clearBit(CC(), ZF, datum.word); }
		constexpr void adjustNegative(const uint8_t datum) { CC() = setBit(CC(), NF, datum & Bit7); }
		constexpr void adjustNegative(const register16_t datum) { adjustNegative(datum.high); }
		constexpr void adjustNegative(const uint16_t datum) { adjustNegative(register16_t(datum)); }

		template<class T> constexpr void adjustNZ(const T datum) {
			adjustZero(datum);
			adjustNegative(datum);
		}

		constexpr void adjustCarry(const uint16_t datum) { CC() = setBit(CC(), CF, datum & Bit8); }		// 8-bit addition
		constexpr void adjustCarry(const uint32_t datum) { CC() = setBit(CC(), CF, datum & Bit16); }		// 16-bit addition
		constexpr void adjustCarry(const register16_t datum) { adjustCarry(datum.word); }

		constexpr void adjustBorrow(const uint16_t datum) { CC() = clearBit(CC(), CF, datum & Bit8); }		// 8-bit subtraction
		constexpr void adjustBorrow(const uint32_t datum) { CC() = clearBit(CC(), CF, datum & Bit16); }	// 16-bit subtraction
		constexpr void adjustBorrow(const register16_t datum) { adjustBorrow(datum.word); }

		constexpr void adjustOverflow(const uint8_t before, const uint8_t data, const register16_t after) {
			const uint8_t lowAfter = after.low;
			const uint8_t highAfter = after.high;
			CC() = setBit(CC(), VF, (before ^ data ^ lowAfter ^ (highAfter << 7)) & Bit7);
		}

		constexpr void adjustOverflow(const uint16_t before, const uint16_t data, const uint32_t after) {
			const uint16_t lowAfter = after & Mask16;
			const uint16_t highAfter = after >> 16;
			CC() = setBit(CC(), VF, (before ^ data ^ lowAfter ^ (highAfter << 15)) & Bit15);
		}

		constexpr void adjustOverflow(const register16_t before, const register16_t data, const register16_t after) {
			adjustOverflow(before.word, data.word, after.word);
		}

		constexpr void adjustHalfCarry(const uint8_t before, const uint8_t data, const uint8_t after) {
			CC() = setBit(CC(), HF, (before ^ data ^ after) & Bit4);
		}

		constexpr void adjustAddition(const uint8_t before, const uint8_t data, const register16_t after) {
			const auto result = after.low;
			adjustNZ(result);
			adjustCarry(after);
			adjustOverflow(before, data, after);
			adjustHalfCarry(before, data, result);
		}

		constexpr void adjustAddition(const uint16_t before, const uint16_t data, const uint32_t after) {
			const register16_t result = after & Mask16;
			adjustNZ(result);
			adjustCarry(after);
			adjustOverflow(before, data, after);
		}

		constexpr void adjustAddition(const register16_t before, const register16_t data, const uint32_t after) {
			adjustAddition(before.word, data.word, after);
		}

		constexpr void adjustSubtraction(const uint8_t before, const uint8_t data, const register16_t after) {
			const auto result = after.low;
			adjustNZ(result);
			adjustCarry(after);
			adjustOverflow(before, data, after);
		}

		constexpr void adjustSubtraction(const uint16_t before, const uint16_t data, const uint32_t after) {
			const register16_t result = after & Mask16;
			adjustNZ(result);
			adjustCarry(after);
			adjustOverflow(before, data, after);
		}

		constexpr void adjustSubtraction(const register16_t before, const register16_t data, const uint32_t after) {
			adjustSubtraction(before.word, data.word, after);
		}


		[[nodiscard]] constexpr auto LS() const { return carry() || zero(); }						// (C OR Z)
		[[nodiscard]] constexpr auto HI() const { return !LS(); }									// !(C OR Z)
		[[nodiscard]] constexpr auto LT() const { return (negative() >> 3) ^ (overflow() >> 1); }	// (N XOR V)
		[[nodiscard]] constexpr auto GE() const { return !LT(); }									// !(N XOR V)
		[[nodiscard]] constexpr auto LE() const { return zero() || LT(); }							// (Z OR (N XOR V))
		[[nodiscard]] constexpr auto GT() const { return !LE(); }									// !(Z OR (N XOR V))

		// Branching

		auto branch(const register16_t destination, const bool condition) {
			if (condition)
				jump(destination);
			return condition;
		}

		void branchShort(const bool condition) {
			branch(Address_relative_byte(), condition);
		}

		void branchLong(const bool condition) {
			if (branch(Address_relative_word(), condition))
				eat();
		}

		// Miscellaneous

		void saveEntireRegisterState();
		void savePartialRegisterState();
		void saveRegisterState();
		void restoreRegisterState();

		template <class T> constexpr T through(const T data) {
			CC() = clearBit(CC(), VF);
			adjustNZ(data);
			return data;
		}

		// Instruction implementations

		uint8_t adc(uint8_t operand, uint8_t data);
		uint8_t add(uint8_t operand, uint8_t data, uint8_t carry = 0);
		register16_t add(register16_t operand, register16_t data);
		uint8_t andr(uint8_t operand, uint8_t data);
		uint8_t asl(uint8_t operand);
		uint8_t asr(uint8_t operand);
		void bit(uint8_t operand, uint8_t data);
		uint8_t clr(uint8_t data = 0);	// In this form for Read/Modify/Write operations
		void cmp(uint8_t operand, uint8_t data);
		void cmp(register16_t operand, register16_t data);
		uint8_t com(uint8_t operand);
		void cwai(uint8_t data);
		uint8_t da(uint8_t operand);
		uint8_t dec(uint8_t operand);
		[[nodiscard]] uint8_t eorr(uint8_t operand, uint8_t data);
		void exg(uint8_t data);
		uint8_t inc(uint8_t operand);
		void jsr(register16_t address);
		uint8_t lsr(uint8_t operand);
		register16_t mul(uint8_t first, uint8_t second);
		uint8_t neg(uint8_t operand);
		uint8_t orr(uint8_t operand, uint8_t data);
		void psh(register16_t& stack, uint8_t data);
		void pul(register16_t& stack, uint8_t data);
		uint8_t rol(uint8_t operand);
		uint8_t ror(uint8_t operand);
		void rti();
		uint8_t sbc(uint8_t operand, uint8_t data);
		uint8_t sub(uint8_t operand, uint8_t data, uint8_t carry = 0);
		register16_t sub(register16_t operand, register16_t data);
		uint8_t sex(uint8_t from);
		void swi();
		void swi2();
		void swi3();
		void tfr(uint8_t data);
		void tst(uint8_t data);

		register16_t m_d;
		register16_t m_x;
		register16_t m_y;
		register16_t m_u;
		register16_t m_s;

		uint8_t m_dp = 0;
		uint8_t m_cc = 0;

		bool m_prefix10 = false;
		bool m_prefix11 = false;
	};
}