using System;
using System.Runtime.InteropServices;

namespace FermentationController
{
	[StructLayout (LayoutKind.Explicit, Pack = 1)]
	public struct ControllerStatus
	{
		//Current System Time as Seconds from Epoch
		[FieldOffset (0)]
		public UInt32 SystemTime;

		//System Mode: 0 = Sensor Only, 1 = Manual Setpoint, 2=Profile Active
		[FieldOffset (4)]
		public byte SystemMode;

		//Regulation Mode: 0 = Simple Threshold, 1=Simple PID, 2=Complex Threshold 3=Complex PID
		[FieldOffset (5)]
		public byte RegulationMode;

		//Probe0 Assignment {0=Sensor, 1=Process, 2=Target}
		[FieldOffset (6)]
		public byte Probe0Assignment;

		//Probe0 Temperature in 0.1C increments
		[FieldOffset (7)]
		public Int16 Probe0Temperature;

		//Probe1 Assignment {0=Sensor, 1=Process, 2=Target}
		[FieldOffset (9)]
		public byte Probe1Assignment;

		//Probe1 Temperature in 0.1C increments
		[FieldOffset (10)]
		public Int16 Probe1Temperature;

		//Heat Relay Status {0=Off, 1=On}
		[FieldOffset (12)]
		public byte HeatRelayStatus;
		[FieldOffset (13)]
		//Cool Relay Status {0=Off, 1=On}
		public byte CoolRelayStatus;

		//64 byte NULL terminated string = Currently Running Profile.  Empty if no profile is active.
		//[MarshalAs(UnmanagedType.ByValArray, SizeConst=64)]
		[FieldOffset (14)]
		public byte[] RunningProfile;

		//Profile Step Index
		[FieldOffset (78)]
		public Int16 ProfileStepIndex;

		//Profile Step Temperature in 0.1C increments
		[FieldOffset (80)]
		public Int16 ProfileStepTemperature;

		//Profile Step Time Remaining (Seconds)
		[FieldOffset (82)]
		public UInt32 ProfileStepTimeRemaining;

		//Manual Setpoint 0.1C increments
		[FieldOffset (86)]
		public Int16 ManualSetpointTemperature;

		//Profile Start Time as Seconds from Epoch
		[FieldOffset (88)]
		public UInt32 ProfileStartTime;
	}


}

