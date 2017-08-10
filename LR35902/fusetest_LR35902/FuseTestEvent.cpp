#include "stdafx.h"
#include "FuseTestEvent.h"

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

void Fuse::TestEvent::write(std::ofstream& file) {

	file << std::dec << cycles;
	file << " " << specifier << " ";

	file << std::hex << std::setfill('0');

	if (specifier == "MR" || specifier == "MW") {
		file << std::setw(4) << address << " " << std::setw(2) << value;
	}
	else if (specifier == "MC" || specifier == "PC") {
		file << std::setw(4) << address;
	}
	else if (specifier == "PR" || specifier == "PW") {
		file << std::setw(4) << address << " " << std::setw(2) << value;
	}
}