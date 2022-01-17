#include "stdafx.h"
#include "FuseTestEvent.h"

#include <boost/algorithm/string/predicate.hpp>

#include <Disassembler.h>

bool Fuse::TestEvent::operator==(const TestEvent& rhs) const {
	const auto equalCycles = cycles == rhs.cycles;
	const auto equalSpecifier = specifier == rhs.specifier;
	const auto equalAddress = address == rhs.address;
	const auto equalValue = value == rhs.value;
	return equalCycles && equalSpecifier && equalAddress && equalValue;
}

void Fuse::TestEvent::read(std::ifstream& file) {

	auto prior = file.tellg();

	std::string line;
	std::getline(file, line);
	std::stringstream in(line);

	in >> cycles;
	in >> specifier;

	in >> std::hex;

	valid = true;
	if (specifier == "MR" || specifier == "MW") {
		in >> address;
		in >> value;
	} else if (specifier == "MC" || specifier == "PC") {
		in >> address;
		value = -1;
	} else if (specifier == "PR" || specifier == "PW") {
		in >> address;
		in >> value;
	} else {
		valid = false;
	}

	if (!valid) {
		file.seekg(prior);
	}
}

void Fuse::TestEvent::dump() const {
	std::cerr << " Event issue " <<
		"Cycles = " << cycles <<
		", Specifier = " << specifier <<
		", Address = " << EightBit::Disassembler::hex((uint16_t)address);
	if (!boost::algorithm::ends_with(specifier, "C"))
		std::cerr << ", Value=" << EightBit::Disassembler::hex((uint8_t)value);
	std::cerr << std::endl;
}