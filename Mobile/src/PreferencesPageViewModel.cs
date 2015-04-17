using System;
using ReactiveUI;
using System.Runtime.Serialization;
using System.Reactive.Linq;

namespace FermentationController
{
	public class PreferencesPageViewModel :ReactiveObject, IRoutableViewModel
	{
		public PreferencesPageViewModel (IScreen hostScreen, UserSettings settings)
		{
			TempPreference = settings.MyTemperatureUnitPreference;
			HostScreen = hostScreen;

			this.WhenAnyValue (x => x.TempPreference)
				.Select (x => x == TemperatureUnit.Celsius ? "C" : "F")
				.ToProperty (this, x => x.TemperatureSettingText, out _TemperatureSettingText);

			this.WhenAnyValue (x => x.TempPreference)
				.StartWith (settings.MyTemperatureUnitPreference)
				.Subscribe (t=> settings.MyTemperatureUnitPreference = t);
		}



		[IgnoreDataMember]
		public IScreen HostScreen {
			get;
			protected set;
		}

		[IgnoreDataMember]
		public string UrlPathSegment {
			get { return "User Preferences"; }
		}

		private TemperatureUnit _TempPreference;
		[DataMember]
		public TemperatureUnit TempPreference { 
			get { return _TempPreference; }
			set { this.RaiseAndSetIfChanged (ref _TempPreference, value); }	
		}

		private ObservableAsPropertyHelper<string> _TemperatureSettingText;

		public string TemperatureSettingText { 
			get { return _TemperatureSettingText.Value; }
		}
	}
}

