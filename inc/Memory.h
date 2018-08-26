#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

namespace EightBit {
	class Memory {
	public:
		static int load(std::ifstream& file, std::vector<uint8_t>& output, int writeOffset = 0, int readOffset = 0, int limit = -1, int maximumSize = -1);
		static int load(const std::string& path, std::vector<uint8_t>& output, int writeOffset = 0, int readOffset = 0, int limit = -1, int maximumSize = -1);

		Memory(const size_t size = 0) noexcept
		: m_bytes(size) {
		}

		size_t size() const { return m_bytes.size();  }

		int load(std::ifstream& file, const int writeOffset = 0, const int readOffset = 0, const int limit = -1) {
			const auto maximumSize = (int)m_bytes.size() - writeOffset;
			return load(file, m_bytes, writeOffset, readOffset, limit, maximumSize);
		}

		int load(const std::string& path, const int writeOffset = 0, const int readOffset = 0, const int limit = -1) {
			const auto maximumSize = (int)m_bytes.size() - writeOffset;
			return load(path, m_bytes, writeOffset, readOffset, limit, maximumSize);
		}

		int load(const std::vector<uint8_t>& bytes, const int writeOffset = 0, const int readOffset = 0, int limit = -1) {
			if (limit < 0)
				limit = (int)bytes.size() - readOffset;
			std::copy(bytes.cbegin() + readOffset, bytes.cbegin() + limit, m_bytes.begin() + writeOffset);
			return limit;
		}

		uint8_t peek(const uint16_t address) const {
			return BYTES()[address];
		}

	protected:
		const std::vector<uint8_t>& BYTES() const { return m_bytes; }
		std::vector<uint8_t>& BYTES() { return m_bytes; }

		void poke(const uint16_t address, const uint8_t value) {
			BYTES()[address] = value;
		}

	private:
		std::vector<uint8_t> m_bytes;
	};

	typedef Memory Rom;
}
