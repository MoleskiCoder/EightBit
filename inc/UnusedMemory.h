#pragma once

#include "Memory.h"

namespace EightBit {
	// A read-only Memory implementation that has a fixed size and will
	// *always* returns the same value, from whichever location
	// is being read.
	class UnusedMemory final : public Memory {
	public:
		UnusedMemory(const size_t size, const uint8_t value)
		: m_size(size), m_value(value) {}
		virtual ~UnusedMemory() = default;

		virtual size_t size() const final { return m_size; }
		virtual uint8_t peek(uint16_t address) const final { return m_value; }

		virtual int load(std::ifstream& file, int writeOffset = 0, int readOffset = 0, int limit = -1) final {
			throw new std::logic_error("load operation not allowed.");
		}

		virtual int load(const std::string& path, int writeOffset = 0, int readOffset = 0, int limit = -1) final {
			throw new std::logic_error("load operation not allowed.");
		}
		
		virtual int load(const std::vector<uint8_t>& bytes, int writeOffset = 0, int readOffset = 0, int limit = -1) final {
			throw new std::logic_error("load operation not allowed.");
		}

	protected:
		virtual void poke(uint16_t address, uint8_t value) {
			throw new std::logic_error("Poke operation not allowed.");
		}

	private:
		size_t m_size;
		uint8_t m_value;
	};
}