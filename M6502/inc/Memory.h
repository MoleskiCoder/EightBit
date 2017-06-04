#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "Signal.h"
#include "AddressEventArgs.h"

class Memory
{
private:
	std::vector<uint8_t> memory;
	std::vector<bool> locked;

public:
	Memory(unsigned memorySize)
	:	memory(memorySize),
		locked(memorySize) {}

	Signal<AddressEventArgs> InvalidWriteAttempt;
	Signal<AddressEventArgs> WritingByte;
	Signal<AddressEventArgs> ReadingByte;

	void ClearMemory() {
		std::fill(memory.begin(), memory.end(), 0);
	}

	void ClearLocking() {
		std::fill(locked.begin(), locked.end(), false);
	}

	uint8_t GetByte(uint16_t offset) const {
		auto content = memory[offset];
		ReadingByte.fire(AddressEventArgs(offset, content));
		return content;
	}

	void SetByte(uint16_t offset, uint8_t value) {
		AddressEventArgs e(offset, value);
		if (locked[offset]) {
			InvalidWriteAttempt.fire(e);
		} else {
			memory[offset] = value;
			WritingByte.fire(e);
		}
	}

	void LoadRom(std::string path, uint16_t offset) {
		auto length = LoadMemory(path, offset);
		LockMemory(offset, length);
	}

	void LoadRam(std::string path, uint16_t offset) {
		LoadMemory(path, offset);
	}

	void LockMemory(uint16_t offset, uint16_t length) {
		for (auto i = 0; i < length; ++i)
			locked[offset + i] = true;
	}

	uint16_t LoadMemory(std::string path, uint16_t offset) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		auto size = (int)file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<char> buffer(size);
		file.read(&buffer[0], size);
		file.close();

		std::copy(buffer.begin(), buffer.end(), memory.begin() + offset);

		return (uint16_t)size;
	}
};
