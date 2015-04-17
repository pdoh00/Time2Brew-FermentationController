using System;
using System.Collections.Generic;
using Xamarin.Forms;
using ReactiveUI;
using ReactiveUI.XamForms;

namespace FermentationController
{
	public partial class ProfileStepTileView : ReactiveContentPage<ProfileStepTileViewModel>
	{
		public ProfileStepTileView ()
		{
			InitializeComponent ();

			this.BindCommand (ViewModel, vm => vm.DeleteStep, v => v.btnDeleteStep);
			this.BindCommand (ViewModel, vm => vm.EditStep, v => v.btnEditStep);
			this.OneWayBind (ViewModel, vm => vm.StepDescription, v => v.lblStepDescription.Text);

//			this.WhenActivated(x=>{
//				this.
//				});
		}
	}
}

