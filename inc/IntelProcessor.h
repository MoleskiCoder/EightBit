#pragma once

#include <cstdint>
#include <array>

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

		static void clearFlag(uint8_t& f, int flag) { f &= ~flag; }
		static void setFlag(uint8_t& f, int flag) { f |= flag; }

		static void setFlag(uint8_t& f, int flag, int condition) { setFlag(f, flag, condition != 0); }
		static void setFlag(uint8_t& f, int flag, uint32_t condition) { setFlag(f, flag, condition != 0); }
		static void setFlag(uint8_t& f, int flag, bool condition) { condition ? setFlag(f, flag) : clearFlag(f, flag); }

		static void clearFlag(uint8_t& f, int flag, int condition) { clearFlag(f, flag, condition != 0); }
		static void clearFlag(uint8_t& f, int flag, uint32_t condition) { clearFlag(f, flag, condition != 0); }
		static void clearFlag(uint8_t& f, int flag, bool condition) { condition ? clearFlag(f, flag) : setFlag(f, flag); }

		static int buildHalfCarryIndex(uint8_t before, uint8_t value, int calculation) {
			return ((before & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
		}

		static bool calculateHalfCarryAdd(uint8_t before, uint8_t value, int calculation) {
			static std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
			auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableAdd[index & Mask3];
		}

		static bool calculateHalfCarrySub(uint8_t before, uint8_t value, int calculation) {
			std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };
			auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableSub[index & Mask3];
		}

		void push(uint8_t value) {
			m_memory.ADDRESS().word = --SP().word;
			m_memory.reference() = value;
		}

		void pushWord(register16_t value) {
			push(value.high);
			push(value.low);
		}

		uint8_t pop() {
			m_memory.ADDRESS().word = SP().word++;
			return m_memory.reference();
		}

		void popWord(register16_t& output) {
			output.low = pop();
			output.high = pop();
		}

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
			PC() = MEMPTR();
		}

		void call() {
			pushWord(PC());
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
			MEMPTR().word = PC().word + offset;
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