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
		const ExpectedTestResult& m_result;

		TestEvents m_expectedEvents;
		TestEvents m_actualEvents;

		bool m_failed = false;
		bool m_unimplemented = false;

		EightBit::Ram m_ram = 0x10000;
		EightBit::InputOutput m_ports;
		EightBit::Z80 m_cpu;

		int m_totalCycles;

		void initialiseRegisters();
		void initialiseMemory();

		void check();
		void checkRegisters();
		void checkMemory();
		void checkEvents();

		void dumpDifference(const std::string& description, uint8_t high, uint8_t low) const;
		void dumpDifference(
			const std::string& highDescription,
			const std::string& lowDescription,
			EightBit::register16_t actual, EightBit::register16_t expected) const;

		void addActualEvent(const std::string& specifier);
		void dumpExpectedEvents() const;
		void dumpActualEvents() const;

		static void dumpEvents(const std::vector<TestEvent>& events);
		static void dumpEvent(const TestEvent& event);

	protected:
		virtual EightBit::MemoryMapping mapping(uint16_t address) final {
			return { m_ram, 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };
		}

	public:
		TestRunner(const Test& test, const ExpectedTestResult& expected);

		void run();
		bool failed() const { return m_failed; }
		bool unimplemented() const { return m_unimplemented; }

		virtual void raisePOWER() final;
		virtual void lowerPOWER() final;

		virtual void initialise() final;
	};
}