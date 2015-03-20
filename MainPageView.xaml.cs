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
			this.OneWayBind (ViewModel, vm => vm.EchoResponse, v => v.lblEchoReturnData.Text);
			this.BindCommand (ViewModel, vm => vm.Echo, v => v.btnEcho);
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

