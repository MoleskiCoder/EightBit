#pragma once

#include <array>
#include <memory>
#include <iostream>
#include <cassert>

#include <Processor.h>

namespace EightBit {
	namespace GameBoy {

		class Envelope {
		public:
			Envelope() {}

			enum Direction { Attenuate, Amplify };

			int default() const { return m_defaultValue; }
			void setDefault(int value) { m_defaultValue = value; }

			Direction direction() const { return (Direction)m_direction; }
			void setDirection(int value) { m_direction = value; }
			void setDirection(Direction value) { setDirection((int)value); }

			int stepLength() const { return m_stepLength; }
			void setStepLength(int value) { m_stepLength = value; }

			void dump() const {
				std::cout << "Envelope: default=" << default() << ",direction=" << direction() << ",step length=" << stepLength() << std::endl;
			}

		private:
			int m_defaultValue;
			int m_direction;
			int m_stepLength;
		};

		class Sweep {
		public:
			Sweep() {}

			enum Direction { Addition, Subtraction };

			int time() const { return m_time; }
			void setTime(int value) { m_time = value; }

			Direction direction() const { return (Direction)m_direction; }
			void setDirection(int value) { m_direction = value; }
			void setDirection(Direction value) { setDirection((int)value); }

			int shift() const { return m_shift; }
			void setShift(int value) { m_shift = value; }

			void dump() const {
				std::cout << "Sweep: time=" << time() << ",direction=" << direction() << ",shift=" << shift() << std::endl;
			}

		private:
			int m_time;
			int m_direction;
			int m_shift;
		};

		class AudioVoice {
		public:
			AudioVoice() {}

			enum Type { Continuous, Counter };

			Type type() const { return (Type)m_counterContinuous; }
			void setType(int value) { m_counterContinuous = value; }
			void setType(Type value) { setType((int)value); }

			bool initialise() const { return !!m_initialise; }
			void setInitialise(bool value) { m_initialise = value; }

			virtual void dump() const {
				std::cout << "Audio Voice: type=" << type() << ",initialise=" << initialise() << std::endl;
			}

		private:
			int m_counterContinuous;
			int m_initialise;
		};

		class WaveVoice : public AudioVoice {
		public:
			WaveVoice() {}

			int frequencyLowOrder() const { return m_frequencyLowOrder; }

			void setFrequencyLowOrder(int value) {
				assert(value < Processor::Bit8);
				m_frequencyLowOrder = value;
			}

			int frequencyHighOrder() const { return m_frequencyHighOrder; }

			void setFrequencyHighOrder(int value) {
				assert(value < Processor::Bit3);
				m_frequencyHighOrder = value;
			}

			int frequency() const {
				return (m_frequencyHighOrder >> 8) | m_frequencyLowOrder;
			}

			void setFrequency(int value) {
				assert(value < Processor::Bit11);
				m_frequencyHighOrder = (value >> 8) & Processor::Mask3;
				m_frequencyLowOrder = value & Processor::Mask8;
			}

			virtual void dump() const override {
				AudioVoice::dump();
				std::cout << "Wave Voice: frequency=" << frequency() << std::endl;
			}

		private:
			int m_frequencyLowOrder;	// 8 bits
			int m_frequencyHighOrder;	// 3 bits
		};

		class RectangularVoice : public WaveVoice {
		public:
			RectangularVoice() {}

			int waveFormDutyCycle() const { return m_waveFormDutyCycle; }
			void setWaveFormDutyCycle(int value) { m_waveFormDutyCycle = value; }

			int length() const { return m_soundLength; }
			void setLength(int value) { m_soundLength = value; }

			virtual void dump() const override {
				WaveVoice::dump();
				std::cout << "Rectangular Voice: wave form duty=" << waveFormDutyCycle() << ",length=" << length() << std::endl;
			}

		private:
			int m_waveFormDutyCycle;
			int m_soundLength;
		};

		// NR2X
		class EnvelopedRectangularVoice : public RectangularVoice {
		public:
			EnvelopedRectangularVoice() {}

			Envelope& envelope() { return m_envelope; }

			virtual void dump() const override {
				RectangularVoice::dump();
				m_envelope.dump();
			}

		private:
			Envelope m_envelope;
		};

		// NR1X
		class SweptEnvelopedRectangularVoice : public EnvelopedRectangularVoice {
		public:
			SweptEnvelopedRectangularVoice() {}

			Sweep& sweep() { return m_sweep; }

			virtual void dump() const override {
				EnvelopedRectangularVoice::dump();
				m_sweep.dump();
			}

		private:
			Sweep m_sweep;
		};

		// NR3X
		class UserDefinedWaveVoice : public WaveVoice {
		public:
			UserDefinedWaveVoice() {}

			bool enabled() const { return !!m_enabled; }
			void setEnabled(bool value) { m_enabled = value; }
				
			int length() const { return m_soundLength; }
			void setLength(int value) { m_soundLength = value; }

			int level() const { return m_outputLevel; }
			void setLevel(int value) { m_outputLevel = value; }

			int packedWaveDatum(int i) const {
				assert(i < 16);
				return m_waveData[i];
			}

			void setPackedWaveDatum(int i, uint8_t value) {
				assert(i < 16);
				m_waveData[i] = value;
			}

			int waveDatum(int i) const {
				assert(i < 32);
				const auto packed = packedWaveDatum(i >> 1);
				return i & 1 ? Processor::lowNibble(packed) : Processor::highNibble(packed);
			}

		private:
			int m_enabled;
			int m_soundLength;
			int m_outputLevel;
			std::array<uint8_t, 16> m_waveData;
		};

		// NR4X
		class WhiteNoiseWaveVoice : public AudioVoice {
		public:
			WhiteNoiseWaveVoice() {}

			Envelope& envelope() { return m_envelope;  }

			int length() const { return m_soundLength; }
			void setLength(int value) { m_soundLength = value; }

			int polynomialShiftClockFrequency() const { return m_polynomialShiftClockFrequency; }
			void setPolynomialShiftClockFrequency(int value) { m_polynomialShiftClockFrequency = value; }

			int polynomialCounterSteps() const { return m_polynomialCounterSteps; }
			void setPolynomialCounterSteps(int value) { m_polynomialCounterSteps = value; }

			int frequencyDivisionRatio() const { return m_frequencyDivisionRatio; }
			void setFrequencyDivisionRatio(int value) { m_frequencyDivisionRatio = value; }

		private:
			Envelope m_envelope;
			int m_soundLength;
			int m_polynomialShiftClockFrequency;
			int m_polynomialCounterSteps;
			int m_frequencyDivisionRatio;
		};

		class OutputChannel {
		public:
			OutputChannel() {}

			bool vin() const { return m_vin; }
			void setVin(bool value) { m_vin = value; }

			int outputLevel() const { return m_outputLevel; }
			void setOutputLevel(int value) { m_outputLevel = value; }

			bool& outputVoice(int voice) { return m_outputVoice[voice]; }
			bool& outputVoice1() { return m_outputVoice[0]; }
			bool& outputVoice2() { return m_outputVoice[1]; }
			bool& outputVoice3() { return m_outputVoice[2]; }
			bool& outputVoice4() { return m_outputVoice[3]; }

		private:
			bool m_vin;
			int m_outputLevel;
			std::array<bool, 4> m_outputVoice;
		};

		class Audio {
		public:
			Audio() {
				m_voices[0] = std::make_shared<SweptEnvelopedRectangularVoice>();
				m_voices[1] = std::make_shared<EnvelopedRectangularVoice>();
				m_voices[2] = std::make_shared<UserDefinedWaveVoice>();
				m_voices[3] = std::make_shared<WhiteNoiseWaveVoice>();
			}

			std::shared_ptr<AudioVoice> voice(int i) { return m_voices[i]; }

			SweptEnvelopedRectangularVoice* voice1() {
				return (SweptEnvelopedRectangularVoice*)voice(0).get();
			}

			EnvelopedRectangularVoice* voice2() {
				return (EnvelopedRectangularVoice*)voice(1).get();
			}

			UserDefinedWaveVoice* voice3() {
				return (UserDefinedWaveVoice*)voice(2).get();
			}

			WhiteNoiseWaveVoice* voice4() {
				return (WhiteNoiseWaveVoice*)voice(3).get();
			}

			OutputChannel& channel(int i) { return m_channels[i]; }
			OutputChannel& channel1() { return channel(0); }
			OutputChannel& channel2() { return channel(1); }

			bool enabled() const { return m_enabled; }
			void setEnabled(bool value) { m_enabled = value; }

			void dump() const {
				for (int i = 0; i < 4; ++i) {
					std::cout << "** Voice " << i + 1 << std::endl;
					m_voices[i]->dump();
				}
			}

		private:
			std::array<std::shared_ptr<AudioVoice>, 4> m_voices;
			std::array<OutputChannel, 2> m_channels;
			bool m_enabled;
		};
	}
}