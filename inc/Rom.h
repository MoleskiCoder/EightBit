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
		[[nodiscard]] constexpr const auto& BYTES() const noexcept { return m_bytes; }
		[[nodiscard]] constexpr auto& BYTES() noexcept { return m_bytes; }

		void poke(uint16_t address, uint8_t value) noexcept override;

	public:
		static int load(std::ifstream& file, std::vector<uint8_t>& output, int writeOffset = 0, int readOffset = 0, int limit = -1, int maximumSize = -1);
		static int load(std::string path, std::vector<uint8_t>& output, int writeOffset = 0, int readOffset = 0, int limit = -1, int maximumSize = -1);

		Rom(size_t size = 0) noexcept;
		Rom(const Rom& rhs);
		Rom& operator=(const Rom& rhs);
		bool operator==(const Rom& rhs) const;

		[[nodiscard]] uint16_t size() const noexcept final;

		int load(std::ifstream& file, int writeOffset = 0, int readOffset = 0, int limit = -1) final;
		int load(std::string path, int writeOffset = 0, int readOffset = 0, int limit = -1) final;
		int load(const std::vector<uint8_t>& bytes, int writeOffset = 0, int readOffset = 0, int limit = -1) final;

		template <typename Iter>
		int load(Iter start, Iter end, int writeOffset = 0, int readOffset = 0, int limit = -1) {

			const auto size = end - start;
			if (limit < 0)
				limit = static_cast<int>(size - readOffset);

			const size_t extent = static_cast<size_t>(limit) + writeOffset;
			if (m_bytes.size() < extent)
				m_bytes.resize(extent);

			std::copy(start + readOffset, start + readOffset + limit, m_bytes.begin() + writeOffset);

			return limit;
		}

		[[nodiscard]] uint8_t peek(uint16_t address) const noexcept final;
	};
}
