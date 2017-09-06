#pragma once

#include <cstdint>
#include <vector>

namespace EightBit {
	class Memory {
	public:
		Memory(size_t size)
		: m_bytes(size) {
		}

		void load(const std::string& path, uint16_t offset = 0) {
			loadBinary(path, m_bytes, offset, m_bytes.size() - offset);
		}

	protected:
		std::vector<uint8_t> m_bytes;

		uint8_t read(uint16_t address) const {
			return m_bytes[address];
		}

		void write(uint16_t address, uint8_t value) {
			m_bytes[address] = value;
		}

	private:
		static int loadBinary(const std::string& path, std::vector<uint8_t>& output, int offset = 0, int maximumSize = -1);
	};
}