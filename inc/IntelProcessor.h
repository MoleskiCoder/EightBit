#pragma once

#include "Processor.h"

namespace EightBit {
	class IntelProcessor : public Processor
	{
	public:
		virtual ~IntelProcessor();

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
			return m_halfCarryTableAdd[index & 0x7];
		}

		bool calculateHalfCarrySub(uint8_t before, uint8_t value, int calculation) {
			auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableSub[index & 0x7];
		}

		void push(uint8_t value);
		void pushWord(register16_t value);

		uint8_t pop();
		void popWord(register16_t& output);

		void fetchWord() {
			Processor::fetchWord(MEMPTR());
		}

		virtual void call() {
			pushWord(pc);
			pc = MEMPTR();
		}

	private:
		register16_t m_memptr;
	};
}