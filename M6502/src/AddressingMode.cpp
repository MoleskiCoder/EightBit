namespace Processor
{
	public enum AddressingMode
	{
		Illegal,
		Implied,
		Accumulator,
		Immediate,
		Relative,
		XIndexed,
		IndexedY,
		ZeroPage,
		ZeroPageX,
		ZeroPageY,
		Absolute,
		AbsoluteX,
		AbsoluteY,
		AbsoluteXIndirect,
		Indirect,
		ZeroPageIndirect,
		ZeroPageRelative
	}
}
