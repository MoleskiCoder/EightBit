#include "pch.h"
#include "../inc/MC6850.h"

#include <cassert>

EightBit::mc6850::mc6850() {

	Ticked.connect([this](EightBit::EventArgs&) {
		step();
	});

	RaisedPOWER.connect([this](EventArgs) {
		m_startup = ColdStart;
	});
}

DEFINE_PIN_LEVEL_CHANGERS(RXDATA, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(TXDATA, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(RTS, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(CTS, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(DCD, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(RXCLK, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(TXCLK, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(CS0, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(CS1, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(CS2, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(RS, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(RW, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(E, mc6850);
DEFINE_PIN_LEVEL_CHANGERS(IRQ, mc6850);

void EightBit::mc6850::reset() {
	if (m_startup == ColdStart) {
		raise(IRQ());
		raise(RTS());
	}
	m_statusRDRF = false;
	m_statusTDRE = true;
	m_statusOVRN = false;
	lower(DCD());
	m_startup = WarmStart;
	m_statusRead = false;
}

void EightBit::mc6850::step() {

	resetCycles();

	if (!activated())
		return;

	Accessing.fire();

	const bool writing = lowered(RW());
	//const bool reading = !writing;

	const bool registers = lowered(RS());
	//const bool transferring = !registers;

	if (registers) {
		if (writing) {
			m_counterDivide = (CounterDivideSelect)(DATA() & (CR0 | CR1));
			if (m_counterDivide == MasterReset) {
				reset();
			} else {
				m_wordSelect = (WordSelect)((DATA() & (CR2 | CR3 | CR4)) >> 2);
				m_transmitControl = (TransmitterControl)((DATA() & (CR5 | CR6)) >> 5);
				m_receiveControl = (ReceiveControl)((DATA() & CR7) >> 7);
				match(RTS(), transmitReadyHigh());
			}
		} else {
			DATA() = status();
		}
	} else {
		raise(IRQ());
		if (writing)
			startTransmit();
		else
			completeReceive();
	}

	// Catch the transition to lost carrier
	if (lowered(m_oldDCD) && raised(DCD())) {
		raise(IRQ());
		m_statusRead = false;
	}
	m_oldDCD = DCD();

	Accessed.fire();
}

uint8_t EightBit::mc6850::status() {
	uint8_t status = 0;
	status = setBit(status, STATUS_RDRF, m_statusRDRF);
	status = setBit(status, STATUS_TDRE, m_statusTDRE);
	status = setBit(status, STATUS_DCD, raised(DCD()));
	status = setBit(status, STATUS_CTS, raised(CTS()));
	status = clearBit(status, STATUS_FE);
	status = setBit(status, STATUS_OVRN, m_statusOVRN);
	status = clearBit(status, STATUS_PE);
	status = setBit(status, STATUS_IRQ, lowered(IRQ()));
	return status;
}

std::string EightBit::mc6850::dumpStatus() {
	const auto value = status();
	std::string returned;
	returned += "(";
	returned += (value & STATUS_IRQ) ? "IRQ " : "- ";
	returned += (value & STATUS_PE) ? "PE " : "- ";
	returned += (value & STATUS_OVRN) ? "OVRN " : "- ";
	returned += (value & STATUS_FE) ? "FE " : "- ";
	returned += (value & STATUS_CTS) ? "CTS " : "- ";
	returned += (value & STATUS_DCD) ? "DCD " : "- ";
	returned += (value & STATUS_TDRE) ? "TDRE " : "- ";
	returned += (value & STATUS_RDRF) ? "RDRF " : "- ";
	returned += ") ";
	return returned;
}

void EightBit::mc6850::markTransmitComplete() {
	m_statusTDRE = lowered(CTS());
	if (m_statusTDRE && transmitInterruptEnabled())
		lower(IRQ());
	Transmitted.fire();
}

void EightBit::mc6850::markReceiveStarting() {
	m_statusOVRN = m_statusRDRF;	// If the RDR was already full, this means we're losing data
	m_statusRDRF = true;
	if (receiveInterruptEnabled())
		lower(IRQ());
	Receiving.fire();
}

void EightBit::mc6850::startTransmit() {
	TDR() = DATA();
	m_statusTDRE = false;
	Transmitting.fire();
}

void EightBit::mc6850::completeReceive() {
	DATA() = RDR();
	m_statusOVRN = m_statusRDRF = false;	// Any existing data overrun is now moot.
	Received.fire();
}
