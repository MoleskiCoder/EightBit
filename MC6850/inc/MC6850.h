#pragma once

#include <cstdint>

#include <ClockedChip.h>
#include <Signal.h>

namespace EightBit {
	class mc6850 final : public ClockedChip {
	public:
		// Receive data, (I) Active high
		DECLARE_PIN_INPUT(RXDATA)

		// Transmit data, (O) Active high
		DECLARE_PIN_OUTPUT(TXDATA)

		// Request to send, (O) Active low
		DECLARE_PIN_OUTPUT(RTS)

		// Clear to send, (I) Active low
		DECLARE_PIN_INPUT(CTS)

		// Data carrier detect, (I) Active low
		DECLARE_PIN_INPUT(DCD)

		// Transmit clock, (I) Active high
		DECLARE_PIN_INPUT(RXCLK)

		// Receive clock, (I) Active high
		DECLARE_PIN_INPUT(TXCLK)

		// Chip select, bit 0, (I) Active high
		DECLARE_PIN_INPUT(CS0)

		// Chip select, bit 1, (I) Active high
		DECLARE_PIN_INPUT(CS1)

		// Chip select, bit 2, (I) Active low
		DECLARE_PIN_INPUT(CS2)

		// Register select, (I) Active high
		DECLARE_PIN_INPUT(RS)

		// Read/Write, (I) Read high, write low
		DECLARE_PIN_INPUT(RW)

		// ACIA Enable, (I) Active high
		DECLARE_PIN_INPUT(E)

		// Interrupt request, (O) Active low
		DECLARE_PIN_OUTPUT(IRQ)

	public:
		mc6850();

        // +--------+----------------------------------------------------------------------------------+
        // |        |                                Buffer address                                    |
        // |        +------------------+------------------+--------------------+-----------------------+
        // |        |            _     |            _     |              _     |               _       |
        // |  Data  |     RS * R/W     |     RS * R/W     |       RS * R/W     |        RS * R/W       |
        // |  Bus   |   (high)(low)    |   (high)(high)   |      (low)(low)    |       (low)(low)      |
        // |  Line  |     Transmit     |      Receive     |                    |                       |
        // | Number |       Data       |        Data      |        Control     |         Status        |
        // |        |     Register     |      Register    |       Register     |        register       |
        // |        +------------------+------------------+--------------------+-----------------------+
        // |        |  (Write only)    |   (Read only)    |       (Write only) |      (Read only)      |
        // +--------+------------------+------------------+--------------------+-----------------------+
        // |    0   |    Data bit 0*   |    Data bit 0    |   Counter divide   | Receive data register |
        // |        |                  |                  |   select 1 (CR0)   |      full (RDRF)      |
        // +--------+------------------+------------------+--------------------+-----------------------+
        // |    1   |    Data bit 1    |    Data bit 1    |   Counter divide   | Transmit data register|
        // |        |                  |                  |   select 2 (CR1)   |     empty (TDRE)      |
        // +--------+------------------+------------------+--------------------+-----------------------+
        // |    2   |    Data bit 2    |    Data bit 2    |   Word select 1    |  Data carrier detect  |
        // |        |                  |                  |          (CR2)     |      (DCD active)     |
        // +--------+------------------+------------------+--------------------+-----------------------+
        // |    3   |    Data bit 3    |    Data bit 3    |   Word select 1    |     Clear to send     |
        // |        |                  |                  |          (CR3)     |      (CTS active)     |
        // +--------+------------------+------------------+--------------------+-----------------------+
        // |    4   |    Data bit 4    |    Data bit 4    |   Word select 1    |     Framing error     |
        // |        |                  |                  |          (CR4)     |          (FE)         |
        // +--------+------------------+------------------+--------------------+-----------------------+
        // |    5   |    Data bit 5    |    Data bit 5    | Transmit control 1 |    Receiver overrun   |
        // |        |                  |                  |          (CR5)     |        (OVRN)         |
        // +--------+------------------+------------------+--------------------+-----------------------+
        // |    6   |    Data bit 6    |    Data bit 6    | Transmit control 2 |    Parity error (PE)  |
        // |        |                  |                  |          (CR6)     |                       |
        // +--------+------------------+------------------+--------------------+-----------------------+
        // |    7   |    Data bit 7*** |    Data bit 7**  | Receive interrupt  |    Interrupt request  |
        // |        |                  |                  |   enable (CR7)     |      (IRQ active)     |
        // +--------+------------------+------------------+--------------------+-----------------------+
		//		* Leading bit = LSB = Bit 0
		//	   ** Data bit will be zero in 7-bit plus parity modes
		//	  *** Data bit is "don't care" in 7-bit plus parity modes

		enum ControlRegisters {
			CR0 =        0b1,	// Counter divide
			CR1 =       0b10,	//		"
			CR2 =      0b100,	// Word select
			CR3 =     0b1000,	//		"
			CR4 =    0b10000,	//		"
			CR5 =   0b100000,	// Transmit control
			CR6 =  0b1000000,	//		"
			CR7 = 0b10000000		// Receive control
		};

		// CR0 and CR1
		enum CounterDivideSelect {
			One			= 0b00,
			Sixteen		= 0b01,
			SixtyFour	= 0b10,
			MasterReset	= 0b11
		};

		// CR2, CR3 and CR4
		enum WordSelect {
			SevenEvenTwo		= 0b000,
			SevenOddTwo		= 0b001,
			SevenEvenOne		= 0b010,
			SevenOddOne		= 0b011,
			EightTwo			= 0b100,
			EightOne			= 0b101,
			EightEvenOne		= 0b110,
			EightOddOne		= 0b111,
		};

		// CR5 and CR6
		enum TransmitterControl {
			ReadyLowInterruptDisabled				= 0b00,
			ReadyLowInterruptEnabled					= 0b01,
			ReadyHighInterruptDisabled				= 0b10,
			ReadyLowInterruptDisabledTransmitBreak	= 0b11,
		};

		// CR7
		enum ReceiveControl {
			ReceiveInterruptDisable = 0b0,
			ReceiveInterruptEnable = 0b1,	// Triggers on: RDR full, overrun, DCD low -> high
		};

		// STATUS REGISTER Information on the status of the ACIA is
		// available to the MPU by reading the ACIA Status Register.
		// This read-only register is selected when RS is low and R/W is high.
		// Information stored in this register indicates the status of the
		// Transmit Data Register, the Receive Data Register and error logic,
		// and the peripheral/modem status inputs of the ACIA
		enum StatusRegisters {

			// Receive Data Register Full (RDRF), Bit 0 - Receive Data
			// Register Full indicates that received data has been
			// transferred to the Receive Data Register. RDRF is cleared
			// after an MPU read of the Receive Data Register or by a
			// master reset. The cleared or empty state indicates that the
			// contents of the Receive Data Register are not current.
			// Data Carrier Detect being high also causes RDRF to indicate
			// empty.
			STATUS_RDRF = 0b1,

			// Transmit Data Register Empty (TDRE), Bit 1 - The Transmit
			// Data Register Empty bit being set high indicates that the
			// Transmit Data Register contents have been transferred and
			// that new data may be entered. The low state indicates that
			// the register is full and that transmission of a new
			// character has not begun since the last write data command.
			STATUS_TDRE = 0b10,

			//						___
			// Data Carrier Detect (DCD), Bit 2 - The Data Carrier Detect
			// bit will be high when the DCD (low) input from a modem has gone
			// high to indicate that a carrier is not present. This bit
			// going high causes an Interrupt Request to be generated when
			// the Receive Interrupt Enable is set. It remains high after
			// the DCD (low) input is returned low until cleared by first reading
			// the Status Register and then the Data Register or until a
			// master reset occurs. If the DCD (low) input remains high after
			// read status and read data or master reset has occurred, the
			// interrupt is cleared, the DCD (low) status bit remains high and
			// will follow the DCD (low) input. 
			STATUS_DCD = 0b100,

			//				  ___
			// Clear-to-Send (CTS), Bit 3 - The Clear-to-Send bit indicates
			// the state of the Clear-to-Send input from a modem. A low CTS (low)
			// indicates that there is a Clear-to-Send from the modem. In
			// the high state, the Transmit Data Register Empty bit is
			// inhibited and the Clear-to-Send status bit will be high.
			// Master reset does not affect the Clear-to-Send status bit.
			STATUS_CTS = 0b1000,

			// Framing Error (FE), Bit 4 - Framing error indicates that the
			// received character is improperly framed by a start and a
			// stop bit and is detected by the absence of the first stop
			// bit. This error indicates a synchronization error, faulty
			// transmission, or a break condition. The framing error flag
			// is set or reset during the receive data transfer time.
			// Therefore, this error indicator is present throughout the
			// time that the associated character is available. 
			STATUS_FE = 0b10000,

			// Receiver Overrun (OVRN), Bit 5- Overrun is an error flag
			// that indicates that one or more characters in the data
			// stream were lost. That is, a character or a number of
			// characters were received but not read from the Receive
			// Data Register (RDR) prior to subsequent characters being
			// received. The overrun condition begins at the midpoint of
			// the last bit of the second character received in succession
			// without a read of the RDR having occurred. The Overrun does
			// not occur in the Status Register until the valid character
			// prior to Overrun has been read. The RDRF bit remains set
			// until the Overrun is reset. Character synchronization is
			// maintained during the Overrun condition. The Overrun
			// indication is reset after the reading of data from the
			// Receive Data Register or by a Master Reset. 
			STATUS_OVRN = 0b100000,

			// Parity Error (PE), Bit 6 - The parity error flag indicates
			// that the number of highs {ones) in the character does not
			// agree with the preselected odd or even parity. Odd parity
			// is defined to be when the total number of ones is odd. The
			// parity error indication will be present as long as the data
			// character is in the RDR. If no parity is selected, then both
			// the transmitter parity generator output and the receiver
			// parity check results are inhibited
			STATUS_PE = 0b1000000,

			//					  ___
			// Interrupt Request (IRQ), Bit 7- The IRQ (low) bit indicates the
			// state of the IRQ (low) output. Any interrupt condition with its
			// applicable enable will be indicated in this status bit.
			// Anytime the IRQ (low) output is low the IRQ bit will be high to
			// indicate the interrupt or service request status. IRQ (low) is
			// cleared by a read operation to the Receive Data Register or
			// a write operation to the Transmit Data Register. 
			STATUS_IRQ = 0b10000000,
		};

		// Data, (I/O)
		auto& DATA() { return m_data; }

		// Expose these internal registers, so we can update internal state

		// Transmit data register;
		auto& TDR() { return m_TDR; }

		// Receive data register;
		auto& RDR() { return m_RDR; }

		bool activated() { return powered() && raised(E()) && selected(); }
		bool selected() { return raised(CS0()) && raised(CS1()) && lowered(CS2()); }

		void markTransmitComplete();
		void markReceiveStarting();

		std::string dumpStatus();

		Signal<EventArgs> Accessing;
		Signal<EventArgs> Accessed;

		Signal<EventArgs> Transmitting;
		Signal<EventArgs> Transmitted;

		Signal<EventArgs> Receiving;
		Signal<EventArgs> Received;

	private:
		void step();

		uint8_t status();

		void reset();

		void startTransmit();
		void completeReceive();

		bool transmitInterruptEnabled() const { return m_transmitControl == ReadyLowInterruptEnabled;  }
		bool receiveInterruptEnabled() const { return m_receiveControl == ReceiveInterruptEnable;  }

		bool transmitReadyHigh() const { return m_transmitControl == ReadyHighInterruptDisabled; }
		bool transmitReadyLow() const { return !transmitReadyHigh(); }

		PinLevel m_oldDCD = PinLevel::Low;	// So we can detect low -> high transition

		uint8_t m_data = 0;

		bool m_statusRead = false;

		// Control registers
		CounterDivideSelect m_counterDivide = CounterDivideSelect::One;
		WordSelect m_wordSelect = WordSelect::SevenEvenTwo;
		TransmitterControl m_transmitControl = TransmitterControl::ReadyLowInterruptDisabled;
		ReceiveControl m_receiveControl = ReceiveControl::ReceiveInterruptDisable;

		// Status registers
		bool m_statusRDRF = false;
		bool m_statusTDRE = true;
		bool m_statusOVRN = false;

		// Data registers
		uint8_t m_TDR = 0;
		uint8_t m_RDR = 0;

		enum StartupCondition { ColdStart, WarmStart, Unknown };
		StartupCondition m_startup = WarmStart;
	};
}
