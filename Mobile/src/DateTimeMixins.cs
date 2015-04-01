using System;

namespace FermentationController
{
	public static class DateTimeMixins
	{
		static readonly DateTime Epoch = new DateTime (1970, 1, 1, 0,0,0, DateTimeKind.Utc);

		public static DateTime TimeFromEpoch(UInt32 secondsFromEpoch )
		{
			return Epoch.AddSeconds (secondsFromEpoch);
		}

		public static long SecondsFromEpoch()
		{
			return (long)(DateTime.UtcNow - Epoch).TotalSeconds;
		}
	}
}

