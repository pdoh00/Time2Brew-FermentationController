using System;
using System.Collections.Generic;
using ReactiveUI;

using Xamarin.Forms;
using System.Reactive.Linq;
using ReactiveUI.XamForms;

namespace FermentationController
{
	public partial class CreateProfileView : ReactiveContentPage<CreateProfileViewModel>
	{
		public CreateProfileView ()
		{
			InitializeComponent ();

			this.BindCommand (ViewModel, vm => vm.AddStep, v => v.btnAddStep);
			this.BindCommand (ViewModel, vm => vm.CommitProfile, v => v.btnCommitProfile);

			this.Bind (ViewModel, vm => vm.Name, v => v.entryName.Text);

			//start temp
			this.Bind (ViewModel, vm => vm.StartingTemp, v => v.stpStartTemp.Value);
			this.Bind (ViewModel, vm => vm.StartingTemp, v => v.entryStartTemp.Text);

			this.Bind (ViewModel, vm => vm.StepDays, v => v.entryStepTimeDays.Text);
			this.Bind (ViewModel, vm => vm.StepHours, v => v.entryStepTimeHours.Text);
			this.Bind (ViewModel, vm => vm.StepMins, v => v.entryStepTimeMinutes.Text);

			//end temp
			this.Bind (ViewModel, vm => vm.EndingTemp, v => v.stpEndTemp.Value);
			this.Bind (ViewModel, vm => vm.EndingTemp, v => v.entryEndTemp.Text);

			//end temp = start temp when ramping is off
			this.WhenAnyValue (x => x.ViewModel.IsRampStep)
				.Subscribe (x => {
					this.stpEndTemp.IsVisible = x;
					this.entryEndTemp.IsVisible = x;
				});

			//Ramp
			this.Bind (ViewModel, vm => vm.IsRampStep, v => v.isRamp.IsToggled);

			this.OneWayBind (ViewModel, vm => vm.StepTiles, v => v.lstStepDescriptions.ItemsSource);
		}

//		public CreateProfileViewModel ViewModel {
//			get { return (CreateProfileViewModel)GetValue (ViewModelProperty); }
//			set { SetValue (ViewModelProperty, value); }
//		}
//
//		public static readonly BindableProperty ViewModelProperty =
//			BindableProperty.Create<CreateProfileView, CreateProfileViewModel> (x => x.ViewModel, default(CreateProfileViewModel), BindingMode.OneWay);
//
//		object IViewFor.ViewModel {
//			get { return ViewModel; }
//			set { ViewModel = (CreateProfileViewModel)value; }
//		}
	}
}

