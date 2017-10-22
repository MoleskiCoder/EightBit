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
	ReadingPort.fire(port);
}

void EightBit::InputOutput::OnReadPort(uint8_t port) {
	ReadPort.fire(port);
}

void EightBit::InputOutput::OnWritingPort(uint8_t port) {
	WritingPort.fire(port);
}

void EightBit::InputOutput::OnWrittenPort(uint8_t port) {
	WrittenPort.fire(port);
}
