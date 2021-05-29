#include "stdafx.h"
#include "../inc/M6532.h"

#include <cassert>

EightBit::M6532::M6532() noexcept {
	Ticked.connect([this](EightBit::EventArgs&) {
		step();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(RES, M6532);
DEFINE_PIN_LEVEL_CHANGERS(RW, M6532);
DEFINE_PIN_LEVEL_CHANGERS(IRQ, M6532);
DEFINE_PIN_LEVEL_CHANGERS(RS, M6532);
DEFINE_PIN_LEVEL_CHANGERS(CS1, M6532);
DEFINE_PIN_LEVEL_CHANGERS(CS2, M6532);

void EightBit::M6532::step() {

	resetCycles();

	if (!activated())
		return;

	Accessing.fire();

	if (lowered(RES())) {
		reset();
		raise(RES());
		return;
	}

	if (--m_currentIncrement == 0) {
		m_currentIncrement = m_timerIncrement;
		--m_timerInterval;
	}

	const bool interruptPA7 = m_allowPA7Interrupts && (PA() & Bit7);
	if (interruptPA7)
		IF() = setBit(IF(), Bit6);

	const bool interruptTimer = m_allowTimerInterrupts && (m_timerInterval == 0);
	if (interruptTimer)
		IF() = setBit(IF(), Bit7);

	match(IRQ(), !(interruptPA7 || interruptTimer));

	const auto read = raised(RW());
	const auto write = lowered(RW());
	assert(read == !write);

	const auto ram = lowered(RS());
	if (ram) {

		auto& cell = RAM().reference(address() & 0x7f);
		read ? DATA() = cell : cell = DATA();

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
				read ? DATA() = DDRA() : DDRA() = DATA();
				break;
			case 0b10:
				// R/W output reg B
				break;
			case 0b11:
				read ? DATA() = DDRB() : DDRB() = DATA();
				break;
			}

		} else {

			if (read && !a4 && a2) {
				m_allowPA7Interrupts = !a1;
				m_edgeDetection = a0 ? Positive : Negative;
			}

			if (read && a2 && a0) {
				DATA() = IF();
				IF() = clearBit(IF(), Bit6);
			}

			m_allowTimerInterrupts = !!a3;

			if (write && a4) {
				m_timerInterval = DATA();
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
				IF() = clearBit(IF(), Bit7);
			}
		}
	}

	Accessed.fire();
}

void EightBit::M6532::reset() {
	PA() = m_dra = m_ddra = 0;	// Zero port A registers
	PB() = m_drb = m_ddrb = 0;	// Zero port B registers
	m_allowTimerInterrupts = m_allowPA7Interrupts = false; // Interrupts are disabled
}
