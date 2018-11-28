#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

#include "Memory.h"

namespace EightBit {
	// ROM is a basic implementation of the Memory interface.
	// Nothing over and above the interface is exposed to users
	// of the ROM class.
	class Rom : public Memory {
	private:
		std::vector<uint8_t> m_bytes;

	protected:
		const auto& BYTES() const noexcept { return m_bytes; }
		auto& BYTES() noexcept { return m_bytes; }

		void poke(uint16_t address, uint8_t value) override;

	public:
		static int load(std::ifstream& file, std::vector<uint8_t>& output, int writeOffset = 0, int readOffset = 0, int limit = -1, int maximumSize = -1);
		static int load(const std::string& path, std::vector<uint8_t>& output, int writeOffset = 0, int readOffset = 0, int limit = -1, int maximumSize = -1);

		Rom(size_t size = 0);

		size_t size() const final;

		int load(std::ifstream& file, int writeOffset = 0, int readOffset = 0, int limit = -1) final;
		int load(const std::string& path, int writeOffset = 0, int readOffset = 0, int limit = -1) final;
		int load(const std::vector<uint8_t>& bytes, int writeOffset = 0, int readOffset = 0, int limit = -1) final;

		uint8_t peek(uint16_t address) const final;
	};
}
