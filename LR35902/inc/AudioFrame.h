#pragma once

#include <Signal.h>

namespace EightBit {
	namespace GameBoy {
		class AudioFrame final {
		private:
			enum { StepCycle = 7, Frequency = 512 };

			int m_cyclesPerTick;
			int m_currentStep;
			int m_currentCycles;

		public:
			AudioFrame(int cyclesPerSecond)
			: m_cyclesPerTick(cyclesPerSecond / Frequency),
			  m_currentStep(0),
			  m_currentCycles(m_cyclesPerTick) {
			}

			Signal<int> FrameStep;
			Signal<int> LengthStep;
			Signal<int> VolumeStep;
			Signal<int> SweepStep;

			void step() {
				m_currentStep = (m_currentStep + 1) % StepCycle;
				FrameStep.fire(m_currentStep);
				if ((m_currentStep % 2) == 0)
					LengthStep.fire(m_currentStep);
				if (m_currentStep == 7)
					VolumeStep.fire(m_currentStep);
				if ((m_currentStep == 2) || (m_currentStep == 6))
					SweepStep.fire(m_currentStep);
			}

			void step(int cycles) {
				m_currentCycles -= cycles;
				if (m_currentCycles < 0) {
					step();
					m_currentCycles += m_cyclesPerTick;
				}
			}
		};
	}
}
