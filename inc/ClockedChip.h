#pragma once

#include "Chip.h"
#include "EventArgs.h"
#include "Signal.h"

namespace EightBit {
	class ClockedChip : public Chip {
	public:
		~ClockedChip() = default;

		Signal<EventArgs> Ticked;

		[[nodiscard]] auto cycles() const noexcept { return m_cycles; }

		void tick(int extra);
		void tick();

	protected:
		void resetCycles() noexcept;

	private:
		int m_cycles = 0;
	};
}
