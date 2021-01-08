#pragma once

#include "Chip.h"
#include "EventArgs.h"
#include "Signal.h"

namespace EightBit {
	class ClockedChip : public Chip {
	public:
		~ClockedChip() = default;

		Signal<EventArgs> Ticked;

		void tick(const int extra) { for (int i = 0; i < extra; ++i) tick(); }
		void tick() { ++m_cycles;  	Ticked.fire(); }
		[[nodiscard]] auto cycles() const noexcept { return m_cycles; }

	protected:
		void resetCycles() noexcept { m_cycles = 0; }

	private:
		int m_cycles = 0;
	};
}
