using System;
using System.Runtime.InteropServices;

namespace FermentationController
{
	[StructLayout (LayoutKind.Explicit, Pack = 1)]
	public struct ProfileStepData
	{
		//Step Starting Temperature in 0.1C increments
		[FieldOffset (0)]
		public Int16 StepStartingTemperature;

		//Step Ending Temperature in 0.1C increments
		[FieldOffset (2)]
		public Int16 StepEndingTemperature;

		//Step Duration (Seconds)
		[FieldOffset (4)]
		public UInt32 StepDuration;
	}
}

