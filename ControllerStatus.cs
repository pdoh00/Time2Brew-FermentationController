using System;
using System.Runtime.InteropServices;

namespace FermentationController
{
	[StructLayout(LayoutKind.Sequential, Pack=1, CharSet=CharSet.Ansi)]
	public struct ControllerStatus
	{
		//Current System Time as Seconds from Epoch
		public UInt32 SystemTime;
		//System Mode: 0 = Sensor Only, 1 = Manual Setpoint, 2=Profile Active
		public byte SystemMode;
		//Regulation Mode: 0 = Simple Threshold, 1=Simple PID, 2=Complex Threshold 3=Complex PID
		public byte RegulationMode;

		//Probe0 Assignment {0=Sensor, 1=Process, 2=Target}
		public byte Probe0Assignment;
		//Probe0 Temperature in 0.1C increments
		public Int16 Probe0Temperature;

		//Probe1 Assignment {0=Sensor, 1=Process, 2=Target}
		public byte Probe1Assignment;
		//Probe1 Temperature in 0.1C increments
		public Int16 Probe1Temperature;

		//Heat Relay Status {0=Off, 1=On}
		public byte HeatRelayStatus;
		//Cool Relay Status {0=Off, 1=On}
		public byte CoolRelayStatus;

		//64 byte NULL terminated string = Currently Running Profile.  Empty if no profile is active.
		[MarshalAs(UnmanagedType.ByValArray, SizeConst=64)]
		char[] RunningProfile;

		//Profile Step Index
		public Int16 ProfileStepIndex;
		//Profile Step Temperature in 0.1C increments
		public Int16 ProfileStepTemperature;
		//Profile Step Time Remaining (Seconds)
		public UInt32 ProfileStepTimeRemaining;
		//Manual Setpoint 0.1C increments
		public Int16 ManualSetpointTemperature;
		//Profile Start Time as Seconds from Epoch
		public UInt32 ProfileStartTime;
	}


}

