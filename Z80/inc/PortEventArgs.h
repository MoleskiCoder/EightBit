#pragma once

#include <cstdint>

class PortEventArgs {
public:
	PortEventArgs(uint8_t port)
	: m_port(port) {}

	uint8_t getPort() const {
		return m_port;
	}

private:
	uint8_t m_port;
};
