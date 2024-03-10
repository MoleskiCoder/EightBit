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

		int execute() noexcept final;
		[[nodiscard]] int step() noexcept final;

		[[nodiscard]] constexpr auto& X() noexcept { return m_x; }
		[[nodiscard]] constexpr auto& Y() noexcept { return m_y; }
		[[nodiscard]] constexpr auto& A() noexcept { return m_a; }
		[[nodiscard]] constexpr auto& S() noexcept { return m_s; }

		[[nodiscard]] constexpr auto& P() noexcept { return m_p; }
		[[nodiscard]] constexpr const auto& P() const noexcept { return m_p; }

	protected:
		void handleRESET() noexcept final;
		void handleINT() noexcept final;

		void busWrite() noexcept final;
		[[nodiscard]] uint8_t busRead() noexcept final;

		// Instructions with BCD effects

		[[nodiscard]] virtual uint8_t sub(uint8_t operand, uint8_t data, int borrow = 0) noexcept;
		[[nodiscard]] void sbc() noexcept;
		[[nodiscard]] uint8_t sub_b(uint8_t operand, uint8_t data, int borrow = 0) noexcept;
		[[nodiscard]] uint8_t sub_d(uint8_t operand, uint8_t data, int borrow = 0) noexcept;

		[[nodiscard]] virtual uint8_t add(uint8_t operand, uint8_t data, int carry = 0) noexcept;
		[[nodiscard]] void adc() noexcept;
		[[nodiscard]] uint8_t add_b(uint8_t operand, uint8_t data, int carry) noexcept;
		[[nodiscard]] uint8_t add_d(uint8_t operand, uint8_t data, int carry) noexcept;

		// Undocumented compound instructions (with BCD effects)

		virtual void arr(uint8_t value) noexcept;
		virtual void arr_b(uint8_t value) noexcept;
		virtual void arr_d(uint8_t value) noexcept;

	private:
		const uint8_t IRQvector = 0xfe;		// IRQ vector
		const uint8_t RSTvector = 0xfc;		// RST vector
		const uint8_t NMIvector = 0xfa;		// NMI vector

		void handleNMI() noexcept;
		void handleSO() noexcept;

		void interrupt() noexcept;

		constexpr void setStackAddress(uint8_t position) noexcept {
			BUS().ADDRESS() = { position, 1 };
		}

		constexpr void pushDownStackAddress(uint8_t value) noexcept {
			BUS().DATA() = value;
			setStackAddress(S()--);
		}

		constexpr void popUpStackAddress() noexcept {
			setStackAddress(++S());
		}

		void push(uint8_t value) noexcept final;
		[[nodiscard]] uint8_t pop() noexcept final;

		// Dummy stack push, used during RESET
		void dummyPush(uint8_t value) noexcept;

		// Addressing modes

		[[nodiscard]] auto Address_Immediate() noexcept { return PC()++; }
		[[nodiscard]] auto Address_Absolute() noexcept { return fetchWord(); }
		[[nodiscard]] auto Address_ZeroPage() noexcept { return register16_t(fetchByte(), 0); }
		[[nodiscard]] register16_t Address_ZeroPageIndirect() noexcept;
		[[nodiscard]] register16_t Address_Indirect() noexcept;
		[[nodiscard]] register16_t Address_ZeroPageX() noexcept;
		[[nodiscard]] register16_t Address_ZeroPageY() noexcept;
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_AbsoluteX() noexcept;
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_AbsoluteY() noexcept;
		[[nodiscard]] register16_t Address_IndexedIndirectX() noexcept;
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_IndirectIndexedY() noexcept;
		[[nodiscard]] register16_t Address_relative_byte() noexcept;

		// Addressing modes, read

		auto AM_Immediate() noexcept { return memoryRead(Address_Immediate()); }
		auto AM_Absolute() noexcept { return memoryRead(Address_Absolute()); }
		auto AM_ZeroPage() noexcept { return memoryRead(Address_ZeroPage()); }
		auto AM_ZeroPageX() noexcept { return memoryRead(Address_ZeroPageX()); }
		auto AM_ZeroPageY() noexcept { return memoryRead(Address_ZeroPageY()); }
		auto AM_IndexedIndirectX() noexcept { return memoryRead(Address_IndexedIndirectX()); }

		auto AM_AbsoluteX() noexcept {
			maybe_fixup(Address_AbsoluteX());
			return memoryRead();
		}

		auto AM_AbsoluteY() noexcept {
			maybe_fixup(Address_AbsoluteY());
			return memoryRead();
		}

		auto AM_IndirectIndexedY() noexcept {
			maybe_fixup(Address_IndirectIndexedY());
			return memoryRead();
		}

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

		constexpr void adjustZero(const uint8_t datum) noexcept { reset_flag(ZF, datum); }
		constexpr void adjustNegative(const uint8_t datum) noexcept { set_flag(NF, negative(datum)); }

		constexpr void adjustNZ(const uint8_t datum) noexcept {
			adjustZero(datum);
			adjustNegative(datum);
		}

		constexpr void adjustOverflow_add(uint8_t operand, uint8_t data, uint8_t intermediate) noexcept {
			set_flag(VF, negative(~(operand ^ data) & (operand ^ intermediate)));
		}

		constexpr void adjustOverflow_subtract(uint8_t operand, uint8_t data, uint8_t intermediate) noexcept {
			set_flag(VF, negative((operand ^ data) & (operand ^ intermediate)));
		}

		// Miscellaneous

		void branch(int condition) noexcept;

		[[nodiscard]] constexpr auto through(const uint8_t data) noexcept {
			adjustNZ(data);
			return data;
		}

#define FIXUP_RMW(ADDRESSING, OPERATION) \
		{ \
			fixup(ADDRESSING()); \
			_RMW(OPERATION); \
		}

#define RMW(ADDRESSING, OPERATION) \
		{ \
			BUS().ADDRESS() = ADDRESSING(); \
			_RMW(OPERATION); \
		}

#define _RMW(OPERATION) \
		{ \
			const auto data = memoryRead(); \
			const auto result = OPERATION(data); \
			memoryWrite(); \
			memoryWrite(result); \
		}

		void maybe_fixup(register16_t address, uint8_t unfixed_page) noexcept {
			BUS().ADDRESS() = { address.low, unfixed_page };
			if (unfixed_page != address.high) {
				memoryRead();
				BUS().ADDRESS().high = address.high;
			}
		}

		void maybe_fixup(std::pair<register16_t, uint8_t> fixing) noexcept {
			maybe_fixup(fixing.first, fixing.second);
		}

		void fixup(register16_t address, uint8_t unfixed_page) noexcept {
			BUS().ADDRESS() = { address.low, unfixed_page };
			memoryRead();
			BUS().ADDRESS().high = address.high;
		}

		void fixup(std::pair<register16_t, uint8_t> fixing) noexcept {
			fixup(fixing.first, fixing.second);
		}

		// Status flag operations
		
		constexpr void set_flag(int which, int condition) noexcept { P() = setBit(P(), which, condition); }
		constexpr void set_flag(int which) noexcept { P() = setBit(P(), which); }

		constexpr void reset_flag(int which, int condition) noexcept { P() = clearBit(P(), which, condition); }
		constexpr void reset_flag(int which) noexcept { P() = clearBit(P(), which); }

		// Chew up a cycle
		void swallow() noexcept { memoryRead(PC()); }
		void swallow_stack() noexcept { getBytePaged(1, S()); }
		void swallow_fetch() noexcept { fetchByte(); }

		// Instruction implementations

		void andr() noexcept;
		void bit(uint8_t operand, uint8_t data) noexcept;
		void cmp(uint8_t first) noexcept;
		[[nodiscard]] uint8_t dec(uint8_t value) noexcept;
		void eorr() noexcept;
		[[nodiscard]] uint8_t inc(uint8_t value) noexcept;
		void jsr() noexcept;
		void orr() noexcept;
		void php() noexcept;
		void plp() noexcept;
		void rti() noexcept;
		void rts() noexcept;

		[[nodiscard]] constexpr uint8_t asl(uint8_t value) noexcept {
			set_flag(CF, value & Bit7);
			return through(value << 1);
		}

		[[nodiscard]] constexpr uint8_t rol(uint8_t operand) noexcept {
			const auto carryIn = carry();
			return through(asl(operand) | carryIn);
		}

		[[nodiscard]] constexpr uint8_t lsr(uint8_t value) noexcept {
			set_flag(CF, value & Bit0);
			return through(value >> 1);
		}

		[[nodiscard]] constexpr uint8_t ror(uint8_t operand) noexcept {
			const auto carryIn = carry();
			return through(lsr(operand) | (carryIn << 7));
		}

		// Undocumented compound instructions

		void anc() noexcept;
		void axs() noexcept;
		void jam() noexcept;

		// Undocumented complicated mode implementations

		// SHA
		void sha_AbsoluteY() noexcept;
		void sha_IndirectIndexedY() noexcept;

		// TAS
		void tas_AbsoluteY() noexcept;

		// LAS
		void las_AbsoluteY() noexcept;

		// SYA
		void sya_AbsoluteX() noexcept;

		// SXA
		void sxa_AbsoluteY() noexcept;

		uint8_t m_x = 0;		// index register X
		uint8_t m_y = 0;		// index register Y
		uint8_t m_a = 0;		// accumulator
		uint8_t m_s = 0;		// stack pointer
		uint8_t m_p = 0;		// processor status

		register16_t m_intermediate;

		bool m_handlingRESET = false;
		bool m_handlingNMI = false;
		bool m_handlingINT = false;
	};
}