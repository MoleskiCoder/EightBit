#include "stdafx.h"
#include "M6532.h"

#include <cassert>

EightBit::M6532::M6532() noexcept {
}

void EightBit::M6532::tick() {

	if (selected()) {

		// Process interrupts

		if (--m_currentIncrement == 0) {
			m_currentIncrement = m_timerIncrement;
			--m_timerInterval;
		}
		if (m_allowPA7Interrupts && (PA() & 0x80))
			IF() &= 0x40;

		if (m_allowTimerInterrupts && (m_timerInterval == 0))
			IF() &= 0x80;

		const auto read = raised(RW());
		const auto write = lowered(RW());
		assert(read == !write);

		const auto ram = lowered(RS());
		if (ram) {

			auto& cell = RAM().reference(address() & 0x7f);
			read ? data() = cell : cell = data();

		} else {

			const auto a0 = address() & 0b00001;
			const auto a1 = address() & 0b00010;
			const auto a2 = address() & 0b00100;
			const auto a3 = address() & 0b01000;
			const auto a4 = address() & 0b10000;

			const auto portControls = a2 == 0;
			const auto otherControls = a2 == 1;

			if (portControls) {

				switch (a0 | a1) {
				case 0b00:
					// R/W output reg A
					break;
				case 0b01:
					read ? data() = DDRA() : DDRA() = data();
					break;
				case 0b10:
					// R/W output reg B
					break;
				case 0b11:
					read ? data() = DDRB() : DDRB() = data();
					break;
				}

			} else {

				if (read && !a4 && a2) {
					m_allowPA7Interrupts = !a1;
					m_edgeDetection = a0 ? Positive : Negative;
				}

				if (read && a2 && a0)
					data() = IF() & (0x80 & 0x40);

				m_allowTimerInterrupts = !!a3;

				if (write && a4) {
					m_timerInterval = data();
					switch (a1 | a0) {
					case 0b00:
						m_timerIncrement = One;
						break;
					case 0b01:
						m_timerIncrement = Eight;
						break;
					case 0b10:
						m_timerIncrement = SixtyFour;
						break;
					case 0b11:
						m_timerIncrement = OneThousandAndTwentyFour;
						break;
					}
					m_currentIncrement = m_timerIncrement;
				}
			}
		}
	}
}

void EightBit::M6532::initialise() {
}

void EightBit::M6532::reset() {
	PA() = m_dra = m_ddra = 0;	// Zero port A registers
	PB() = m_drb = m_ddrb = 0;	// Zero port B registers
	m_allowTimerInterrupts = m_allowPA7Interrupts = false; // Interrupts are disabled
}
