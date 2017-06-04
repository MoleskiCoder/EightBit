#include "stdafx.h"
#include "Symbols.h"

#include <string>

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

Symbols::Symbols(std::string path) {
	if (!path.empty()) {
		Parse(path);
		AssignSymbols();
		AssignScopes();
	}
}

void Symbols::AssignScopes() {
	auto parsedScopes = parsed["scope"];
	for(auto& parsedScopeElement : parsedScopes) {
		auto& parsedScope = parsedScopeElement.second.element;
		auto name = parsedScope["name"];
		auto trimmedName = name.substr(1, name.length() - 2);
		auto size = parsedScope["size"];
		scopes[trimmedName] = (uint16_t)std::stoi(size);
	}
}

void Symbols::AssignSymbols() {
	auto symbols = parsed["sym"];
	for(auto& symbolElement : symbols) {
		auto& symbol = symbolElement.second.element;
		auto name = symbol["name"];
		auto trimmedName = name.substr(1, name.length() - 2);
		auto value = symbol["val"].substr(2);
		auto number = (uint16_t)std::stoi(value, nullptr, 16);
		auto symbolType = symbol["type"];
		if (symbolType == "lab") {
			labels[number] = trimmedName;
			addresses[trimmedName] = number;
		} else if (symbolType == "equ") {
			constants[number] = trimmedName;
		}
	}
}

void Symbols::Parse(std::string path) {
	std::string line;
	std::ifstream reader(path);
	while (std::getline(reader, line)) {
		auto lineElements = split(line, { " ", "\t" });
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
				if (parsed.find(type) == parsed.end())
					parsed[type] = std::map<std::string, kv_pair_t>();
				auto id = data.element["id"];
				data.element.erase("id");
				parsed[type][id] = data;
			}
		}
	}
}

std::vector<std::string> Symbols::split(const std::string& input, const std::vector<std::string>& delimiters) {
	std::vector<std::string> tokens;
	boost::algorithm::split_regex(
		tokens,
		input,
		boost::regex(boost::join(delimiters, "|"))
	);
	return tokens;
}
