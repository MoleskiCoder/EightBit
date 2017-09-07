#pragma once

#include "FuseTest.h"
#include "FuseExpectedTestResult.h"

#include <Ram.h>
#include <Bus.h>
#include <InputOutput.h>
#include <Z80.h>

namespace Fuse {
	class TestRunner : public EightBit::Bus {
	private:
		const Test& m_test;
		const ExpectedTestResult& m_expected;

		bool m_failed;
		bool m_unimplemented;

		EightBit::Ram m_ram;
		EightBit::InputOutput m_ports;
		EightBit::Z80 m_cpu;

		void initialise();
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
		virtual uint8_t& reference(uint16_t address, bool& rom) {
			rom = false;
			return m_ram.reference(address);
		}

	public:
		TestRunner(const Test& test, const ExpectedTestResult& expected);

		void run();
		bool failed() const { return m_failed; }
		bool unimplemented() const { return m_unimplemented; }
	};
}