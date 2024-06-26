#pragma once

#include <cstdint>

#include "Memory.h"
#include "Ram.h"

namespace EightBit {
	class InputOutput final : public Memory {
	public:
		enum class AccessType { Unknown, Reading, Writing };

		[[nodiscard]] uint16_t size() const noexcept override;
		[[nodiscard]] uint8_t peek(uint16_t address) const noexcept override;

		[[nodiscard]] uint8_t& reference(uint16_t address) noexcept override;

		int load(std::ifstream& file, int writeOffset = 0, int readOffset = 0, int limit = -1) override;
		int load(std::string path, int writeOffset = 0, int readOffset = 0, int limit = -1) override;
		int load(const std::vector<uint8_t>& bytes, int writeOffset = 0, int readOffset = 0, int limit = -1) override;

		[[nodiscard]] constexpr const auto& accessType() const noexcept { return m_access; }
		[[nodiscard]] constexpr auto& accessType() noexcept { return m_access; }

		[[nodiscard]] auto readPort(uint8_t port, AccessType access) noexcept {
			accessType() = access;
			return reference(port);
		}

		[[nodiscard]] auto readInputPort(uint8_t port) noexcept { return readPort(port, AccessType::Reading); }
		[[nodiscard]] auto readOutputPort(uint8_t port) noexcept { return readPort(port, AccessType::Writing); }

		void writePort(uint8_t port, uint8_t value, AccessType access) noexcept {
			accessType() = access;
			reference(port) = value;
		}

		void writeInputPort(uint8_t port, uint8_t value) noexcept { writePort(port, value, AccessType::Reading); }
		void writeOutputPort(uint8_t port, uint8_t value) noexcept { writePort(port, value,  AccessType::Writing); }

	protected:
		void poke(uint16_t address, uint8_t value) noexcept override;

	private:
		Ram m_input = 0x100;
		Ram m_output = 0x100;
		uint8_t m_delivered = 0xff;

		AccessType m_access = AccessType::Unknown;
	};
}
