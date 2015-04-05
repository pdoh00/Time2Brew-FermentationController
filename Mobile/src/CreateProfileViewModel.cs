using System;
using ReactiveUI;
using System.Reactive.Linq;
using System.Runtime.Serialization;

namespace FermentationController
{
	public class CreateProfileViewModel :ReactiveObject, IRoutableViewModel
	{
		public CreateProfileViewModel (IScreen hostScreen)
		{
			
			HostScreen = hostScreen;

			AddStep = ReactiveCommand.Create ();


		}

		public ReactiveCommand<object> AddStep { get; protected set; }

		public ReactiveCommand<object> CommitStep { get; private set; }

		public ReactiveCommand<object> RemoveStep { get; protected set; }

		public ReactiveCommand<object> SaveProfile { get; protected set; }


		private int _SelectedDays;

		public int SelectedDays {
			get { return _SelectedDays; }
			set { this.RaiseAndSetIfChanged (ref _SelectedDays, value); }
		}

		private int _SelectedHours;

		public int SelectedHours {
			get { return _SelectedHours; }
			set { this.RaiseAndSetIfChanged (ref _SelectedHours, value); }
		}

		private int _SelectedMins;

		public int SelectedMins {
			get { return _SelectedMins; }
			set { this.RaiseAndSetIfChanged (ref _SelectedMins, value); }
		}

		private double _StartingTemp;

		public double StartingTemp { 
			get { return _StartingTemp; }
			set { this.RaiseAndSetIfChanged (ref _StartingTemp, value); }	
		}

		private double _EndingTemp;

		public double EndingTemp { 
			get { return _EndingTemp; }
			set { this.RaiseAndSetIfChanged (ref _EndingTemp, value); }	
		}

		private int _StepDays;

		public int StepDays { 
			get { return _StepDays; }
			set { this.RaiseAndSetIfChanged (ref _StepDays, value); }	
		}

		private int _StepHours;

		public int StepHours { 
			get { return _StepHours; }
			set { this.RaiseAndSetIfChanged (ref _StepHours, value); }	
		}

		private int _StepMinutes;

		public int StepMinutes { 
			get { return _StepMinutes; }
			set { this.RaiseAndSetIfChanged (ref _StepMinutes, value); }	
		}

		private bool _IsRampStep;

		public bool IsRampStep { 
			get { return _IsRampStep; }
			set { this.RaiseAndSetIfChanged (ref _IsRampStep, value); }	
		}

		public string UrlPathSegment {
			get {
				return "Create Profile";
			}
		}

		public IScreen HostScreen { get; protected set; }
	}
}

