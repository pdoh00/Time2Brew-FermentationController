using System;

using Android.App;
using Android.Content;
using Android.Content.PM;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using Android.OS;
using Xamarin.Forms;
using Xamarin.Forms.Platform.Android;
using ReactiveUI;

namespace FermentationController.Droid
{
	[Activity (Label = "FermentationController.Droid", Icon = "@drawable/icon", MainLauncher = true, ConfigurationChanges = ConfigChanges.ScreenSize | ConfigChanges.Orientation)]
	public class MainActivity : global::Xamarin.Forms.Platform.Android.FormsApplicationActivity
	{
		protected override void OnCreate (Bundle bundle)
		{
			base.OnCreate (bundle);
			Forms.Init (this, bundle);
			var mainPage = RxApp.SuspensionHost.GetAppState<AppBootstrapper> ().CreateMainPage ();
			this.SetPage (mainPage);
//			LoadApplication (new App ());
		}
	}
}

