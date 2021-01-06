#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <ios>
#include <sstream>
#include <map>
#include <optional>
#include <vector>
#include <utility>

namespace EightBit {
	class IntelHexFile final {
	public:
		IntelHexFile(std::string path);

		[[nodiscard]] std::map<uint16_t, std::vector<uint8_t>> parse();

	private:
		[[nodiscard]] std::optional<std::pair<uint16_t, std::vector<uint8_t>>> parse(std::string line);
		[[nodiscard]] std::vector<uint8_t> parseDataRecord(std::string line, uint8_t count);

		template <class T> [[nodiscard]] T fromHex(std::string input) {
			std::istringstream converter(input);
			unsigned output;
			converter >> std::hex >> output;
			return static_cast<T>(output);
		}

		std::ifstream m_file;
		bool m_eof;
	};
}
