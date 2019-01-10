#pragma once

#include "Chip.h"
#include "EventArgs.h"
#include "Signal.h"

namespace EightBit {
	class ClockedChip : public Chip {
	public:
		~ClockedChip() {};

		Signal<EventArgs> Ticked;

		[[nodiscard]] auto cycles() const noexcept { return m_cycles; }

	protected:
		void resetCycles() noexcept { m_cycles = 0; }
		void tick(const int extra) { for (int i = 0; i < extra; ++i) tick(); }
		void tick() { ++m_cycles;  	Ticked.fire(EventArgs::empty()); }

	private:
		int m_cycles = 0;
	};
}
