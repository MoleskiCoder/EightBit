#include "stdafx.h"
#include "InputOutput.h"

uint8_t EightBit::InputOutput::readInputPort(uint8_t port) {
	OnReadingPort(port);
	auto value = input[port];
	OnReadPort(port);
	return value;
}

void EightBit::InputOutput::writeOutputPort(uint8_t port, uint8_t value) {
	OnWritingPort(port);
	output[port] = value;
	OnWrittenPort(port);
}

void EightBit::InputOutput::OnReadingPort(uint8_t port) {
	PortEventArgs event(port);
	ReadingPort.fire(event);
}

void EightBit::InputOutput::OnReadPort(uint8_t port) {
	PortEventArgs event(port);
	ReadPort.fire(event);
}

void EightBit::InputOutput::OnWritingPort(uint8_t port) {
	PortEventArgs event(port);
	WritingPort.fire(event);
}

void EightBit::InputOutput::OnWrittenPort(uint8_t port) {
	PortEventArgs event(port);
	WrittenPort.fire(event);
}
