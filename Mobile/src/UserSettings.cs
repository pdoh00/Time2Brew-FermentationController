using System;
using Lager;
using Akavache;

namespace FermentationController
{
	public enum TemperatureUnit
	{
		Celsius=0,
		Fahrenheit
	}

	public class UserSettings : SettingsStorage
	{
		public UserSettings ()
			:base("fe02a3f5-fa91-48d4-a75b-c1bd6f220575", BlobCache.UserAccount)
		{
			
		}

		public TemperatureUnit MyTemperatureUnitPreference {
			get{ return this.GetOrCreate (TemperatureUnit.Fahrenheit); }
			set{ this.SetOrCreate (value); }
		}
	}
}

