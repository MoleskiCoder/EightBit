#pragma once

#include <cstdint>
#include <array>

#include "Signal.h"
#include "Memory.h"
#include "Ram.h"

namespace EightBit {
	class InputOutput final : public Memory {
	public:
		enum class AccessType { Unknown, Reading, Writing };

		InputOutput() = default;

		[[nodiscard]] size_t size() const override;
		[[nodiscard]] uint8_t peek(uint16_t address) const override;

		[[nodiscard]] uint8_t& reference(uint16_t address) override;

		int load(std::ifstream& file, int writeOffset = 0, int readOffset = 0, int limit = -1) override;
		int load(const std::string& path, int writeOffset = 0, int readOffset = 0, int limit = -1) override;
		int load(const std::vector<uint8_t>& bytes, int writeOffset = 0, int readOffset = 0, int limit = -1) override;

		AccessType getAccessType() const noexcept { return m_access; }
		void setAccessType(AccessType value) noexcept { m_access = value; }

		auto readPort(uint8_t port, AccessType access) {
			setAccessType(access);
			return reference(port);
		}

		auto readInputPort(uint8_t port) { return readPort(port, AccessType::Reading); }
		auto readOutputPort(uint8_t port) { return readPort(port, AccessType::Writing); }

		void writePort(uint8_t port, uint8_t value, AccessType access) {
			setAccessType(access);
			reference(port) = value;
		}

		auto writeInputPort(uint8_t port, uint8_t value) { return writePort(port, value, AccessType::Reading); }
		auto writeOutputPort(uint8_t port, uint8_t value) { return writePort(port, value,  AccessType::Writing); }

	protected:
		void poke(uint16_t address, uint8_t value) override;

	private:
		Ram m_input = 0x100;
		Ram m_output = 0x100;

		AccessType m_access = AccessType::Unknown;
	};
}
