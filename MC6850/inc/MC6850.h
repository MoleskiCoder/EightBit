#pragma once

#include <cstdint>

#include <Chip.h>
#include <Signal.h>

namespace EightBit {
	class mc6850 : public Chip {
	public:
		// +--------+----------------------------------------------------------------------------------+
		// |		|								Buffer address									   |
		// |		+------------------+------------------+--------------------+-----------------------+
		// |        |			 _	   |			_	  |			   _	   |			   _	   |
		// |  Data	|	  RS * R/W	   |	 RS * R/W	  |		RS * R/W	   |		RS * R/W	   |
		// |  Bus	|   (high)(low)	   |   (high)(high)   |	   (low)(low)	   |	   (low)(low)	   |
		// |  Line	|	 Transmit	   |	 Receive	  |					   |					   |
		// | Number	|	   Data		   |	  Data		  |		 Control	   |		 Status		   |
		// |		|	 Register	   |	 Register	  |		 register	   |		register	   |
		// |		+------------------+------------------+--------------------+-----------------------+
		// |		|  (Write only)    +   (Read only)	  +	   (Write only)	   |      (Read only)	   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	0	|	Data bit 0*	   |	Data bit 0	  |   Counter divide   | Receive data register |
		// |		|				   |				  |	  select 1 (CR0)   |	  full (RDRF)	   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	1	|	Data bit 1	   |	Data bit 1	  |   Counter divide   | Transmit data register|
		// |		|				   |				  |   select 2 (CR1)   |	 empty (TDRE)	   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	2	|	Data bit 2	   |	Data bit 2	  |   Word select 1	   |  Data carrier detect  |
		// |		|				   |				  |		  (CR2)		   |	  (DCD active)	   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	3	|	Data bit 3	   |	Data bit 3	  |   Word select 1	   |	 Clear to send	   |
		// |		|				   |				  |		  (CR3)		   |	  (CTS active)	   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	4	|	Data bit 4	   |	Data bit 4	  |	  Word select 1	   |	 Framing error	   |
		// |		|				   |				  |		  (CR4)		   |		  (FE)		   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	5	|	Data bit 5	   |	Data bit 5	  | Transmit control 1 |	Receiver overrun   |
		// |		|				   |				  |		  (CR5)		   |		(OVRN)		   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	6	|	Data bit 6	   |	Data bit 6	  | Transmit control 2 |	Parity error (PE)  |
		// |		|				   |				  |		  (CR6)		   |					   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	7	|	Data bit 7***  |	Data bit 7**  | Receive interrupt  |	Interrupt request  |
		// |		|				   |				  |   enable (CR7)	   |	  (IRQ active)	   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		//		* Leading bit = LSB = Bit 0
		//	   ** Data bit will be zero in 7-bit plus parity modes
		//	  *** Data bit is "don't case" in 7-bit plus parity modes

		enum ControlRegisters {
			CR0 = 0b1,
			CR1 = 0b10,
			CR2 = 0b100,
			CR3 = 0b1000,
			CR4 = 0b10000,
			CR5 = 0b100000,
			CR6 = 0b1000000,
			CR7 = 0b10000000
		};
		
		enum StatusRegisters {
			RDRF	= 0b1,
			TDRE	= 0b10,
			kDCD	= 0b100,
			kCTS	= 0b1000,
			FE		= 0b10000,
			OVRN	= 0b100000,
			PE		= 0b1000000,
			kIRQ	= 0b10000000,
		};

		PinLevel& RXDATA() { return m_RXDATA; }	// Receive data, (I) Active high
		PinLevel& TXDATA() { return m_TXDATA; }	// Transmit data, (O) Active high

		PinLevel& RTS() { return m_RTS; }		// Request to send, (O) Active low
		PinLevel& CTS() { return m_CTS; }		// Clear to send, (I) Active low
		PinLevel& DCD() { return m_DCD; }		// Data carrier detect, (I) Active low

		PinLevel& RXCLK() { return m_RXCLK; }	// Transmit clock, (I) Active high
		PinLevel& TXCLK() { return m_TXCLK; }	// Receive clock, (I) Active high


		PinLevel& CS0() { return m_CS0; }		// Chip select, bit 0, (I) Active high
		PinLevel& CS1() { return m_CS1; }		// Chip select, bit 1, (I) Active high
		PinLevel& CS2() { return m_CS2; }		// Chip select, bit 2, (I) Active low

		PinLevel& RS() { return m_RS; }			// Register select, (I) Active high
		PinLevel& RW() { return m_RW; }			// Read/Write, (I) Read high, write low

		PinLevel& E() { return m_E; }			// ACIA Enable, (I) Active high
		PinLevel& IRQ() { return m_IRQ; }		// Interrupt request, (O) Active low

		uint8_t& DATA() { return m_data; }		// Data, (I/O)

		void step(int cycles);

		void fillRDR();
		void drainTDR();

		Signal<EventArgs> Accessing;
		Signal<EventArgs> Accessed;

		Signal<EventArgs> Transmitting;
		Signal<EventArgs> Transmitted;

		Signal<EventArgs> Receiving;
		Signal<EventArgs> Received;

	private:
		uint8_t& TDR() { return m_TDR; }	// Transmit data register;
		uint8_t& RDR() { return m_RDR; }	// Receive data register;

		uint8_t& status() { return m_status; }

		bool selected();

		void reset();

		void step();

		void drainRDR();
		void fillTDR();

		PinLevel m_RXDATA;
		PinLevel m_TXDATA;

		PinLevel m_RTS;
		PinLevel m_CTS;
		PinLevel m_DCD;

		PinLevel m_RXCLK;
		PinLevel m_TXCLK;

		PinLevel m_CS0;
		PinLevel m_CS1;
		PinLevel m_CS2;

		PinLevel m_RS;
		PinLevel m_RW;

		PinLevel m_E;
		PinLevel m_IRQ;

		uint8_t m_data;

		// Control registers
		int m_counterDivide;
		int m_wordConfiguration;
		int m_transmitterControl;
		int m_receiveControl;

		// Status registers
		uint8_t m_status;

		// Data registers
		uint8_t m_TDR;
		uint8_t m_RDR;

		bool m_powered = false;
	};
}
