#pragma once

#include <Bus.h>
#include <LR35902.h>

#include "FuseTest.h"
#include "FuseExpectedTestResult.h"

#include <LR35902.h>
#include <GameBoyBus.h>
#include <Ram.h>

namespace Fuse {
	class TestRunner : public EightBit::GameBoy::Bus {
	private:
		const Test& m_test;
		const ExpectedTestResult& m_expected;

		bool m_failed = false;
		bool m_unimplemented = false;

		EightBit::Ram m_ram = 0x10000;

		void initialiseRegisters();
		void initialiseMemory();

		void check();
		void checkregisters();
		void checkMemory();

		void dumpDifference(const std::string& description, uint8_t high, uint8_t low) const;
		void dumpDifference(
			const std::string& highDescription,
			const std::string& lowDescription,
			EightBit::register16_t actual, EightBit::register16_t expected) const;

	protected:
		virtual EightBit::MemoryMapping mapping(uint16_t address) noexcept final {
			return { m_ram, 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };
		}

	public:
		TestRunner(const Test& test, const ExpectedTestResult& expected);

		void run();
		bool failed() const { return m_failed; }
		bool unimplemented() const { return m_unimplemented; }

		void raisePOWER() noexcept final;
		void lowerPOWER() noexcept final;

		void initialise();
	};
}