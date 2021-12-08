#pragma once

#include <cstdint>
#include <utility>

#include <EightBitCompilerDefinitions.h>
#include <LittleEndianProcessor.h>
#include <Register.h>
#include <Signal.h>
#include <EventArgs.h>

namespace EightBit {

	class Bus;

	class MOS6502 : public LittleEndianProcessor {
	public:
		DECLARE_PIN_INPUT(NMI)
		DECLARE_PIN_INPUT(SO)
		DECLARE_PIN_OUTPUT(SYNC)
		DECLARE_PIN_INPUT(RDY)
		DECLARE_PIN_OUTPUT(RW)

	public:
		enum StatusBits {
			NF = Bit7,	// Negative
			VF = Bit6,  // Overflow
			RF = Bit5,  // reserved
			BF = Bit4,  // Brk
			DF = Bit3,	// D (use BCD for arithmetic)
			IF = Bit2,	// I (IRQ disable)
			ZF = Bit1,	// Zero
			CF = Bit0,	// Carry
		};

		MOS6502(Bus& bus) noexcept;

		Signal<MOS6502> ExecutingInstruction;
		Signal<MOS6502> ExecutedInstruction;

		int execute() final;
		[[nodiscard]] int step() final;

		[[nodiscard]] constexpr auto& X() noexcept { return x; }
		[[nodiscard]] constexpr auto& Y() noexcept { return y; }
		[[nodiscard]] constexpr auto& A() noexcept { return a; }
		[[nodiscard]] constexpr auto& S() noexcept { return s; }

		[[nodiscard]] constexpr auto& P() noexcept { return p; }
		[[nodiscard]] constexpr const auto& P() const noexcept { return p; }

	protected:
		void handleRESET() final;
		void handleINT() final;

		void busWrite() final;
		[[nodiscard]] uint8_t busRead() final;

		[[nodiscard]] virtual uint8_t sub(uint8_t operand, uint8_t data, int borrow = 0) noexcept;
		[[nodiscard]] uint8_t sbc(uint8_t operand, uint8_t data) noexcept;
		[[nodiscard]] uint8_t sub_b(uint8_t operand, uint8_t data, int borrow) noexcept;
		[[nodiscard]] uint8_t sub_d(uint8_t operand, uint8_t data, int borrow) noexcept;

		[[nodiscard]] virtual uint8_t add(uint8_t operand, uint8_t data, int carry = 0) noexcept;
		[[nodiscard]] uint8_t adc(uint8_t operand, uint8_t data) noexcept;
		[[nodiscard]] uint8_t add_b(uint8_t operand, uint8_t data, int carry) noexcept;
		[[nodiscard]] uint8_t add_d(uint8_t operand, uint8_t data, int carry) noexcept;

	private:
		const uint8_t IRQvector = 0xfe;		// IRQ vector
		const uint8_t RSTvector = 0xfc;		// RST vector
		const uint8_t NMIvector = 0xfa;		// NMI vector

		void handleNMI();
		void handleSO();

		void interrupt();

		void push(uint8_t value) final;
		[[nodiscard]] uint8_t pop() final;

		// Dummy stack push, used during RESET
		void dummyPush(uint8_t value);

		// Addressing modes

		[[nodiscard]] register16_t Address_Absolute();
		[[nodiscard]] uint8_t Address_ZeroPage();
		[[nodiscard]] register16_t Address_ZeroPageIndirect();
		[[nodiscard]] register16_t Address_Indirect();
		[[nodiscard]] uint8_t Address_ZeroPageX();
		[[nodiscard]] uint8_t Address_ZeroPageY();
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_AbsoluteX();
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_AbsoluteY();
		[[nodiscard]] register16_t Address_IndexedIndirectX();
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_IndirectIndexedY();
		[[nodiscard]] register16_t Address_relative_byte();

		// Addressing modes, read

		enum class PageCrossingBehavior { AlwaysReadTwice, MaybeReadTwice };

		uint8_t AM_Immediate();
		uint8_t AM_Absolute();
		uint8_t AM_ZeroPage();
		uint8_t AM_AbsoluteX(PageCrossingBehavior behaviour = PageCrossingBehavior::MaybeReadTwice);
		uint8_t AM_AbsoluteY();
		uint8_t AM_ZeroPageX();
		uint8_t AM_ZeroPageY();
		uint8_t AM_IndexedIndirectX();
		uint8_t AM_IndirectIndexedY();

		// Flag checking

		[[nodiscard]] constexpr auto interruptMasked() const noexcept { return P() & IF; }
		[[nodiscard]] constexpr auto decimal() const noexcept { return P() & DF; }

		[[nodiscard]] static constexpr auto negative(uint8_t data) noexcept { return data & NF; }
		[[nodiscard]] constexpr auto negative() const noexcept { return negative(P()); }

		[[nodiscard]] static constexpr auto zero(uint8_t data) noexcept { return data & ZF; }
		[[nodiscard]] constexpr auto zero() const noexcept { return zero(P()); }

		[[nodiscard]] static constexpr auto overflow(uint8_t data) noexcept { return data & VF; }
		[[nodiscard]] constexpr auto overflow() const noexcept { return overflow(P()); }

		[[nodiscard]] static constexpr auto carry(uint8_t data) noexcept { return data & CF; }
		[[nodiscard]] constexpr auto carry() const noexcept { return carry(P()); }

		// Flag adjustment

		constexpr void adjustZero(const uint8_t datum) noexcept { P() = clearBit(P(), ZF, datum); }
		constexpr void adjustNegative(const uint8_t datum) noexcept { P() = setBit(P(), NF, negative(datum)); }

		constexpr void adjustNZ(const uint8_t datum) noexcept {
			adjustZero(datum);
			adjustNegative(datum);
		}

		// Miscellaneous

		void branch(int condition);

		[[nodiscard]] constexpr auto through(const uint8_t data) noexcept {
			adjustNZ(data);
			return data;
		}

		void memoryReadModifyWrite(const uint8_t data) {
			// The read will have already taken place...
			memoryWrite();
			memoryWrite(data);
		}

		// Instruction implementations

		[[nodiscard]] uint8_t andr(uint8_t operand, uint8_t data) noexcept;
		[[nodiscard]] uint8_t asl(uint8_t value) noexcept;
		void bit(uint8_t operand, uint8_t data) noexcept;
		void cmp(uint8_t first, uint8_t second) noexcept;
		[[nodiscard]] uint8_t dec(uint8_t value) noexcept;
		[[nodiscard]] uint8_t eorr(uint8_t operand, uint8_t data) noexcept;
		[[nodiscard]] uint8_t inc(uint8_t value) noexcept;
		void jsr();
		[[nodiscard]] uint8_t lsr(uint8_t value) noexcept;
		[[nodiscard]] uint8_t orr(uint8_t operand, uint8_t data) noexcept;
		void php();
		void plp();
		[[nodiscard]] uint8_t rol(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t ror(uint8_t operand) noexcept;
		void rti();
		void rts();

		// Undocumented compound instructions

		void anc(uint8_t value) noexcept;
		void arr(uint8_t value) noexcept;
		void asr(uint8_t value) noexcept;
		void axs(uint8_t value) noexcept;
		void dcp(uint8_t value);
		void isb(uint8_t value);
		void rla(uint8_t value);
		void rra(uint8_t value);
		void slo(uint8_t value);
		void sre(uint8_t value);

		// Complicated addressing mode implementations

		void sta_AbsoluteX();
		void sta_AbsoluteY();

		uint8_t x = 0;		// index register X
		uint8_t y = 0;		// index register Y
		uint8_t a = 0;		// accumulator
		uint8_t s = 0;		// stack pointer
		uint8_t p = 0;		// processor status

		register16_t m_intermediate;

		bool m_handlingRESET = false;
		bool m_handlingNMI = false;
		bool m_handlingINT = false;
	};
}