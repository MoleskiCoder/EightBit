#include "stdafx.h"
#include "../inc/InputOutput.h"

namespace EightBit {

// Input port actions

uint8_t InputOutput::read(register16_t port) {
	return readInputPort(port);
}

uint8_t InputOutput::readInputPort(register16_t port) {
	return readInputPort(port.word);
}

uint8_t InputOutput::readInputPort(uint16_t port) {
	ReadingPort.fire(port);
	const auto value = _input[port];
	ReadPort.fire(port);
	return value;
}

void InputOutput::writeInputPort(uint16_t port, uint8_t value) noexcept {
	_input[port] = value;
}

void InputOutput::writeInputPort(register16_t port, uint8_t value) {
	writeInputPort(port.word, value);
}

// Output port actions

void InputOutput::write(register16_t port, uint8_t value) {
	return writeOutputPort(port, value);
}

void InputOutput::writeOutputPort(register16_t port, uint8_t value) {
	writeOutputPort(port.word ,value);
}

void InputOutput::writeOutputPort(uint16_t port, uint8_t value) {
	WritingPort.fire(port);
	_output[port] = value;
	WrittenPort.fire(port);
}

uint8_t InputOutput::readOutputPort(uint16_t port) noexcept {
	return _output[port];
}

uint8_t InputOutput::readOutputPort(register16_t port) {
	return readOutputPort(port.word);
}

}
