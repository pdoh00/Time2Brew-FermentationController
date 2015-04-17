using Xamarin.Forms;
using ReactiveUI;

namespace FermentationController
{
	public partial class MainPageView : ContentPage, IViewFor<MainPageViewModel>
	{
		public MainPageView ()
		{
			InitializeComponent ();

			this.Bind (ViewModel, vm => vm.EchoText, v => v.entryEcho.Text);

			this.BindCommand (ViewModel, vm => vm.SetTimeToNow, v => v.btnSetTimeNow);

			this.OneWayBind (ViewModel, vm => vm.EchoResponse, v => v.lblEchoReturnData.Text);
			this.BindCommand (ViewModel, vm => vm.Echo, v => v.btnEcho);

			this.OneWayBind (ViewModel, vm => vm.StatusResponse, v => v.lblStatusReturnData.Text);
			this.BindCommand (ViewModel, vm => vm.GetStatus, v => v.btnStatus);

			this.BindCommand (ViewModel, vm => vm.NavigateToCreateProfilePage, v => v.btnCreateProfile);
			this.BindCommand (ViewModel, vm => vm.NavigateToPreferencesPage, v => v.btnPreferences);

			UserError.RegisterHandler (async (arg) => {
				 await DisplayAlert (arg.ErrorMessage, arg.ErrorCauseOrResolution, "Cancel");
				return RecoveryOptionResult.CancelOperation;
			});


		}

		public MainPageViewModel ViewModel {
			get { return (MainPageViewModel)GetValue (ViewModelProperty); }
			set { SetValue (ViewModelProperty, value); }
		}

		public static readonly BindableProperty ViewModelProperty =
			BindableProperty.Create<MainPageView, MainPageViewModel> (x => x.ViewModel, default(MainPageViewModel), BindingMode.OneWay);

		object IViewFor.ViewModel {
			get { return ViewModel; }
			set { ViewModel = (MainPageViewModel)value; }
		}
	}
}

