#pragma once

#include <array>
#include <memory>
#include <iostream>
#include <cassert>

#include "AudioFrame.h"

namespace EightBit {
	namespace GameBoy {

		class Envelope final {
		public:
			Envelope();

			enum Direction { Attenuate, Amplify };

			void reset();
			bool zeroed() const;

			int default() const;
			void setDefault(int value);

			Direction direction() const;
			void setDirection(int value);
			void setDirection(Direction value);

			int stepLength() const;
			void setStepLength(int value);

		private:
			int m_defaultValue;
			int m_direction;
			int m_stepLength;
		};

		class Sweep final {
		public:
			Sweep();

			enum Direction { Addition, Subtraction };

			void reset();
			bool zeroed() const;

			int time() const;
			void setTime(int value);

			Direction direction() const;
			void setDirection(int value);
			void setDirection(Direction value);

			int shift() const;
			void setShift(int value);

		private:
			int m_time;
			int m_direction;
			int m_shift;
		};

		class AudioVoice {
		public:
			AudioVoice();

			virtual void reset();
			virtual bool zeroed() const;

			enum Type { Continuous, Counter };

			Type type() const;
			void setType(int value);
			void setType(Type value);

			bool initialise() const;
			void setInitialise(bool value);

		private:
			int m_counterContinuous;
			int m_initialise;
		};

		class WaveVoice : public AudioVoice {
		public:
			WaveVoice(int cyclesPerSecond);

			virtual void reset() override;
			virtual bool zeroed() const override;

			int frequencyLowOrder() const;
			void setFrequencyLowOrder(int value);
			int frequencyHighOrder() const;
			void setFrequencyHighOrder(int value);

			int frequency() const;
			void setFrequency(int value);

			int hertz() const;

		private:
			const int m_cyclesPerSecond;
			int m_frequencyLowOrder;	// 8 bits
			int m_frequencyHighOrder;	// 3 bits
		};

		class RectangularVoice : public WaveVoice {
		public:
			RectangularVoice(int cyclesPerSecond);

			virtual void reset() override;
			virtual bool zeroed() const override;

			int waveFormDutyCycle() const;
			void setWaveFormDutyCycle(int value);

			int length() const;
			void setLength(int value);

		private:
			int m_waveFormDutyCycle;
			int m_soundLength;
		};

		// NR2X
		class EnvelopedRectangularVoice : public RectangularVoice {
		public:
			EnvelopedRectangularVoice(int cyclesPerSecond);

			virtual void reset() override;
			virtual bool zeroed() const override;

			Envelope& envelope();

		private:
			Envelope m_envelope;
		};

		// NR1X
		class SweptEnvelopedRectangularVoice : public EnvelopedRectangularVoice {
		public:
			SweptEnvelopedRectangularVoice(int cyclesPerSecond);

			virtual void reset() override;
			virtual bool zeroed() const override;

			Sweep& sweep();

		private:
			Sweep m_sweep;
		};

		// NR3X
		class UserDefinedWaveVoice : public WaveVoice {
		public:
			UserDefinedWaveVoice(int cyclesPerSecond);

			virtual void reset() override;
			virtual bool zeroed() const override;

			bool enabled() const;
			void setEnabled(bool value);
				
			int length() const;
			void setLength(int value);

			int level() const;
			void setLevel(int value);

			int packedWaveDatum(int i) const;
			void setPackedWaveDatum(int i, uint8_t value);
			int waveDatum(int i) const;

		private:
			int m_enabled;
			int m_soundLength;
			int m_outputLevel;
			std::array<uint8_t, 16> m_waveData;
		};

		// NR4X
		class WhiteNoiseWaveVoice : public AudioVoice {
		public:
			WhiteNoiseWaveVoice();

			virtual void reset() override;
			virtual bool zeroed() const override;

			Envelope& envelope();

			int length() const;
			void setLength(int value);

			int polynomialShiftClockFrequency() const;
			void setPolynomialShiftClockFrequency(int value);

			int polynomialCounterSteps() const;
			void setPolynomialCounterSteps(int value);

			int frequencyDivisionRatio() const;
			void setFrequencyDivisionRatio(int value);

		private:
			Envelope m_envelope;
			int m_soundLength;
			int m_polynomialShiftClockFrequency;
			int m_polynomialCounterSteps;
			int m_frequencyDivisionRatio;
		};

		class OutputChannel final {
		public:
			OutputChannel();

			void reset();
			bool zeroed() const;

			bool vin() const;
			void setVin(bool value);

			int outputLevel() const;
			void setOutputLevel(int value);

			bool& outputVoice(int voice);
			bool& outputVoice1();
			bool& outputVoice2();
			bool& outputVoice3();
			bool& outputVoice4();

		private:
			bool m_vin;
			int m_outputLevel;
			std::array<bool, 4> m_outputVoices;
		};

		class Audio final {
		public:
			Audio(int cyclesPerSecond);

			std::shared_ptr<AudioVoice> voice(int i);
			SweptEnvelopedRectangularVoice* voice1();
			EnvelopedRectangularVoice* voice2();
			UserDefinedWaveVoice* voice3();
			WhiteNoiseWaveVoice* voice4();

			OutputChannel& channel(int i);
			OutputChannel& channel1();
			OutputChannel& channel2();

			bool enabled() const;
			void setEnabled(bool value);

			void reset();
			bool zeroed() const;

			//

			bool voice1On() const;
			bool voice2On() const;
			bool voice3On() const;
			bool voice4On() const;

			//

			uint8_t toNRx1(int i);
			void fromNRx1(int i, uint8_t value);

			uint8_t toNRx2(int i);
			void fromNRx2(int i, uint8_t value);

			uint8_t toNRx3(int i);
			void fromNRx3(int i, uint8_t value);

			// Sound mode 1 register: Sweep
			uint8_t toNR10();
			void fromNR10(uint8_t value);

			// Sound mode 1 register: Sound length / Wave pattern duty
			uint8_t toNR11();
			void fromNR11(uint8_t value);

			// Sound mode 1 register: Envelope
			uint8_t toNR12();
			void fromNR12(uint8_t value);

			// Sound mode 1 register: Frequency lo
			uint8_t toNR13();
			void fromNR13(uint8_t value);

			// Sound mode 1 register: Frequency hi
			uint8_t toNR14();
			void fromNR14(uint8_t value);

			// Sound mode 2 register: Sound length / Wave pattern duty
			uint8_t toNR21();
			void fromNR21(uint8_t value);

			// Sound mode 2 register: Envelope
			uint8_t toNR22();
			void fromNR22(uint8_t value);

			// Sound mode 2 register: Frequency lo
			uint8_t toNR23();
			void fromNR23(uint8_t value);

			// Sound mode 2 register: Frequency hi
			uint8_t toNR24();
			void fromNR24(uint8_t value);

			// Sound mode 3 register: Sound on/off
			uint8_t toNR30();
			void fromNR30(uint8_t value);

			// Sound mode 3 register: Sound length
			uint8_t toNR31();
			void fromNR31(uint8_t value);

			// Sound mode 3 register: Select output level
			uint8_t toNR32();
			void fromNR32(uint8_t value);

			// Sound mode 3 register: Frequency lo
			uint8_t toNR33();
			void fromNR33(uint8_t value);

			// Sound mode 3 register: Frequency hi
			uint8_t toNR34();
			void fromNR34(uint8_t value);

			// Sound mode 4 register: Sound length
			uint8_t toNR41();
			void fromNR41(uint8_t value);

			// Sound mode 4 register: Envelope
			uint8_t toNR42();
			void fromNR42(uint8_t value);

			// Sound mode 4 register: Polynomial counter
			uint8_t toNR43();
			void fromNR43(uint8_t value);

			// Sound mode 4 register: counter/consecutive; inital
			uint8_t toNR44();
			void fromNR44(uint8_t value);

			// Channel control: on-off/volume
			uint8_t toNR50();
			void fromNR50(uint8_t value);

			// Selection of Sound output terminal
			uint8_t toNR51();
			void fromNR51(uint8_t value);

			// Sound on/off
			uint8_t toNR52();
			void fromNR52(uint8_t value);

			void setPackedWaveDatum(int i, uint8_t value);
			uint8_t packedWaveDatum(int i);

			void stepFrame(int cycles);

			void Sequencer_FrameStep(int step);
			void Sequencer_LengthStep(int step);
			void Sequencer_VolumeStep(int step);
			void Sequencer_SweepStep(int step);

		private:
			AudioFrame m_frameSequencer;

			std::array<std::shared_ptr<AudioVoice>, 4> m_voices;
			std::array<OutputChannel, 2> m_channels;
			bool m_enabled;
		};
	}
}