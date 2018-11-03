#pragma once

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace EightBit {
	class MemoryInterface {
	public:
		virtual size_t size() const = 0;
		virtual uint8_t peek(uint16_t address) const = 0;

		virtual uint8_t& reference(uint16_t) {
			throw new std::logic_error("Reference operation not allowed.");
		}

		virtual int load(std::ifstream& file, int writeOffset = 0, int readOffset = 0, int limit = -1) = 0;
		virtual int load(const std::string& path, int writeOffset = 0, int readOffset = 0, int limit = -1) = 0;
		virtual int load(const std::vector<uint8_t>& bytes, int writeOffset = 0, int readOffset = 0, int limit = -1) = 0;

	protected:
		virtual void poke(uint16_t address, uint8_t value) = 0;
	};
}
