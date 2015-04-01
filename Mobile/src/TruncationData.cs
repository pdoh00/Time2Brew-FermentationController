using System;
using System.Runtime.InteropServices;

namespace FermentationController
{
	[StructLayout (LayoutKind.Explicit, Pack = 1)]
	public struct TruncationData
	{
		[FieldOffset (0)]
		public ProfileStepData[] StepData;
	}
}

