#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <string>

namespace EightBit {
	class Symbols {
	public:
		Symbols(std::string path = "") noexcept;

		const std::map<uint16_t, std::string>& labels() const { return m_labels; }
		const std::map<uint16_t, std::string>& constants() const { return m_constants; }
		const std::map<std::string, uint16_t>& scopes() const { return m_scopes; }
		const std::map<std::string, uint64_t>& addresses() const { return m_addresses; }

	private:
		static std::vector<std::string> split(const std::string& input, const std::string& regex);

		void assignScopes();
		void assignSymbols();

		void parse(std::string path);

		std::map<uint16_t, std::string> m_labels;
		std::map<uint16_t, std::string> m_constants;
		std::map<std::string, uint16_t> m_scopes;
		std::map<std::string, uint64_t> m_addresses;

		struct kv_pair_t {
			std::map<std::string, std::string> element;
		};
		std::map<std::string, std::map<std::string, kv_pair_t>> m_parsed;
	};
}
