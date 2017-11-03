#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace EightBit {
	class Memory {
	public:
		Memory(size_t size = 0)
		: m_bytes(size) {
		}

		size_t size() const { return m_bytes.size();  }

		int load(const std::string& path, int writeOffset = 0, int readOffset = 0, int limit = -1) {
			const auto maximumSize = (int)m_bytes.size() - writeOffset;
			return loadBinary(path, m_bytes, writeOffset, readOffset, limit, maximumSize);
		}

	protected:
		std::vector<uint8_t>& BYTES() { return m_bytes; }

		uint8_t read(uint16_t address) const {
			return m_bytes[address];
		}

		void write(uint16_t address, uint8_t value) {
			m_bytes[address] = value;
		}

	private:
		std::vector<uint8_t> m_bytes;

		static int loadBinary(
			const std::string& path,
			std::vector<uint8_t>& output,
			int writeOffset,
			int readOffset,
			int limit,
			int maximumSize);
	};
}