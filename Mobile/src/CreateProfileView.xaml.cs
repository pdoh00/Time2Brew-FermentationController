using System;
using System.Collections.Generic;
using ReactiveUI;
using Xamarin.Forms;

namespace FermentationController
{
	public partial class CreateProfileView : ContentPage, IViewFor<CreateProfileViewModel>
	{
		public CreateProfileView ()
		{
			InitializeComponent ();

//			this.Bind (ViewModel, vm => vm.SomeTime, v => v.time.Time);
			this.OneWayBind (ViewModel, vm => vm.Hours, v => v.listHours.ItemsSource);
			this.OneWayBind (ViewModel, vm => vm.Mins, v => v.listMins.ItemsSource);
			this.OneWayBind (ViewModel, vm => vm.Secs, v => v.listSecs.ItemsSource);

			this.Bind (ViewModel, vm => vm.SelectedHours, v => v.listHours.SelectedItem);
			this.Bind (ViewModel, vm => vm.SelectedMins, v => v.listMins.SelectedItem);
			this.Bind (ViewModel, vm => vm.SelectedSecs, v => v.listSecs.SelectedItem);

			this.OneWayBind (ViewModel, vm => vm.SelectedStepTime, v => v.lblSelectedTime.Text);
		}

		public CreateProfileViewModel ViewModel {
			get { return (CreateProfileViewModel)GetValue (ViewModelProperty); }
			set { SetValue (ViewModelProperty, value); }
		}

		public static readonly BindableProperty ViewModelProperty =
			BindableProperty.Create<CreateProfileView, CreateProfileViewModel> (x => x.ViewModel, default(CreateProfileViewModel), BindingMode.OneWay);

		object IViewFor.ViewModel {
			get { return ViewModel; }
			set { ViewModel = (CreateProfileViewModel)value; }
		}
	}
}

