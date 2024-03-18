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
			VF = Bit6,	// Overflow
			RF = Bit5,	// reserved
			BF = Bit4,	// Brk
			DF = Bit3,	// D (use BCD for arithmetic)
			IF = Bit2,	// I (IRQ disable)
			ZF = Bit1,	// Zero
			CF = Bit0,	// Carry
		};

		MOS6502(Bus& bus) noexcept;

		Signal<MOS6502> ExecutingInstruction;
		Signal<MOS6502> ExecutedInstruction;

		void execute() noexcept final;
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

		void sbc() noexcept;
		[[nodiscard]] virtual uint8_t sub(uint8_t operand, int borrow = 0) noexcept;
		[[nodiscard]] uint8_t sub_b(uint8_t operand, uint8_t data, int borrow = 0) noexcept;
		[[nodiscard]] uint8_t sub_d(uint8_t operand, uint8_t data, int borrow = 0) noexcept;

		void adc() noexcept;
		[[nodiscard]] virtual uint8_t add(uint8_t operand, int carry = 0) noexcept;
		[[nodiscard]] uint8_t add_b(uint8_t operand, uint8_t data, int carry) noexcept;
		[[nodiscard]] uint8_t add_d(uint8_t operand, uint8_t data, int carry) noexcept;

		// Undocumented compound instructions (with BCD effects)

		virtual void arr() noexcept;
		void arr_b(uint8_t value) noexcept;
		void arr_d(uint8_t value) noexcept;

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

		constexpr void noteUnfixedPage() noexcept { m_unfixed_page = BUS().ADDRESS().high; }

		constexpr void Address_Immediate() noexcept { BUS().ADDRESS() = PC()++; }
		void Address_Absolute() noexcept { BUS().ADDRESS() = fetchWord(); }
		void Address_ZeroPage() noexcept { BUS().ADDRESS() = register16_t(fetchByte(), 0); }
		void Address_ZeroPageIndirect() noexcept { Address_ZeroPage(); BUS().ADDRESS() = getWordPaged(); }
		void Address_Indirect() noexcept { Address_Absolute(); BUS().ADDRESS() = getWordPaged(); }
		void Address_ZeroPageWithIndex(uint8_t index) noexcept { AM_ZeroPage(); BUS().ADDRESS().low += index; }
		void Address_ZeroPageX() noexcept { Address_ZeroPageWithIndex(X()); }
		void Address_ZeroPageY() noexcept { Address_ZeroPageWithIndex(Y()); }
		void Address_AbsoluteWithIndex(uint8_t index) noexcept { Address_Absolute(); noteUnfixedPage(); BUS().ADDRESS() += index; }
		void Address_AbsoluteX() noexcept { Address_AbsoluteWithIndex(X()); }
		void Address_AbsoluteY() noexcept { Address_AbsoluteWithIndex(Y()); }
		void Address_IndexedIndirectX() noexcept { Address_ZeroPageX(); BUS().ADDRESS() = getWordPaged(); }
		void Address_IndirectIndexedY() noexcept { Address_ZeroPageIndirect(); noteUnfixedPage(); BUS().ADDRESS() += Y(); }

		// Addressing modes, with read

		void AM_Immediate() noexcept { Address_Immediate(); memoryRead(); }
		void AM_Absolute() noexcept { Address_Absolute(); memoryRead(); }
		void AM_ZeroPage() noexcept { Address_ZeroPage(); memoryRead(); }
		void AM_ZeroPageX() noexcept { Address_ZeroPageX(); memoryRead(); }
		void AM_ZeroPageY() noexcept { Address_ZeroPageY(); memoryRead(); }
		void AM_IndexedIndirectX() noexcept { Address_IndexedIndirectX(); memoryRead(); }
		void AM_AbsoluteX() noexcept { Address_AbsoluteX(); maybe_fixupR(); }
		void AM_AbsoluteY() noexcept { Address_AbsoluteY(); maybe_fixupR(); }
		void AM_IndirectIndexedY() noexcept { Address_IndirectIndexedY(); maybe_fixupR(); }

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

#define MW(OPERATION) { \
			const auto result = OPERATION(BUS().DATA()); \
			memoryWrite(); \
			memoryWrite(result); \
		}

		void maybe_fixup() noexcept {
			const auto fixed_page = BUS().ADDRESS().high;
			BUS().ADDRESS().high = m_unfixed_page;
			if (m_unfixed_page != fixed_page) {
				memoryRead();
				BUS().ADDRESS().high = fixed_page;
			}
		}

		void fixup() noexcept {
			const auto fixed_page = BUS().ADDRESS().high;
			BUS().ADDRESS().high = m_unfixed_page;
			memoryRead();
			BUS().ADDRESS().high = fixed_page;
		}

		void maybe_fixupR() noexcept { maybe_fixup(); memoryRead(); }
		void fixupR() noexcept { fixup(); memoryRead(); }

		// Status flag operations

		constexpr static void set_flag(uint8_t& f, int which, int condition) noexcept { f = setBit(f, which, condition); }
		constexpr void set_flag(int which, int condition) noexcept { set_flag(P(), which, condition); }
		constexpr void set_flag(int which) noexcept { P() = setBit(P(), which); }

		constexpr static void reset_flag(uint8_t& f, int which, int condition) noexcept { f = clearBit(f, which, condition); }
		constexpr void reset_flag(int which, int condition) noexcept { reset_flag(P(), which, condition); }
		constexpr void reset_flag(int which) noexcept { P() = clearBit(P(), which); }

		// Chew up a cycle
		void swallow() noexcept { memoryRead(PC()); }
		void swallow_stack() noexcept { memoryRead({ S(), 1 }); }
		void swallow_fetch() noexcept { fetchByte(); }

		// Instruction implementations

		void andr() noexcept;
		void bit() noexcept;
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
			set_flag(CF, value & NF);
			return through(value << 1);
		}

		[[nodiscard]] constexpr uint8_t rol(uint8_t operand) noexcept {
			const auto carryIn = carry();
			return through(asl(operand) | carryIn);
		}

		[[nodiscard]] constexpr uint8_t lsr(uint8_t value) noexcept {
			set_flag(CF, value & CF);
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

		void storeFixupEffect(uint8_t data) noexcept { memoryWrite(data & (BUS().ADDRESS().high + 1)); }

		void sha() noexcept { storeFixupEffect(A() & X()); }
		void sya() noexcept { storeFixupEffect(Y()); }
		void sxa() noexcept { storeFixupEffect(X()); }
		void tas() noexcept { S() = A() & X(); sha(); }
		void las() noexcept { A() = X() = S() = through(memoryRead() & S()); }
		void ane() noexcept { A() = through((A() | 0xee) & X() & BUS().DATA()); }
		void atx() noexcept { A() = X() = through((A() | 0xee) & BUS().DATA()); }
		void asr() noexcept { andr(); A() = lsr(A()); }

		void isb() noexcept { MW(inc); sbc(); }
		void slo() noexcept { MW(asl); orr(); }
		void rla() noexcept { MW(rol); andr(); }
		void sre() noexcept { MW(lsr); eorr(); }
		void rra() noexcept { MW(ror); adc(); }
		void dcp() noexcept { MW(dec); cmp(A()); }

		uint8_t m_x = 0;		// index register X
		uint8_t m_y = 0;		// index register Y
		uint8_t m_a = 0;		// accumulator
		uint8_t m_s = 0;		// stack pointer
		uint8_t m_p = 0;		// processor status

		register16_t m_intermediate;

		bool m_handlingRESET = false;
		bool m_handlingNMI = false;
		bool m_handlingINT = false;

		uint8_t m_unfixed_page = 0;
	};
}