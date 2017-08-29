#pragma once

#include <cstdint>

#include "IntelProcessor.h"
#include "Bus.h"
#include "Signal.h"
#include "Display.h"

namespace EightBit {
	class LR35902 : public IntelProcessor {
	public:
		enum StatusBits {
			ZF = Bit7,
			NF = Bit6,
			HC = Bit5,
			CF = Bit4,
		};

		LR35902(Bus& memory);

		Signal<LR35902> ExecutingInstruction;

		void stop() { m_stopped = true; }
		void start() { m_stopped = false; }
		bool stopped() const { return m_stopped; }

		bool& IME() { return m_ime; }

		void di();
		void ei();

		virtual register16_t& AF() override {
			af.low &= 0xf0;
			return af;
		}

		virtual register16_t& BC() override  {
			return bc;
		}

		virtual register16_t& DE() override {
			return de;
		}

		virtual register16_t& HL() override {
			return hl;
		}

		virtual void reset();
		virtual void initialise();

		static int framesPerSecond() { return 60; }
		static int cyclesPerFrame() { return cyclesPerSecond() / framesPerSecond(); }

		int runRasterLines() {
			m_bus.resetLY();
			int cycles = 0;
			for (int line = 0; line < Display::RasterHeight; ++line)
				cycles += runRasterLine();
			return cycles;
		}

		int runVerticalBlankLines() {
			m_bus.triggerInterrupt(Bus::Interrupts::VerticalBlank);
			int cycles = 0;
			for (int line = 0; line < (Bus::TotalLineCount - Display::RasterHeight); ++line)
				cycles += runRasterLine();
			return cycles;
		}

		int run(int limit);

	protected:
		virtual int execute(uint8_t opcode);
		int step();

	private:
		Bus& m_bus;

		register16_t af;
		register16_t bc;
		register16_t de;
		register16_t hl;

		bool m_ime;

		bool m_prefixCB;

		bool m_stopped;

		static int cyclesPerSecond() { return 4 * 1024 * 1024; }
		static int cyclesPerLine() { return cyclesPerFrame() / Bus::TotalLineCount; }

		int runRasterLine() {
			auto cycles = run(cyclesPerLine());
			m_bus.incrementLY();
			if ((m_bus.peekRegister(Bus::STAT) & Processor::Bit6) && (m_bus.peekRegister(Bus::LYC) == m_bus.peekRegister(Bus::LY)))
				m_bus.triggerInterrupt(Bus::Interrupts::DisplayControlStatus);
			return cycles;
		}

		uint8_t R(int r, uint8_t& a) {
			switch (r) {
			case 0:
				return B();
			case 1:
				return C();
			case 2:
				return D();
			case 3:
				return E();
			case 4:
				return H();
			case 5:
				return L();
			case 6:
				return getByte(HL());
			case 7:
				return a;
			}
			throw std::logic_error("Unhandled registry mechanism");
		}

		void R(int r, uint8_t& a, uint8_t value) {
			switch (r) {
			case 0:
				B() = value;
				break;
			case 1:
				C() = value;
				break;
			case 2:
				D() = value;
				break;
			case 3:
				E() = value;
				break;
			case 4:
				H() = value;
				break;
			case 5:
				L() = value;
				break;
			case 6:
				setByte(HL(), value);
				break;
			case 7:
				a = value;
				break;
			}
		}

		register16_t& RP(int rp) {
			__assume(rp < 4);
			__assume(rp >= 0);
			switch (rp) {
			case 0b00:
				return BC();
			case 0b01:
				return DE();
			case 0b10:
				return HL();
			case 0b11:
				return SP();
			default:
				__assume(0);
			}
		}

		register16_t& RP2(int rp) {
			__assume(rp < 4);
			__assume(rp >= 0);
			switch (rp) {
			case 0b00:
				return BC();
			case 0b01:
				return DE();
			case 0b10:
				return HL();
			case 0b11:
				return AF();
			default:
				__assume(0);
			}
		}

		static void adjustHalfCarryAdd(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			setFlag(f, HC, calculateHalfCarryAdd(before, value, calculation));
		}

		static void adjustHalfCarrySub(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			setFlag(f, HC, calculateHalfCarrySub(before, value, calculation));
		}

		static void subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);

		int interrupt(uint8_t value);

		void executeCB(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		static void increment(uint8_t& f, uint8_t& operand);
		static void decrement(uint8_t& f, uint8_t& operand);

		void reti();

		bool jrConditionalFlag(uint8_t& f, int flag);
		bool returnConditionalFlag(uint8_t& f, int flag);
		bool jumpConditionalFlag(uint8_t& f, int flag);
		bool callConditionalFlag(uint8_t& f, int flag);

		void add(uint8_t& f, register16_t& operand, register16_t value);

		static void add(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);
		static void adc(uint8_t& f, uint8_t& operand, uint8_t value);
		static void sbc(uint8_t& f, uint8_t& operand, uint8_t value);
		static void andr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void xorr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void orr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void compare(uint8_t& f, uint8_t check, uint8_t value);

		static uint8_t rlc(uint8_t& f, uint8_t operand);
		static uint8_t rrc(uint8_t& f, uint8_t operand);
		static uint8_t rl(uint8_t& f, uint8_t operand);
		static uint8_t rr(uint8_t& f, uint8_t operand);
		static uint8_t sla(uint8_t& f, uint8_t operand);
		static uint8_t sra(uint8_t& f, uint8_t operand);
		static uint8_t srl(uint8_t& f, uint8_t operand);

		static uint8_t bit(uint8_t& f, int n, uint8_t operand);
		static uint8_t res(int n, uint8_t operand);
		static uint8_t set(int n, uint8_t operand);

		static void daa(uint8_t& a, uint8_t& f);

		static void scf(uint8_t& a, uint8_t& f);
		static void ccf(uint8_t& a, uint8_t& f);
		static void cpl(uint8_t& a, uint8_t& f);

		static uint8_t swap(uint8_t& f, uint8_t operand);
	};
}