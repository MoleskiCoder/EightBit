#pragma once

#include <cstdint>

#include <ClockedChip.h>
#include <Signal.h>
#include <Ram.h>

/*
	PIA 6532 combined timer, IO and 128 bytes RAM
	=============================================

	MOS 6532 pin configuration
	--------------------------
			  ____________
			 |     |_|    |
		VSS -| 1        40|- A6
			 |            |
		 A5 -| 2        39|- theta 2
			 |            |
		 A4 -| 3        38|- CS1
			 |            |  ___
		 A3 -| 4        37|- CS2
			 |            |  __
		 A2 -| 5        36|- RS
			 |            |
		 A1 -| 6        35|- R/W
			 |            |  ___
		 A0 -| 7        34|- RES
			 |            |
		PA0 -| 8        33|- D0
			 |            |
		PA1 -| 9        32|- D1
			 |            |
		PA2 -|10        31|- D2
			 |            |
		PA3 -|11        30|- D3
			 |            |
		PA4 -|12        29|- D4
			 |            |
		PA5 -|13        28|- D5
			 |            |
		PA6 -|14        27|- D6
			 |            |
		PA7 -|15        26|- D7
			 |            |  ___
		PB7	-|16        25|- IRQ
			 |            |
		PB6	-|17        24|- PB0
			 |            |
		PB5	-|18        23|- PB1
			 |            |
		PB4	-|19        22|- PB2
			 |            |
		Vcc	-|20        21|- PB3
			 |____________|

	VSS		= power
	A0-A6	= address
	PA0-PA7	= port A
	PB0-PB7	= port B
	Vcc		= ground
	theta 2	= clock input
	CS1		= chip select 1
	CS2		= chip select 2 (low)
	RS		= RAM select (low)
	R/W		= read/write
	RES		= reset (low)
	D0 - D7	= data
	IRQ		= interrupt request (low)

	MOS 6532 Block Diagram
	======================
	           ___________                _____________
	 D0 <---->|           |      __      |     DATA    |
	 D1 <---->|   DATA    |      ||<---->|  DIRECTION  |
	 D2 <---->|    BUS    |      ||      |   CONTROL   |
	 D3 <---->|  BUFFER   |<---->||      |  REGISTER   |
	 D4 <---->|           |      ||      |      A      |
	 D5 <---->|           |      ||      |_____________|-----
	 D6 <---->|           |      ||                         |
	 D7 <---->|___________|      ||       _____________     |
	                             ||      |   OUTPUT    |    |
			   ___________       ||<---->| REGISTER A  |    |
	CS1  ---->|           |      ||      |_____________|    |
	___       |   CHIP    |      ||             |           |
	CS2  ---->|  SELECT   |      ||             V           |
	          |   R/W     |      ||       _____________     |
	 02  ---->|           |      ||      |             |<----
	          |           |      ||      | PERIPHERAL  |<----> PA7
	R/W  ---->|           |      ||      |    DATA     |<----> PA6
	___       |           |      ||<-----|   BUFFER    |<----> PA5
	RES  ---->|___________|      ||      |     A       |<----> PA4
			                     ||      |             |<----> PA3
			   ___________       ||      |             |<----> PA2
	 __       | 128 x 8   |      ||      |             |<----> PA1
	 RS  ---->|   RAM     |<---->||      |_____________|<----> PA0
	          |___________|      ||
			                     ||       _____________
			                     ||      |             |<----> PB7
			   ___________       ||      | PERIPHERAL  |<----> PB6
	___       | INTERRUPT |      ||      |    DATA     |<----> PB5
	IRQ  ---->|  CONTROL  |<---->||<-----|   BUFFER    |<----> PB4
	          |___________|      ||      |     B       |<----> PB3
	                ^            ||      |             |<----> PB2
	                |            ||      |             |<----> PB1
	                V            ||      |             |<----> PB0
	           ___________       ||      |_____________|<----
	          | INTERVAL  |      ||             |           |
	          |   TIMER   |<---->||             V           |
	          |___________|      ||       _____________     |
			   ___________       ||      |   OUTPUT    |    |
	 A0  ---->|           |      ||<---->| REGISTER B  |    |
	 A1  ---->|           |      ||      |_____________|    |
	 A2  ---->| ADDRESS   |      ||                         |
	 A3  ---->| DECODERS  |      ||       _____________     |
	 A4  ---->|           |      ||      |     DATA    |-----
	 A5  ---->|           |      ||      |  DIRECTION  |
	 A6  ---->|___________|      ||<---->|   CONTROL   |
								 --      |  REGISTER   |
								         |      B      |
								         |_____________|

	Basic Elements of Interval Timer
	--------------------------------

				R/W	PA7	A3				D7	D6	D5	D4	D3	D2	D1	D0		R/W			A1		A0
				 |   |   |				|	|	|	|	|	|	|	|		 |			|		|
				 V   V   V				V	V	V	V	V	V	V	V		 |			V		V
			+---------------+		+-----------------------------------+	 |		+---------------+
	___		|				|		|									|<---+----->|				|<---- theta 2
	IRQ <---|	Interrupt	|<------|			Programmable			|			|	Divide		|
			|	Control		|		|			Register				|			|	Down		|
			|				|		|									|<----------|				|
			+---------------+		+-----------------------------------+			+---------------+
				|		|				|	|	|	|	|	|	|	|
				+-----------------------+	|	|	|	|	|	|	|
				|		|					|	|	|	|	|	|	|
				|		+-------------------+	|	|	|	|	|	|
				|		|						|	|	|	|	|	|
				V		V						V	V	V	V	V	V
				D7		D6						D5	D4	D3	D2	D1	D0

	RAM128 Bytes (1024 Bits)
	The 128 x 8 Read/Write memory acts as a conventional static RAM. Data can be written into the RAM from
	the microprocessor by selecting the chip (CS1 = 1, CS2 = 0) and by setting RS to a logic 0 (0.4v). Address
	lines AO through A6 are then used to select the desired byte of storage.

	Addressing Decode
	-----------------
								__
								RS	R/W	A4	A3	A2	A1	A0
	Write RAM					0	0	-	-	-	-	-
	Read RAM					0	1	-	-	-	-	-
	Write DDRA					1	0	-	-	0	0	1
	Read DDRA					1	1	-	-	0	0	1
	Write DDRB					1	0	-	-	0	1	1
	Read DDRB					1	1	-	-	0	1	1
	Write Output Reg A			1	0	-	-	0	0	0
	Read Output Reg A			1	1	-	-	0	0	0
	Write Output Reg B			1	0	-	-	0	1	0
	Read Output Reg B			1	1	-	-	0	1	0
	Write Timer
		+ 1T					1	0	1	(a)	1	0	0
		+ 8T					1	0	1	(a)	1	0	1
		+ 64T					1	0	1	(a)	1	1	0
		+ 1024T					1	0	1	(a)	1	1	1
	Read Timer					1	1	-	(a)	1		0
	Read Interrupt Flag(s)		1	1			1		1
	Write Edge Detect Control	1	0	0		1	(b) (c)

	(a)	A3 = 0 to disable interrupt trom timer to IRQ
		A3 = 1 to enable interrupt from timer to IRQ
	(b) A1 = 0 to disable interrupt from PA7 to IRQ
		A1 = 1 to enable interrupt from PA7 to IRQ
	(c) A0 = 0 for negative edge-detect
		AO = 1 for positive edge-detect
*/


namespace EightBit {
	class M6532 final : public ClockedChip {
	public:
		M6532() noexcept;
		virtual ~M6532() = default;

		/*     ___
		Reset (RES)
		During system initialization a logic "0" on the RES input will cause a zeroing of all four I/O registers. This
		in turn will cause all I/O buses to act as inputs thus protecting external components from possible damage
		and erroneous data while the system is being configured under software control. The Data Bus Buffers are
		put into an OFF-STATE during Reset. Interrupt capability is disabled with the RES signal. The RES signal
		must be held low for at least one clock period when reset is required.
		*/
		DECLARE_PIN_INPUT(RES)

		/*            _
		Read/Write (R/W)
		The R/W signal is supplied by the microprocessor array and is used to control the transfer of data to and
		from the microprocessor array and the 6532. A high on the R/W pin allows the processor to read (with proper
		addressing) the data supplied by the 6532. A low on the R/W pin allows a write (with proper addressing) to
		the 6532.
		*/
		DECLARE_PIN_INPUT(RW)

		/*                 ___
		Interrupt Request (IRQ)
		The IRQ pin is an interrupt pin from the interrupt control logic. The pin will be normally high with a low indicating
		an interrupt from the 6532. An external pull-up device is required. The IRQ pin may be activated by a
		transition on PA7 or timeout of the interval timer.
		*/
		DECLARE_PIN_OUTPUT(IRQ)

		/*
		Data Bus (D0-D7)
		The 6532 has eight bi-directional data pins (D0-D7). These pins connect to the system's data lines and
		allow transfer of data to and from the microprocessor array. The output buffers remain in the off state except
		when a Read operation occurs and are capable of driving one standard TTL load and 130 pf.
		*/
		auto& DATA() { return m_data; }

		/*
		Peripheral Data Ports
		The 6532 has 16 pins available for peripheral I/O operations. Each pin is individually software programmable
		to act as either an input or an output. The 16 pins are divided into 2 8-bit ports, PA0-PA7 and PB0-PB7.
		PA7 also has other uses which are discussed in later sections. The pins are set up as an input by writing a
		"0" into the corresponding bit of the data direction register. A "1" into the data direction register will cause
		its corresponding bit to be an output. When in the input mode, the peripheral output buffers are in the "1"
		state and pull-up device acts as less than one TTL load to the peripheral data lines. On a Read operation, the
		microprocessor unit reads the peripheral pin. When the peripheral device gets information from the 6532 it
		receives data stored in the data register. The microprocessor will read correct information if the peripheral
		lines are greater than 2.0 volts for a "1" and less than 0.8 volts for a "0" as the peripheral pins are all TTL
		compatible. Pins PB0-PB7 are also capable of sourcing 3 ma at 1.5v, thus making them capable of Darlington
		drive.
		*/
		uint8_t& PA() { return m_pa; }
		uint8_t& PB() { return m_pb; }

		/*
		Address Lines (A0-A6)
		There are 7 address pins. In addition to these 7, there is a RAM SELECT pin. These pins, A0-A6 and RAM
		SELECT, are always used as addressing pins. There are two additional pins which are used as CHIP
		SELECTS. They are pins CS1 and CS2.
		*/
		uint8_t& address() { return m_address; }

		// RAM SELECT, active low
		DECLARE_PIN_INPUT(RS)

		// CHIP SELECT 1, active high
		DECLARE_PIN_INPUT(CS1)

		// CHIP SELECT 2, active low
		DECLARE_PIN_INPUT(CS2)

		bool activated() { return powered() && selected(); }
		bool selected() { return raised(CS1()) && lowered(CS2()); }

		Signal<EventArgs> Accessing;
		Signal<EventArgs> Accessed;

	private:
		void step();

		void reset();

		enum EdgeDetect { Positive, Negative };
		enum TimerIncrement { One = 1, Eight = 8, SixtyFour = 64, OneThousandAndTwentyFour = 1024 };

		auto& RAM() { return m_ram; }

		auto& DDRA() { return m_ddra; }
		auto& DDRB() { return m_ddrb; }

		auto& IF() { return m_interruptFlags; }

		uint8_t m_address;
		uint8_t m_data;

		uint8_t m_pa;
		uint8_t m_dra;
		uint8_t m_ddra;

		uint8_t m_pb;
		uint8_t m_drb;
		uint8_t m_ddrb;

		Ram m_ram = 0x80;

		bool m_allowTimerInterrupts;
		bool m_allowPA7Interrupts;
		EdgeDetect m_edgeDetection;

		TimerIncrement m_timerIncrement;
		uint8_t m_currentIncrement;
		uint8_t m_timerInterval;
		bool m_timerInterrupt;

		uint8_t m_interruptFlags;
	};
}
