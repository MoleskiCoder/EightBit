#include "stdafx.h"
#include "Audio.h"

//

EightBit::GameBoy::Envelope::Envelope() {}

void EightBit::GameBoy::Envelope::reset() {
	m_position = m_volume = m_direction = m_period = 0;
}

bool EightBit::GameBoy::Envelope::zeroed() const {
	return (volume() == 0) && (period() == 0) && (direction() == Attenuate);
}

int EightBit::GameBoy::Envelope::volume() const {
	return m_volume;
}

void EightBit::GameBoy::Envelope::setVolume(int value) {
	m_volume = value;
}

EightBit::GameBoy::Envelope::Direction EightBit::GameBoy::Envelope::direction() const {
	return (Direction)m_direction;
}

void EightBit::GameBoy::Envelope::setDirection(int value) {
	m_direction = value;
}

void EightBit::GameBoy::Envelope::setDirection(Direction value) {
	setDirection((int)value);
}

int EightBit::GameBoy::Envelope::period() const {
	return m_period;
}

void EightBit::GameBoy::Envelope::setPeriod(int value) {
	m_position = m_period = value;
}

void EightBit::GameBoy::Envelope::step() {
	if (m_period != 0) {
		if (--m_position == 0) {
			auto volume = m_volume + (m_direction == Amplify ? +1 : -1);
			if (volume >= 0 || volume <= 15)
				m_volume = volume;
			m_position == m_period;
		}
	}
}

//

EightBit::GameBoy::Sweep::Sweep() {}

void EightBit::GameBoy::Sweep::reset() {
	m_time = m_direction = m_shift = 0;
}

bool EightBit::GameBoy::Sweep::zeroed() const {
	return (time() == 0) && (shift() == 0) && (direction() == Addition);
}

int EightBit::GameBoy::Sweep::time() const {
	return m_time;
}

void EightBit::GameBoy::Sweep::setTime(int value) {
	m_time = value;
}

EightBit::GameBoy::Sweep::Direction EightBit::GameBoy::Sweep::direction() const {
	return (Direction)m_direction;
}

void EightBit::GameBoy::Sweep::setDirection(int value) {
	m_direction = value;
}

void EightBit::GameBoy::Sweep::setDirection(Direction value) {
	setDirection((int)value);
}

int EightBit::GameBoy::Sweep::shift() const {
	return m_shift;
}

void EightBit::GameBoy::Sweep::setShift(int value) {
	m_shift = value;
}

//

EightBit::GameBoy::AudioVoice::AudioVoice() {}

void EightBit::GameBoy::AudioVoice::reset() {
	m_counterContinuous = m_initialise = 0;
}

bool EightBit::GameBoy::AudioVoice::zeroed() const {
	return !initialise() && (type() == Continuous);
}

EightBit::GameBoy::AudioVoice::Type EightBit::GameBoy::AudioVoice::type() const {
	return (Type)m_counterContinuous;
}
void EightBit::GameBoy::AudioVoice::setType(int value) {
	m_counterContinuous = value;
}
void EightBit::GameBoy::AudioVoice::setType(Type value) {
	setType((int)value);
}

bool EightBit::GameBoy::AudioVoice::initialise() const {
	return !!m_initialise;
}
void EightBit::GameBoy::AudioVoice::setInitialise(bool value) {
	m_initialise = value;
}

//

EightBit::GameBoy::WaveVoice::WaveVoice(int cyclesPerSecond)
: m_cyclesPerSecond(cyclesPerSecond) {}

void EightBit::GameBoy::WaveVoice::reset() {
	AudioVoice::reset();
	m_frequencyLowOrder = m_frequencyHighOrder = 0;
}

bool EightBit::GameBoy::WaveVoice::zeroed() const {
	return AudioVoice::zeroed() && (frequency() == 0);
}

int EightBit::GameBoy::WaveVoice::frequencyLowOrder() const {
	return m_frequencyLowOrder;
}

void EightBit::GameBoy::WaveVoice::setFrequencyLowOrder(int value) {
	assert(value < Processor::Bit8);
	m_frequencyLowOrder = value;
}

int EightBit::GameBoy::WaveVoice::frequencyHighOrder() const {
	return m_frequencyHighOrder;
}

void EightBit::GameBoy::WaveVoice::setFrequencyHighOrder(int value) {
	assert(value < Processor::Bit3);
	m_frequencyHighOrder = value;
}

int EightBit::GameBoy::WaveVoice::frequency() const {
	return (m_frequencyHighOrder << 8) | m_frequencyLowOrder;
}

int EightBit::GameBoy::WaveVoice::hertz() const {
	// f = 4194304 / (4 x 8 x (2048 - X)) Hz
	auto division = 4 * 8 * (2048 - frequency());
	return m_cyclesPerSecond / division;
}

void EightBit::GameBoy::WaveVoice::setFrequency(int value) {
	assert(value < Processor::Bit11);
	m_frequencyHighOrder = (value >> 8) & Processor::Mask3;
	m_frequencyLowOrder = value & Processor::Mask8;
}

//

EightBit::GameBoy::RectangularVoice::RectangularVoice(int cyclesPerSecond)
: WaveVoice(cyclesPerSecond) {}

void EightBit::GameBoy::RectangularVoice::reset() {
	WaveVoice::reset();
	m_waveFormDutyCycle = m_soundLength = 0;
}

bool EightBit::GameBoy::RectangularVoice::zeroed() const {
	return WaveVoice::zeroed() && (waveFormDutyCycle() == 0) && (length() == 0);
}

int EightBit::GameBoy::RectangularVoice::waveFormDutyCycle() const {
	return m_waveFormDutyCycle;
}

void EightBit::GameBoy::RectangularVoice::setWaveFormDutyCycle(int value) {
	m_waveFormDutyCycle = value;
}

int EightBit::GameBoy::RectangularVoice::length() const {
	return m_soundLength;
}

void EightBit::GameBoy::RectangularVoice::setLength(int value) {
	m_soundLength = value;
}

//

EightBit::GameBoy::EnvelopedRectangularVoice::EnvelopedRectangularVoice(int cyclesPerSecond)
: RectangularVoice(cyclesPerSecond) {}

void EightBit::GameBoy::EnvelopedRectangularVoice::reset() {
	RectangularVoice::reset();
	m_envelope.reset();
}

bool EightBit::GameBoy::EnvelopedRectangularVoice::zeroed() const {
	return RectangularVoice::zeroed() && m_envelope.zeroed();
}

EightBit::GameBoy::Envelope& EightBit::GameBoy::EnvelopedRectangularVoice::envelope() {
	return m_envelope;
}

//

EightBit::GameBoy::SweptEnvelopedRectangularVoice::SweptEnvelopedRectangularVoice(int cyclesPerSecond)
: EnvelopedRectangularVoice(cyclesPerSecond) {}

void EightBit::GameBoy::SweptEnvelopedRectangularVoice::reset() {
	EnvelopedRectangularVoice::reset();
	m_sweep.reset();
}

bool EightBit::GameBoy::SweptEnvelopedRectangularVoice::zeroed() const {
	return EnvelopedRectangularVoice::zeroed() && m_sweep.zeroed();
}

EightBit::GameBoy::Sweep& EightBit::GameBoy::SweptEnvelopedRectangularVoice::sweep() {
	return m_sweep;
}

//

EightBit::GameBoy::UserDefinedWaveVoice::UserDefinedWaveVoice(int cyclesPerSecond)
: WaveVoice(cyclesPerSecond) {}

void EightBit::GameBoy::UserDefinedWaveVoice::reset() {
	WaveVoice::reset();
	m_enabled = m_soundLength = m_outputLevel = 0;
	for (auto& datum : m_waveData)
		datum = 0;
}

bool EightBit::GameBoy::UserDefinedWaveVoice::zeroed() const {
	bool dataZeroed = true;
	for (const auto& datum : m_waveData) {
		if (datum != 0) {
			dataZeroed = false;
			break;
		}
	}
	return WaveVoice::zeroed() && dataZeroed && !enabled() && (length() == 0) && (level() == 0);
}

bool EightBit::GameBoy::UserDefinedWaveVoice::enabled() const {
	return !!m_enabled;
}

void EightBit::GameBoy::UserDefinedWaveVoice::setEnabled(bool value) {
	m_enabled = value;
}

int EightBit::GameBoy::UserDefinedWaveVoice::length() const {
	return m_soundLength;
}

void EightBit::GameBoy::UserDefinedWaveVoice::setLength(int value) {
	m_soundLength = value;
}

int EightBit::GameBoy::UserDefinedWaveVoice::level() const {
	return m_outputLevel;
}

void EightBit::GameBoy::UserDefinedWaveVoice::setLevel(int value) {
	m_outputLevel = value;
}

int EightBit::GameBoy::UserDefinedWaveVoice::packedWaveDatum(int i) const {
	assert(i < 16);
	return m_waveData[i];
}

void EightBit::GameBoy::UserDefinedWaveVoice::setPackedWaveDatum(int i, uint8_t value) {
	assert(i < 16);
	m_waveData[i] = value;
}

int EightBit::GameBoy::UserDefinedWaveVoice::waveDatum(int i) const {
	assert(i < 32);
	const auto packed = packedWaveDatum(i >> 1);
	return i & 1 ? Processor::lowNibble(packed) : Processor::highNibble(packed);
}

//
	
EightBit::GameBoy::WhiteNoiseWaveVoice::WhiteNoiseWaveVoice() {}

void EightBit::GameBoy::WhiteNoiseWaveVoice::reset() {
	AudioVoice::reset();
	m_envelope.reset();
	m_soundLength = m_polynomialShiftClockFrequency = m_polynomialCounterSteps = m_frequencyDivisionRatio = 0;
}

bool EightBit::GameBoy::WhiteNoiseWaveVoice::zeroed() const {
	return
		AudioVoice::zeroed()
		&& m_envelope.zeroed()
		&& (length() == 0)
		&& (polynomialShiftClockFrequency() == 0)
		&& (polynomialCounterSteps() == 0)
		&& (frequencyDivisionRatio() == 0);
}

EightBit::GameBoy::Envelope& EightBit::GameBoy::WhiteNoiseWaveVoice::envelope() {
	return m_envelope;
}

int EightBit::GameBoy::WhiteNoiseWaveVoice::length() const {
	return m_soundLength;
}

void EightBit::GameBoy::WhiteNoiseWaveVoice::setLength(int value) {
	m_soundLength = value;
}

int EightBit::GameBoy::WhiteNoiseWaveVoice::polynomialShiftClockFrequency() const {
	return m_polynomialShiftClockFrequency;
}

void EightBit::GameBoy::WhiteNoiseWaveVoice::setPolynomialShiftClockFrequency(int value) {
	m_polynomialShiftClockFrequency = value;
}

int EightBit::GameBoy::WhiteNoiseWaveVoice::polynomialCounterSteps() const {
	return m_polynomialCounterSteps;
}

void EightBit::GameBoy::WhiteNoiseWaveVoice::setPolynomialCounterSteps(int value) {
	m_polynomialCounterSteps = value;
}

int EightBit::GameBoy::WhiteNoiseWaveVoice::frequencyDivisionRatio() const {
	return m_frequencyDivisionRatio;
}

void EightBit::GameBoy::WhiteNoiseWaveVoice::setFrequencyDivisionRatio(int value) {
	m_frequencyDivisionRatio = value;
}

//

EightBit::GameBoy::OutputChannel::OutputChannel() {}

void EightBit::GameBoy::OutputChannel::reset() {
	m_vin = false;
	m_outputLevel = 0;
	for (auto& outputVoice : m_outputVoices)
		outputVoice = false;
}

bool EightBit::GameBoy::OutputChannel::zeroed() const {
	return
		!vin()
		&& outputLevel() == 0
		&& !m_outputVoices[0] && !m_outputVoices[1] && !m_outputVoices[2] && !m_outputVoices[3];
}

bool EightBit::GameBoy::OutputChannel::vin() const {
	return m_vin;
}
void EightBit::GameBoy::OutputChannel::setVin(bool value) {
	m_vin = value;
}

int EightBit::GameBoy::OutputChannel::outputLevel() const {
	return m_outputLevel;
}
void EightBit::GameBoy::OutputChannel::setOutputLevel(int value) {
	m_outputLevel = value;
}

bool& EightBit::GameBoy::OutputChannel::outputVoice(int voice) {
	return m_outputVoices[voice];
}
bool& EightBit::GameBoy::OutputChannel::outputVoice1() {
	return m_outputVoices[0];
}
bool& EightBit::GameBoy::OutputChannel::outputVoice2() {
	return m_outputVoices[1];
}
bool& EightBit::GameBoy::OutputChannel::outputVoice3() {
	return m_outputVoices[2];
}
bool& EightBit::GameBoy::OutputChannel::outputVoice4() {
	return m_outputVoices[3];
}

EightBit::GameBoy::Audio::Audio(int cyclesPerSecond)
: m_frameSequencer(cyclesPerSecond),
  m_enabled(false) {

	m_voices[0] = std::make_shared<SweptEnvelopedRectangularVoice>(cyclesPerSecond);
	m_voices[1] = std::make_shared<EnvelopedRectangularVoice>(cyclesPerSecond);
	m_voices[2] = std::make_shared<UserDefinedWaveVoice>(cyclesPerSecond);
	m_voices[3] = std::make_shared<WhiteNoiseWaveVoice>();

	m_frameSequencer.FrameStep.connect(std::bind(&Audio::Sequencer_FrameStep, this, std::placeholders::_1));
	m_frameSequencer.LengthStep.connect(std::bind(&Audio::Sequencer_LengthStep, this, std::placeholders::_1));
	m_frameSequencer.VolumeStep.connect(std::bind(&Audio::Sequencer_VolumeStep, this, std::placeholders::_1));
	m_frameSequencer.SweepStep.connect(std::bind(&Audio::Sequencer_SweepStep, this, std::placeholders::_1));
}

std::shared_ptr<EightBit::GameBoy::AudioVoice> EightBit::GameBoy::Audio::voice(int i) {
	return m_voices[i];
}

EightBit::GameBoy::SweptEnvelopedRectangularVoice* EightBit::GameBoy::Audio::voice1() {
	return (SweptEnvelopedRectangularVoice*)voice(0).get();
}

EightBit::GameBoy::EnvelopedRectangularVoice* EightBit::GameBoy::Audio::voice2() {
	return (EnvelopedRectangularVoice*)voice(1).get();
}

EightBit::GameBoy::UserDefinedWaveVoice* EightBit::GameBoy::Audio::voice3() {
	return (UserDefinedWaveVoice*)voice(2).get();
}

EightBit::GameBoy::WhiteNoiseWaveVoice* EightBit::GameBoy::Audio::voice4() {
	return (WhiteNoiseWaveVoice*)voice(3).get();
}

EightBit::GameBoy::OutputChannel& EightBit::GameBoy::Audio::channel(int i) {
	return m_channels[i];
}
EightBit::GameBoy::OutputChannel& EightBit::GameBoy::Audio::channel1() {
	return channel(0);
}
EightBit::GameBoy::OutputChannel& EightBit::GameBoy::Audio::channel2() {
	return channel(1);
}

bool EightBit::GameBoy::Audio::enabled() const {
	return m_enabled;
}

void EightBit::GameBoy::Audio::setEnabled(bool value) {
	m_enabled = value;
	if (!enabled())
		reset();
}

void EightBit::GameBoy::Audio::reset() {
	m_enabled = false;
	for (auto voice : m_voices)
		voice->reset();
	for (auto& channel : m_channels)
		channel.reset();
}

bool EightBit::GameBoy::Audio::zeroed() const {
	auto channelsZeroed = m_channels[0].zeroed() && m_channels[1].zeroed();
	auto voice1Zero = m_voices[0]->zeroed();
	auto voice2Zero = m_voices[1]->zeroed();
	auto voice3Zero = m_voices[2]->zeroed();
	auto voice4Zero = m_voices[3]->zeroed();
	auto voicesZeroed = voice1Zero && voice2Zero && voice3Zero && voice4Zero;
	return !enabled() && channelsZeroed && voicesZeroed;
}

//

bool EightBit::GameBoy::Audio::voice1On() const {
	return true;
}
bool EightBit::GameBoy::Audio::voice2On() const {
	return true;
}
bool EightBit::GameBoy::Audio::voice3On() const {
	return true;
}
bool EightBit::GameBoy::Audio::voice4On() const {
	return true;
}

//

uint8_t EightBit::GameBoy::Audio::toNRx1(int i) {
	auto voice = (RectangularVoice*)m_voices[i].get();
	return (voice->waveFormDutyCycle() << 6) | Processor::Mask6;
}

void EightBit::GameBoy::Audio::fromNRx1(int i, uint8_t value) {
	auto voice = (RectangularVoice*)m_voices[i].get();
	voice->setWaveFormDutyCycle((value >> 6) & Processor::Mask2);	// Bits 6-7
	voice->setLength(value & Processor::Mask6);						// Bits 0-5
}

uint8_t EightBit::GameBoy::Audio::toNRx2(int i) {
	auto voice = (EnvelopedRectangularVoice*)m_voices[i].get();
	auto& envelope = voice->envelope();
	return (envelope.volume() << 4) | (envelope.direction() << 3) | envelope.period();
}

void EightBit::GameBoy::Audio::fromNRx2(int i, uint8_t value) {
	auto voice = (EnvelopedRectangularVoice*)m_voices[i].get();
	auto& envelope = voice->envelope();
	envelope.setVolume((value >> 4) & Processor::Mask4);	// Bits 4-7
	envelope.setDirection((value >> 3) & Processor::Mask1);	// Bit 3
	envelope.setPeriod(value & Processor::Mask3); 			// Bits 0-2
}

uint8_t EightBit::GameBoy::Audio::toNRx3(int i) {
	return Processor::Mask8;
}

void EightBit::GameBoy::Audio::fromNRx3(int i, uint8_t value) {
	auto voice = (WaveVoice*)m_voices[i].get();
	voice->setFrequencyLowOrder(value);
}

// Sound mode 1 register: Sweep

uint8_t EightBit::GameBoy::Audio::toNR10() {
	auto& sweep = voice1()->sweep();
	return
		Processor::Bit7
		| (sweep.time() << 4)
		| (sweep.direction() << 3)
		| sweep.shift();
}

void EightBit::GameBoy::Audio::fromNR10(uint8_t value) {
	auto& sweep = voice1()->sweep();
	sweep.setTime((value >> 4) & Processor::Mask3);			// Bits 4-6
	sweep.setDirection((value >> 3) & Processor::Mask1);	// Bit 3
	sweep.setShift(value & Processor::Mask3);				// Bits 0-2
}

// Sound mode 1 register: Sound length / Wave pattern duty
uint8_t EightBit::GameBoy::Audio::toNR11() { 
	return toNRx1(0);
}
void EightBit::GameBoy::Audio::fromNR11(uint8_t value) {
	fromNRx1(0, value);
}

// Sound mode 1 register: Envelope
uint8_t EightBit::GameBoy::Audio::toNR12() {
	return toNRx2(0);
}
void EightBit::GameBoy::Audio::fromNR12(uint8_t value) {
	fromNRx2(0, value);
}

// Sound mode 1 register: Frequency lo
uint8_t EightBit::GameBoy::Audio::toNR13() {
	return toNRx3(0);
}
void EightBit::GameBoy::Audio::fromNR13(uint8_t value) {
	fromNRx3(0, value);
}

// Sound mode 1 register: Frequency hi

uint8_t EightBit::GameBoy::Audio::toNR14() {
	return Processor::Bit7 | (voice1()->type() << 6) | Processor::Mask6;
}

void EightBit::GameBoy::Audio::fromNR14(uint8_t value) {
	voice1()->setInitialise((value >> 7) & Processor::Mask1);	// Bits 7
	voice1()->setType((value >> 6) & Processor::Mask1);			// Bits 6
	voice1()->setFrequencyHighOrder(value & Processor::Mask3);	// Bits 0-2
}

// Sound mode 2 register: Sound length / Wave pattern duty
uint8_t EightBit::GameBoy::Audio::toNR21() {
	return toNRx1(1);
}
void EightBit::GameBoy::Audio::fromNR21(uint8_t value) {
	fromNRx1(1, value);
}

// Sound mode 2 register: Envelope
uint8_t EightBit::GameBoy::Audio::toNR22() {
	return toNRx2(1);
}
void EightBit::GameBoy::Audio::fromNR22(uint8_t value) {
	fromNRx2(1, value);
}

// Sound mode 2 register: Frequency lo
uint8_t EightBit::GameBoy::Audio::toNR23() {
	return toNRx3(1);
}
void EightBit::GameBoy::Audio::fromNR23(uint8_t value) {
	fromNRx3(1, value);
}

// Sound mode 2 register: Frequency hi

uint8_t EightBit::GameBoy::Audio::toNR24() {
	return Processor::Bit7 | (voice2()->type() << 6) | Processor::Mask6;
}

void EightBit::GameBoy::Audio::fromNR24(uint8_t value) {
	voice2()->setInitialise((value >> 7) & Processor::Mask1);	// Bits 7
	voice2()->setType((value >> 6) & Processor::Mask1);			// Bits 6
	voice2()->setFrequencyHighOrder(value & Processor::Mask3);	// Bits 0-2
}

// Sound mode 3 register: Sound on/off

uint8_t EightBit::GameBoy::Audio::toNR30() {
	return (voice3()->enabled() << 7) | Processor::Mask7;
}

void EightBit::GameBoy::Audio::fromNR30(uint8_t value) {
	voice3()->setEnabled((value >> 7) & Processor::Mask1);		// Bit 7
}

// Sound mode 3 register: Sound length

uint8_t EightBit::GameBoy::Audio::toNR31() {
	return voice3()->length();
}

void EightBit::GameBoy::Audio::fromNR31(uint8_t value) {
	voice3()->setLength(value);
}

// Sound mode 3 register: Select output level

uint8_t EightBit::GameBoy::Audio::toNR32() {
	return Processor::Bit7 | Processor::Bit6 | voice3()->level() << 5 | Processor::Mask5;
}

void EightBit::GameBoy::Audio::fromNR32(uint8_t value) {
	voice3()->setLevel((value >> 5) & Processor::Mask2);	// Bits 6-5
}

// Sound mode 3 register: Frequency lo
uint8_t EightBit::GameBoy::Audio::toNR33() {
	return toNRx3(2);
}
void EightBit::GameBoy::Audio::fromNR33(uint8_t value) {
	fromNRx3(2, value);
}

// Sound mode 3 register: Frequency hi

uint8_t EightBit::GameBoy::Audio::toNR34() {
	return Processor::Bit7 | (voice3()->type() << 6) | Processor::Mask6;
}

void EightBit::GameBoy::Audio::fromNR34(uint8_t value) {
	voice3()->setInitialise((value >> 7) & Processor::Mask1);	// Bits 7
	voice3()->setType((value >> 6) & Processor::Mask1);			// Bits 6
	voice3()->setFrequencyHighOrder(value & Processor::Mask3);	// Bits 0-2
}

// Sound mode 4 register: Sound length

uint8_t EightBit::GameBoy::Audio::toNR41() {
	return Processor::Bit7 | Processor::Bit6 | voice4()->length();
}

void EightBit::GameBoy::Audio::fromNR41(uint8_t value) {
	voice4()->setLength(value);
}

// Sound mode 4 register: Envelope
uint8_t EightBit::GameBoy::Audio::toNR42() {
	return toNRx2(3);
}
void EightBit::GameBoy::Audio::fromNR42(uint8_t value) {
	fromNRx2(3, value);
}

// Sound mode 4 register: Polynomial counter

uint8_t EightBit::GameBoy::Audio::toNR43() {
	return
		(voice4()->polynomialShiftClockFrequency() << 4)
		| voice4()->polynomialCounterSteps() << 3
		| voice4()->frequencyDivisionRatio();
}

void EightBit::GameBoy::Audio::fromNR43(uint8_t value) {
	voice4()->setPolynomialShiftClockFrequency((value >> 4) & Processor::Mask4);	// Bits 4-7
	voice4()->setPolynomialCounterSteps((value >> 3) & Processor::Mask1);			// Bit 3
	voice4()->setFrequencyDivisionRatio(value & Processor::Mask3);					// Bits 0-2
}

// Sound mode 4 register: counter/consecutive; inital

uint8_t EightBit::GameBoy::Audio::toNR44() {
	return Processor::Bit7 | (voice4()->type() << 6) | Processor::Mask6;
}

void EightBit::GameBoy::Audio::fromNR44(uint8_t value) {
	voice4()->setInitialise((value >> 7) & Processor::Mask1);	// Bit 7
	voice4()->setType((value >> 6) & Processor::Mask1);			// Bit 6
}

// Channel control: on-off/volume

uint8_t EightBit::GameBoy::Audio::toNR50() {
	return
		(channel2().vin() << 7)
		| (channel2().outputLevel() << 4)
		| (channel2().vin() << 3)
		| channel2().outputLevel();
}

void EightBit::GameBoy::Audio::fromNR50(uint8_t value) {
	channel2().setVin((value >> 7) & Processor::Mask1);			// Bit 7
	channel2().setOutputLevel((value >> 4) & Processor::Mask3);	// Bits 4-6
	channel1().setVin((value >> 3) & Processor::Mask1);			// Bit 3
	channel1().setOutputLevel(value & Processor::Mask3);		// Bits 0-2
}

// Selection of Sound output terminal

uint8_t EightBit::GameBoy::Audio::toNR51() {
	return
		(channel2().outputVoice4() << 7)
		| (channel2().outputVoice3() << 6)
		| (channel2().outputVoice2() << 5)
		| (channel2().outputVoice1() << 4)
		| (channel1().outputVoice4() << 3)
		| (channel1().outputVoice3() << 2)
		| (channel1().outputVoice2() << 1)
		| (int)channel1().outputVoice1();
}

void EightBit::GameBoy::Audio::fromNR51(uint8_t value) {
	channel2().outputVoice4() = (value >> 7) & Processor::Mask1;	// Bit 7
	channel2().outputVoice3() = (value >> 6) & Processor::Mask1;	// Bit 6
	channel2().outputVoice2() = (value >> 5) & Processor::Mask1;	// Bit 5
	channel2().outputVoice1() = (value >> 4) & Processor::Mask1;	// Bit 4
	channel1().outputVoice4() = (value >> 3) & Processor::Mask1;	// Bit 3
	channel1().outputVoice3() = (value >> 2) & Processor::Mask1;	// Bit 2
	channel1().outputVoice2() = (value >> 1) & Processor::Mask1;	// Bit 1
	channel1().outputVoice1() = value & Processor::Mask1;			// Bit 0
}

// Sound on/off

uint8_t EightBit::GameBoy::Audio::toNR52() {
	return
		(enabled() << 7)
		| Processor::Bit6 | Processor::Bit5 | Processor::Bit4
		| (voice4On() << 3)
		| (voice3On() << 2)
		| (voice2On() << 1)
		| (int)voice1On();
}

void EightBit::GameBoy::Audio::fromNR52(uint8_t value) {
	setEnabled((value >> 7) & Processor::Mask1);	// Bit 7
}

void EightBit::GameBoy::Audio::setPackedWaveDatum(int i, uint8_t value) {
	voice3()->setPackedWaveDatum(i, value);
}

uint8_t EightBit::GameBoy::Audio::packedWaveDatum(int i) {
	return voice3()->packedWaveDatum(i);
}

void EightBit::GameBoy::Audio::stepFrame(int cycles) {
	m_frameSequencer.step(cycles);
}

void EightBit::GameBoy::Audio::Sequencer_FrameStep(const int step) {
}

void EightBit::GameBoy::Audio::Sequencer_LengthStep(const int step) {
}

void EightBit::GameBoy::Audio::Sequencer_VolumeStep(const int step) {
	voice1()->envelope().step();
	voice2()->envelope().step();
	voice4()->envelope().step();
}

void EightBit::GameBoy::Audio::Sequencer_SweepStep(const int step) {
}
