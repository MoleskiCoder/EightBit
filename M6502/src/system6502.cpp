#include "stdafx.h"
#include "system6502.h"

#include <thread>
#include <functional>

System6502::System6502(ProcessorType level, double processorSpeed, clock_t pollInterval)
:	MOS6502(level),
	memory(MemorySize)
{
	speed = processorSpeed;

	cyclesPerSecond = speed * Mega;     // speed is in MHz
	cyclesPerMillisecond = cyclesPerSecond * Milli;
	cyclesPerInterval = (uint64_t)(cyclesPerMillisecond * pollInterval);

	Starting.connect(std::bind(&System6502::System6502_Starting, this));
	Finished.connect(std::bind(&System6502::System6502_Finished, this));
}

void System6502::Initialise() {
	__super::Initialise();
	memory.ClearLocking();
	memory.ClearMemory();
	intervalCycles = 0;
}

void System6502::Run() {
	Starting.fire(EventArgs());
	__super::Run();
	Finished.fire(EventArgs());
}

uint8_t System6502::GetByte(uint16_t offset) const {
	return memory.GetByte(offset);
}

void System6502::SetByte(uint16_t offset, uint8_t value) {
	memory.SetByte(offset, value);
}

void System6502::Execute(uint8_t cell) {

	auto oldCycles = getCycles();

	CheckPoll();

	// XXXX Fetch byte has already incremented PC.
	auto executingAddress = (uint16_t)(getPC() - 1);

	AddressEventArgs e(executingAddress, cell);
	ExecutingInstruction.fire(e);
	__super::Execute(cell);
	ExecutedInstruction.fire(e);

	auto deltaCycles = getCycles() - oldCycles;
	intervalCycles += deltaCycles;
}

void System6502::CheckPoll() {
	if (intervalCycles >= cyclesPerInterval) {
		intervalCycles -= cyclesPerInterval;
		Throttle();
		Polling.fire(EventArgs());
	}
}

void System6502::System6502_Starting() {
	startTime = std::chrono::high_resolution_clock::now();
	running = true;
}

void System6502::System6502_Finished() {
	running = false;
}

void System6502::Throttle() {
	auto now = std::chrono::high_resolution_clock::now();
	auto elapsed = now - startTime;
	auto timerCurrent = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	auto cyclesAllowed = timerCurrent * cyclesPerMillisecond;
	auto cyclesMismatch = getCycles() - cyclesAllowed;
	if (cyclesMismatch > 0.0) {
		auto delay = cyclesMismatch / cyclesPerMillisecond;
		if (delay > 0) {
			heldCycles += (uint64_t)cyclesMismatch;
			std::this_thread::sleep_for(std::chrono::milliseconds((long long)delay));
		}
	}
}