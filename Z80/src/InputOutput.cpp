#include "stdafx.h"
#include "InputOutput.h"

InputOutput::InputOutput() {
}

uint8_t InputOutput::readInputPort(uint8_t port) {
	OnReadingPort(port);
	auto value = input[port];
	OnReadPort(port);
	return value;
}

void InputOutput::writeOutputPort(uint8_t port, uint8_t value) {
	OnWritingPort(port);
	output[port] = value;
	OnWrittenPort(port);
}

void InputOutput::OnReadingPort(uint8_t port) {
	PortEventArgs event(port);
	ReadingPort.fire(event);
}

void InputOutput::OnReadPort(uint8_t port) {
	PortEventArgs event(port);
	ReadPort.fire(event);
}

void InputOutput::OnWritingPort(uint8_t port) {
	PortEventArgs event(port);
	WritingPort.fire(event);
}

void InputOutput::OnWrittenPort(uint8_t port) {
	PortEventArgs event(port);
	WrittenPort.fire(event);
}
