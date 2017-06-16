#pragma once

#include <cstdint>

#include "IntelProcessor.h"
#include "Bus.h"
#include "Signal.h"

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

		int interrupt(uint8_t value);

		int execute(uint8_t opcode);
		int step();

		// Mutable access to processor!!

		virtual register16_t& AF() override {
			m_accumulatorFlag.low &= 0xf0;
			return m_accumulatorFlag;
		}

		virtual register16_t& BC() override  {
			return m_registers[BC_IDX];
		}

		virtual register16_t& DE() override {
			return m_registers[DE_IDX];
		}

		virtual register16_t& HL() override {
			return m_registers[HL_IDX];
		}

		virtual void reset();
		virtual void initialise();

	private:
		enum { BC_IDX, DE_IDX, HL_IDX };

		std::array<register16_t, 3> m_registers;
		register16_t m_accumulatorFlag;

		bool m_ime;

		bool m_prefixCB;

		bool m_stopped;

		std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
		std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };

		int fetchExecute() {
			return execute(fetchByte());
		}

		uint8_t& R(int r) {
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
				m_memory.ADDRESS() = HL();
				return m_memory.reference();
			case 7:
				return A();
			}
			throw std::logic_error("Unhandled registry mechanism");
		}

		register16_t& RP(int rp) {
			switch (rp) {
			case 3:
				return sp;
			default:
				return m_registers[rp];
			}
		}

		register16_t& RP2(int rp) {
			switch (rp) {
			case 3:
				return AF();
			default:
				return m_registers[rp];
			}
		}

		void adjustHalfCarryAdd(uint8_t before, uint8_t value, int calculation) {
			setFlag(HC, calculateHalfCarryAdd(before, value, calculation));
		}

		void adjustHalfCarrySub(uint8_t before, uint8_t value, int calculation) {
			setFlag(HC, calculateHalfCarrySub(before, value, calculation));
		}

		void executeCB(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		void adjustZero(uint8_t value);

		void postIncrement(uint8_t value);
		void postDecrement(uint8_t value);

		void reti();

		bool jrConditionalFlag(int flag);
		bool returnConditionalFlag(int flag);
		bool jumpConditionalFlag(int flag);
		bool callConditionalFlag(int flag);

		void sbc(register16_t& operand, register16_t value);
		void adc(register16_t& operand, register16_t value);

		void add(register16_t& operand, register16_t value);

		void add(uint8_t& operand, uint8_t value, int carry = 0);
		void adc(uint8_t& operand, uint8_t value);
		void sub(uint8_t& operand, uint8_t value, int carry = 0);
		void sbc(uint8_t& operand, uint8_t value);
		void andr(uint8_t& operand, uint8_t value);
		void xorr(uint8_t& operand, uint8_t value);
		void orr(uint8_t& operand, uint8_t value);
		void compare(uint8_t value);

		void rlc(uint8_t& operand);
		void rrc(uint8_t& operand);
		void rl(uint8_t& operand);
		void rr(uint8_t& operand);
		void sla(uint8_t& operand);
		void sra(uint8_t& operand);
		void srl(uint8_t& operand);

		void bit(int n, uint8_t& operand);
		void res(int n, uint8_t& operand);
		void set(int nit, uint8_t& operand);

		void daa();

		void scf();
		void ccf();
		void cpl();

		void swap(uint8_t& operand);
	};
}