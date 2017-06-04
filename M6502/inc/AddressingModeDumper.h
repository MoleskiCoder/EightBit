#pragma once

#include <cstdint>
#include <functional>

typedef std::function<std::string(uint16_t current)> dumper_t;

struct AddressingModeDumper {
	dumper_t byteDumper;
	dumper_t disassemblyDumper;
};
