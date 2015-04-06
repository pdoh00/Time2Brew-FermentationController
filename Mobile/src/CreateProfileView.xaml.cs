using System;
using System.Collections.Generic;
using ReactiveUI;
using Xamarin.Forms;
using System.Reactive.Linq;

namespace FermentationController
{
	public partial class CreateProfileView : ContentPage, IViewFor<CreateProfileViewModel>
	{
		public CreateProfileView ()
		{
			InitializeComponent ();

			this.Bind (ViewModel, vm => vm.StartingTemp, v => v.stpStartTemp.Value);
			this.Bind (ViewModel, vm => vm.EndingTemp, v => v.stpEndTemp.Value);

			this.WhenActivated (d => {
				d (this.WhenAnyValue (x => x.stpStepTime.Value)
					.SelectMany (x => ViewModel.IncrementTimeMins.ExecuteAsync (x))
					.Subscribe ());
			});


			//this.Bind (ViewModel, vm => vm.SelectedDays, v => v.entryStepTimeDays.Value);

//			this.Bind (ViewModel, vm => vm.SomeTime, v => v.time.Time);
//			this.OneWayBind (ViewModel, vm => vm.Hours, v => v.listHours.ItemsSource);
//			this.OneWayBind (ViewModel, vm => vm.Mins, v => v.listMins.ItemsSource);
//			this.OneWayBind (ViewModel, vm => vm.Secs, v => v.listSecs.ItemsSource);
//
//			this.Bind (ViewModel, vm => vm.SelectedHours, v => v.listHours.SelectedItem);
//			this.Bind (ViewModel, vm => vm.SelectedMins, v => v.listMins.SelectedItem);
//			this.Bind (ViewModel, vm => vm.SelectedSecs, v => v.listSecs.SelectedItem);

//			this.OneWayBind (ViewModel, vm => vm.SelectedStepTime, v => v.lblSelectedTime.Text);
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

