#pragma once

#include "Processor.h"

namespace EightBit {
	class IntelProcessor : public Processor
	{
	public:
		register16_t& MEMPTR() { return m_memptr; }

		virtual void initialise();

	protected:
		IntelProcessor(Memory& memory);

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

		uint8_t getViaMemptr() {
			m_memory.ADDRESS() = MEMPTR();
			MEMPTR().word++;
			return m_memory.reference();
		}

		void setViaMemptr(uint8_t value) {
			m_memory.ADDRESS() = MEMPTR();
			MEMPTR().word++;
			m_memory.reference() = value;
			MEMPTR().high = value;
		}

		void getWordViaMemptr(register16_t& value) {
			m_memory.ADDRESS() = MEMPTR();
			MEMPTR().word++;
			value.low = m_memory.reference();
			m_memory.ADDRESS().word++;
			value.high = m_memory.reference();
		}

		void setWordViaMemptr(register16_t value) {
			m_memory.ADDRESS() = MEMPTR();
			MEMPTR().word++;
			m_memory.reference() = value.low;
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