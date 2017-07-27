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

		virtual int execute(uint8_t opcode);
		int step();

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

	protected:
		virtual uint8_t fetchByte() override {
			auto returned = IntelProcessor::fetchByte();
			m_memory.fireReadBusEvent();
			return returned;
		}

		virtual void push(uint8_t value) override {
			IntelProcessor::push(value);
			m_memory.fireWriteBusEvent();
		}

		virtual uint8_t pop() override {
			auto returned = IntelProcessor::pop();
			m_memory.fireReadBusEvent();
			return returned;
		}

		virtual void getWordViaMemptr(register16_t& value) {
			value.low = memptrReference();
			m_memory.fireReadBusEvent();
			m_memory.ADDRESS().word++;
			value.high = m_memory.reference();
			m_memory.fireReadBusEvent();
		}

		virtual void setWordViaMemptr(register16_t value) {
			memptrReference() = value.low;
			m_memory.fireWriteBusEvent();
			m_memory.ADDRESS().word++;
			m_memory.reference() = value.high;
			m_memory.fireWriteBusEvent();
		}

	private:
		Bus& m_bus;

		register16_t af;
		register16_t bc;
		register16_t de;
		register16_t hl;

		bool m_ime;

		bool m_prefixCB;

		bool m_stopped;

		uint8_t& R(int r, uint8_t& a) {
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
				return a;
			}
			throw std::logic_error("Unhandled registry mechanism");
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

		static void rlc(uint8_t& f, uint8_t& operand);
		static void rrc(uint8_t& f, uint8_t& operand);
		static void rl(uint8_t& f, uint8_t& operand);
		static void rr(uint8_t& f, uint8_t& operand);
		static void sla(uint8_t& f, uint8_t& operand);
		static void sra(uint8_t& f, uint8_t& operand);
		static void srl(uint8_t& f, uint8_t& operand);

		static void bit(uint8_t& f, int n, uint8_t& operand);
		static void res(int n, uint8_t& operand);
		static void set(int n, uint8_t& operand);

		static void daa(uint8_t& a, uint8_t& f);

		static void scf(uint8_t& a, uint8_t& f);
		static void ccf(uint8_t& a, uint8_t& f);
		static void cpl(uint8_t& a, uint8_t& f);

		static void swap(uint8_t& f, uint8_t& operand);
	};
}