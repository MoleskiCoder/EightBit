#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <string>

class Symbols
{
public:
	Symbols(std::string path);

	const std::map<uint16_t, std::string>& getLabels() const { return labels; }
	const std::map<uint16_t, std::string>& getConstants() const { return constants; }
	const std::map<std::string, uint16_t>& getScopes() const { return scopes; }
	const std::map<std::string, uint64_t>& getAddresses() const { return addresses; }

private:
	void AssignScopes();
	void AssignSymbols();

	void Parse(std::string path);

	std::map<uint16_t, std::string> labels;
	std::map<uint16_t, std::string> constants;
	std::map<std::string, uint16_t> scopes;
	std::map<std::string, uint64_t> addresses;

	struct kv_pair_t {
		std::map<std::string, std::string> element;
	};

	std::vector<std::string> split(const std::string& input, const std::vector<std::string>& delimiters);

	std::map<std::string, std::map<std::string, kv_pair_t>> parsed;
};
