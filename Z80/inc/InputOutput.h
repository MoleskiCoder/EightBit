#pragma once

#include <cstdint>
#include "Signal.h"
#include "PortEventArgs.h"

class InputOutput {
public:
	InputOutput();

	uint8_t read(uint8_t port) { return readInputPort(port); }
	void write(uint8_t port, uint8_t value) { return writeOutputPort(port, value); }

	uint8_t readInputPort(uint8_t port);
	void writeInputPort(uint8_t port, uint8_t value) { input[port] = value;  }

	uint8_t readOutputPort(uint8_t port) { return output[port]; }
	void writeOutputPort(uint8_t port, uint8_t value);

	Signal<PortEventArgs> ReadingPort;
	Signal<PortEventArgs> ReadPort;

	Signal<PortEventArgs> WritingPort;
	Signal<PortEventArgs> WrittenPort;

protected:
	void OnReadingPort(uint8_t port);
	void OnReadPort(uint8_t port);

	void OnWritingPort(uint8_t port);
	void OnWrittenPort(uint8_t port);

private:
	std::array<uint8_t, 0x100> input;
	std::array<uint8_t, 0x100> output;
};
