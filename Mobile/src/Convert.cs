using System;

namespace FermentationController
{
	public static class TemperatureConvert
	{
		public static double ConvertToCelsius (double farenheit)
		{
			return (farenheit - 32.0) * (5.0 / 9.0);
		}

		public static double ConvertToFarentheit(double celsius)
		{
			return celsius * (9.0 / 5.0) + 32.0;
		}
	}
}

