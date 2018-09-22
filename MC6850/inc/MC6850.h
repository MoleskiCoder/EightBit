#pragma once

#include <cstdint>

#include <Processor.h>

namespace EightBit {
	class mc6850 {
	public:
		// +--------+----------------------------------------------------------------------------------+
		// |		|								Buffer address									   |
		// |		+------------------+------------------+--------------------+-----------------------+
		// |        |			 _	   |			_	  |			   _	   |			   _	   |
		// | Data	|	  RS * R/W	   |	 RS * R/W	  |		RS * R/W	   |		RS * R/W	   |
		// |  Bus	|   (high)(low)	   |   (high)(high)   |	   (low)(low)	   |	   (low)(low)	   |
		// | Line	|	 Transmit	   |	 Receive	  |					   |					   |
		// | Number	|	  Data		   |	  Data		  |		 Control	   |		 Control	   |
		// |		|	 Register	   |	 Register	  |		 register	   |		 register	   |
		// |		+------------------+------------------+--------------------+-----------------------+
		// |		|  (Write only)    +   (Read only)	  +	   (Write only)	   |   (Read only)		   |
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
		// |		|				   |				  |		  (CR6)		   |					   ||
		// +--------+------------------+------------------+--------------------+-----------------------+
		// |	7	|	Data bit 7***  |	Data bit 7**  | Receive interrupt  |	Interrupt request  |
		// |		|				   |				  |   enable (CR7)	   |	  (IRQ active)	   |
		// +--------+------------------+------------------+--------------------+-----------------------+
		//		* Leading bit = LSB = Bit 0
		//	   ** Data bit will be zero in 7-bit plus parity modes
		//	  *** Data bit is "don't case" in 7-bit plus parity modes

		Processor::PinLevel& RXDATA() { return m_RXDATA; }	// Receive data, (I) Active high
		Processor::PinLevel& TXDATA() { return m_TXDATA; }	// Transmit data, (O) Active high

		Processor::PinLevel& RTS() { return m_RTS; }		// Request to send, (O) Active low
		Processor::PinLevel& CTS() { return m_CTS; }		// Clear to send, (I) Active low
		Processor::PinLevel& DCD() { return m_DCD; }		// Data carrier detect, (I) Active low

		Processor::PinLevel& RXCLK() { return m_RXCLK; }	// Transmit clock, (I) Active high
		Processor::PinLevel& TXCLK() { return m_TXCLK; }	// Receive clock, (I) Active high

		uint8_t& DATA() { return m_data; }					// Data, (I/O)

		Processor::PinLevel& CS0() { return m_CS0; }		// Chip select, bit 0, (I) Active high
		Processor::PinLevel& CS1() { return m_CS1; }		// Chip select, bit 1, (I) Active high
		Processor::PinLevel& CS2() { return m_CS2; }		// Chip select, bit 2, (I) Active low

		Processor::PinLevel& RS() { return m_RS; }			// Register select, (I) Active high
		Processor::PinLevel& RW() { return m_RW; }			// Read/Write, (I) Read high, write low

		Processor::PinLevel& E() { return m_E; }			// ACIA Enable, (I) Active high
		Processor::PinLevel& IRQ() { return m_IRQ; }		// Interrupt request, (O) Active low

	private:
		Processor::PinLevel m_RXDATA;
		Processor::PinLevel m_TXDATA;

		Processor::PinLevel m_RTS;
		Processor::PinLevel m_CTS;
		Processor::PinLevel m_DCD;

		Processor::PinLevel m_RXCLK;
		Processor::PinLevel m_TXCLK;

		uint8_t m_data;

		Processor::PinLevel m_CS0;
		Processor::PinLevel m_CS1;
		Processor::PinLevel m_CS2;

		Processor::PinLevel m_RS;
		Processor::PinLevel m_RW;

		Processor::PinLevel m_E;
		Processor::PinLevel m_IRQ;
	};
}
