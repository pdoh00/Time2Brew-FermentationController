using System;
using ReactiveUI;
using ReactiveUI.XamForms;
using Splat;
using Xamarin.Forms;
using Refit;
using System.Net.Http;
using ModernHttpClient;
using Akavache;

namespace FermentationController
{
	public class AppBootstrapper : ReactiveObject, IScreen
	{
		// The Router holds the ViewModels for the back stack. Because it's
		// in this object, it will be serialized automatically.
		public RoutingState Router { get; protected set; }

		public AppBootstrapper ()
		{
			Router = new RoutingState ();

			Locator.CurrentMutable.RegisterConstant (this, typeof(IScreen));
			Locator.CurrentMutable.Register (() => new MainPageView (), typeof(IViewFor<MainPageViewModel>));
			Locator.CurrentMutable.Register (() => new CreateProfileView (), typeof(IViewFor<CreateProfileViewModel>));
			Locator.CurrentMutable.Register (() => new ProfileStepTileView(), typeof(IViewFor<ProfileStepTileViewModel>));
			Locator.CurrentMutable.Register (() => new PreferencesPageView (), typeof(IViewFor<PreferencesPageViewModel>));

			BlobCache.ApplicationName = "Time2BrewFermController";
			BlobCache.EnsureInitialized ();

			var fermApi = RestService.For<IFermentationControllerAPI> (new HttpClient (new NativeMessageHandler ())
				{
					BaseAddress = new Uri ("http://192.168.4.1/API")
				});

			Router.Navigate.Execute (new MainPageViewModel (fermApi, this));
		}

		public Page CreateMainPage ()
		{
			return new RoutedViewHost ();
		}
	}
}

