#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace EightBit {
	// Memory is:
	// *) Definitely has a size
	// *) Probably 'peek'able (although you might not like the answer you get!)
	// *) Probably 'load'able (i.e. able to be externally initialised)
	// *) At the implementation level, probably 'poke'able (although may not be exposed to users)
	// *) Possibly 'reference'able (Very likely if you've exposed 'poke')
	class Memory {
	public:
		virtual ~Memory() = default;

		[[nodiscard]] virtual size_t size() const = 0;
		[[nodiscard]] virtual uint8_t peek(uint16_t address) const = 0;

		[[nodiscard]] virtual uint8_t& reference(uint16_t);

		virtual int load(std::ifstream& file, int writeOffset = 0, int readOffset = 0, int limit = -1) = 0;
		virtual int load(const std::string& path, int writeOffset = 0, int readOffset = 0, int limit = -1) = 0;
		virtual int load(const std::vector<uint8_t>& bytes, int writeOffset = 0, int readOffset = 0, int limit = -1) = 0;

	protected:
		virtual void poke(uint16_t address, uint8_t value) = 0;
	};
}
