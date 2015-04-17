using System;
using System.Collections.Generic;
using Xamarin.Forms;
using ReactiveUI.XamForms;
using ReactiveUI;
using System.Reactive.Linq;

namespace FermentationController
{
	public partial class PreferencesPageView : ReactiveContentPage<PreferencesPageViewModel>
	{
		public PreferencesPageView ()
		{
			InitializeComponent ();

			this.OneWayBind (ViewModel, vm => vm.TemperatureSettingText, v => v.lblTempPreference.Text);

			this.WhenAnyValue (x => x.switchTempPreference.IsToggled)
				.Select(x=> TemperatureUnit.Celsius)
				.Subscribe (_ => {
					if(ViewModel != null) this.ViewModel.TempPreference = TemperatureUnit.Celsius;
			});

			this.WhenAnyValue (x => x.switchTempPreference.IsToggled)
				.Where (toggled => !toggled)
				.Subscribe (_ => {
					if(ViewModel != null) this.ViewModel.TempPreference = TemperatureUnit.Fahrenheit;
				});

			this.WhenAnyValue (x=>x.ViewModel.TempPreference)
				.Subscribe (x => {
					this.switchTempPreference.IsToggled = x == TemperatureUnit.Celsius ? true : false;
			});
		}
	}
}

