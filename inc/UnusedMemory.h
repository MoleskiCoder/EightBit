#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "Memory.h"

namespace EightBit {
	// A read-only Memory implementation that has a fixed size and will
	// *always* returns the same value, from whichever location
	// is being read.
	class UnusedMemory final : public Memory {
	public:
		UnusedMemory(uint16_t size, uint8_t value) noexcept;

		[[nodiscard]] uint16_t size() const noexcept final;
		[[nodiscard]] uint8_t peek(uint16_t address) const noexcept final;

		int load(std::ifstream& file, int writeOffset = 0, int readOffset = 0, int limit = -1) final;
		int load(std::string path, int writeOffset = 0, int readOffset = 0, int limit = -1) final;
		int load(const std::vector<uint8_t>& bytes, int writeOffset = 0, int readOffset = 0, int limit = -1) final;

	protected:
		void poke(uint16_t address, uint8_t value) noexcept final;

	private:
		uint16_t m_size;
		uint8_t m_value;
	};
}