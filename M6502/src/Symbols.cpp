#include "stdafx.h"
#include "../inc/Symbols.h"

#include <fstream>
#include <regex>
#include <string>

EightBit::Symbols::Symbols(std::string path) noexcept {
	if (!path.empty()) {
		parse(path);
		assignSymbols();
		assignScopes();
	}
}

void EightBit::Symbols::assignScopes() {
	auto parsedScopes = m_parsed["scope"];
	for(auto& parsedScopeElement : parsedScopes) {
		auto& parsedScope = parsedScopeElement.second.element;
		auto name = parsedScope["name"];
		auto trimmedName = name.substr(1, name.length() - 2);
		auto size = parsedScope["size"];
		m_scopes[trimmedName] = (uint16_t)std::stoi(size);
	}
}

void EightBit::Symbols::assignSymbols() {
	auto symbols = m_parsed["sym"];
	for(auto& symbolElement : symbols) {
		auto& symbol = symbolElement.second.element;
		auto name = symbol["name"];
		auto trimmedName = name.substr(1, name.length() - 2);
		auto value = symbol["val"].substr(2);
		auto number = (uint16_t)std::stoi(value, nullptr, 16);
		auto symbolType = symbol["type"];
		if (symbolType == "lab") {
			m_labels[number] = trimmedName;
			m_addresses[trimmedName] = number;
		} else if (symbolType == "equ") {
			m_constants[number] = trimmedName;
		}
	}
}

void EightBit::Symbols::parse(std::string path) {
	std::string line;
	std::ifstream reader(path);
	while (std::getline(reader, line)) {
		auto lineElements = split(line, " |\t");
		if (lineElements.size() == 2) {
			auto type = lineElements[0];
			auto dataElements = split(lineElements[1], { "," });
			kv_pair_t data;
			for (auto& dataElement : dataElements) {
				auto definition = split(dataElement, { "=" });
				if (definition.size() == 2)
					data.element[definition[0]] = definition[1];
			}

			if (data.element.find("id") != data.element.end()) {
				if (m_parsed.find(type) == m_parsed.end())
					m_parsed[type] = std::map<std::string, kv_pair_t>();
				auto id = data.element["id"];
				data.element.erase("id");
				m_parsed[type][id] = data;
			}
		}
	}
}

std::vector<std::string> EightBit::Symbols::split(const std::string& input, const std::string& regex) {
    std::regex re(regex);
    std::sregex_token_iterator
        first { input.begin(), input.end(), re, -1 },
        last;
    return { first, last };
}
