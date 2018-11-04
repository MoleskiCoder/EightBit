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
		const auto& BYTES() const { return m_bytes; }
		auto& BYTES() { return m_bytes; }

		virtual void poke(const uint16_t address, const uint8_t value) override {
			BYTES()[address] = value;
		}

	public:
		static int load(std::ifstream& file, std::vector<uint8_t>& output, int writeOffset = 0, int readOffset = 0, int limit = -1, int maximumSize = -1);
		static int load(const std::string& path, std::vector<uint8_t>& output, int writeOffset = 0, int readOffset = 0, int limit = -1, int maximumSize = -1);

		Rom(const size_t size = 0) noexcept
		: m_bytes(size) {
		}

		virtual size_t size() const final { return m_bytes.size();  }

		virtual int load(std::ifstream& file, const int writeOffset = 0, const int readOffset = 0, const int limit = -1) final {
			const auto maximumSize = (int)size() - writeOffset;
			return load(file, m_bytes, writeOffset, readOffset, limit, maximumSize);
		}

		virtual int load(const std::string& path, const int writeOffset = 0, const int readOffset = 0, const int limit = -1) final {
			const auto maximumSize = (int)size() - writeOffset;
			return load(path, m_bytes, writeOffset, readOffset, limit, maximumSize);
		}

		virtual int load(const std::vector<uint8_t>& bytes, const int writeOffset = 0, const int readOffset = 0, int limit = -1) final {
			if (limit < 0)
				limit = (int)bytes.size() - readOffset;
			std::copy(bytes.cbegin() + readOffset, bytes.cbegin() + limit, m_bytes.begin() + writeOffset);
			return limit;
		}

		virtual uint8_t peek(const uint16_t address) const final {
			return BYTES()[address];
		}
	};
}
