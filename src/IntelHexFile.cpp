#include "stdafx.h"
#include "IntelHexFile.h"

#include <sstream>

EightBit::IntelHexFile::IntelHexFile(std::string path)
:	m_eof(false) {
	m_file.open(path);
}

std::map<uint16_t, std::vector<uint8_t>> EightBit::IntelHexFile::parse() {

	std::map<uint16_t, std::vector<uint8_t>> returned;

	while (!m_file.eof() && !m_eof) {

		std::string line;
		std::getline(m_file, line);

		const auto parsed = parse(line);
		if (parsed.has_value())
			returned[parsed.value().first] = parsed.value().second;
	}

	if (!m_eof)
		throw std::out_of_range("File is missing an EOF record");

	return returned;
}

std::optional<std::pair<uint16_t, std::vector<uint8_t>>> EightBit::IntelHexFile::parse(std::string line) {

	const auto colon = line.substr(0, 1);
	if (colon != ":")
		throw std::out_of_range("Invalid hex file: line does not begin with a colon");

	const auto countString = line.substr(1, 2);
	const auto count = fromHex<uint8_t>(countString);

	const auto addressString = line.substr(3, 4);
	const auto address = fromHex<uint16_t>(addressString);

	const auto recordTypeString = line.substr(7, 2);
	const auto recordType = fromHex<uint8_t>(recordTypeString);

	switch (recordType) {
	case 0x00:
		return std::make_pair(address, parseDataRecord(line, count));
	case 0x01:
		m_eof = true;
		return {};
	default:
		throw std::out_of_range("Unhandled hex file record.");
	}
}

std::vector<uint8_t> EightBit::IntelHexFile::parseDataRecord(std::string line, uint8_t count) {
	std::vector<uint8_t> data(count);
	const auto requiredLength = 9 + 2 + (count * 2);
	if (line.length() != requiredLength)
		throw std::out_of_range("Invalid hex file: line is not the required length");
	for (int i = 0; i < count; ++i) {
		const auto position = 9 + i * 2;
		const auto datumString = line.substr(position, 2);
		const auto datum = fromHex<uint8_t>(datumString);
		data.at(i) = datum;
	}
	return data;
}
