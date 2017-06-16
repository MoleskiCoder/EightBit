#pragma once

#include "Processor.h"

namespace EightBit {
	class IntelProcessor : public Processor
	{
	public:
		register16_t& MEMPTR() { return m_memptr; }

		virtual void initialise();

		virtual register16_t& AF() = 0;
		uint8_t& A() { return AF().high; }
		uint8_t& F() { return AF().low; }

		virtual register16_t& BC() = 0;
		uint8_t& B() { return BC().high; }
		uint8_t& C() { return BC().low; }

		virtual register16_t& DE() = 0;
		uint8_t& D() { return DE().high; }
		uint8_t& E() { return DE().low; }

		virtual register16_t& HL() = 0;
		uint8_t& H() { return HL().high; }
		uint8_t& L() { return HL().low; }

	protected:
		IntelProcessor(Memory& memory);

		void clearFlag(int flag) { F() &= ~flag; }
		void setFlag(int flag) { F() |= flag; }

		void setFlag(int flag, int condition) { setFlag(flag, condition != 0); }
		void setFlag(int flag, uint32_t condition) { setFlag(flag, condition != 0); }
		void setFlag(int flag, bool condition) { condition ? setFlag(flag) : clearFlag(flag); }

		void clearFlag(int flag, int condition) { clearFlag(flag, condition != 0); }
		void clearFlag(int flag, uint32_t condition) { clearFlag(flag, condition != 0); }
		void clearFlag(int flag, bool condition) { condition ? clearFlag(flag) : setFlag(flag); }

		std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
		std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };

		int buildHalfCarryIndex(uint8_t before, uint8_t value, int calculation) {
			return ((before & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
		}

		bool calculateHalfCarryAdd(uint8_t before, uint8_t value, int calculation) {
			auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableAdd[index & Mask3];
		}

		bool calculateHalfCarrySub(uint8_t before, uint8_t value, int calculation) {
			auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableSub[index & Mask3];
		}

		void push(uint8_t value);
		void pushWord(register16_t value);

		uint8_t pop();
		void popWord(register16_t& output);

		void fetchWord() {
			Processor::fetchWord(MEMPTR());
		}

		//

		uint8_t& memptrReference() {
			m_memory.ADDRESS() = MEMPTR();
			MEMPTR().word++;
			return m_memory.reference();
		}

		void getWordViaMemptr(register16_t& value) {
			value.low = memptrReference();
			m_memory.ADDRESS().word++;
			value.high = m_memory.reference();
		}

		void setWordViaMemptr(register16_t value) {
			memptrReference() = value.low;
			m_memory.ADDRESS().word++;
			m_memory.reference() = value.high;
		}

		//

		void jump() {
			pc = MEMPTR();
		}

		void call() {
			pushWord(pc);
			jump();
		}

		void restart(uint8_t address) {
			MEMPTR().low = address;
			MEMPTR().high = 0;
			call();
		}

		bool callConditional(int condition) {
			fetchWord();
			if (condition)
				call();
			return condition != 0;
		}

		bool jumpConditional(int conditional) {
			fetchWord();
			if (conditional)
				jump();
			return conditional != 0;
		}

		void ret() {
			popWord(MEMPTR());
			jump();
		}

		bool returnConditional(int condition) {
			if (condition)
				ret();
			return condition != 0;
		}

		void jr(int8_t offset) {
			MEMPTR().word = pc.word + offset;
			jump();
		}

		bool jrConditional(int conditional) {
			auto offset = fetchByte();
			if (conditional)
				jr(offset);
			return conditional != 0;
		}

	private:
		register16_t m_memptr;
	};
}