using System;
using ReactiveUI;
using System.Reactive.Linq;
using System.Runtime.Serialization;
using System.Threading.Tasks;

namespace FermentationController
{
	public class CreateProfileViewModel :ReactiveObject, IRoutableViewModel
	{
		public CreateProfileViewModel (IScreen hostScreen)
		{
			
			HostScreen = hostScreen;

			AddStep = ReactiveCommand.Create ();

			//Days
			IncrementTimeDays = ReactiveCommand.CreateAsyncTask (x => {
				return Task.FromResult (this.SelectedDays + 1);
			});

			IncrementTimeDays
				.StartWith (0)
				.Where (x => x >= 0)
				.ToProperty (this, x => x.SelectedDays, out _SelectedDays);

			DecrementTimeDays = ReactiveCommand.CreateAsyncTask (x => {
				return Task.FromResult (this.SelectedDays - 1);
			});

			DecrementTimeDays
				.Where (x => x >= 0)
				.ToProperty (this, x => x.SelectedDays, out _SelectedDays);

			//Hours
			IncrementTimeHours = ReactiveCommand.CreateAsyncTask (x => {
				return Task.FromResult (this.SelectedHours + 1);
			});

			IncrementTimeHours
				.StartWith (0)
				.Where (x => x >= 0)
				.ToProperty (this, x => x.SelectedHours, out _SelectedHours);

			DecrementTimeHours = ReactiveCommand.CreateAsyncTask (x => {
				return Task.FromResult (this.SelectedHours - 1);
			});

			DecrementTimeHours
				.Where (x => x >= 0)
				.ToProperty (this, x => x.SelectedHours, out _SelectedHours);

			//Mins
			IncrementTimeMins = ReactiveCommand.CreateAsyncTask (x => {
				return Task.FromResult ((int)x);
			});

			IncrementTimeMins
				.StartWith (0)
				.Where (x => x >= 0)
				.ToProperty (this, x => x.SelectedMins, out _SelectedMins);



			AddStep = ReactiveCommand.Create ();
			AddStep
				.Select (x => new StepData (ConvertToCelcius (this.StartingTemp), ConvertToCelcius (this.EndingTemp), new TimeSpan (this.SelectedDays, this.SelectedHours, this.SelectedMins, 0).TotalSeconds))
				.Subscribe (x => {
				//add to steps here
			});
		}

		public ReactiveCommand<object> AddStep { get; protected set; }

		public ReactiveCommand<object> RemoveStep { get; protected set; }

		public ReactiveCommand<object> SaveProfile { get; protected set; }

		public ReactiveCommand<int> IncrementTimeDays { get; protected set; }

		public ReactiveCommand<int> IncrementTimeHours { get; protected set; }

		public ReactiveCommand<int> IncrementTimeMins { get; protected set; }

		public ReactiveCommand<int> DecrementTimeDays { get; protected set; }

		public ReactiveCommand<int> DecrementTimeHours { get; protected set; }

		public ReactiveCommand<int> DecrementTimeMins { get; protected set; }


		private ObservableAsPropertyHelper<int> _SelectedDays;

		[DataMember]
		public int SelectedDays { 
			get { return _SelectedDays.Value; }
		}

		private ObservableAsPropertyHelper<int> _SelectedHours;

		[DataMember]
		public int SelectedHours { 
			get { return _SelectedHours.Value; }
		}

		private ObservableAsPropertyHelper<int> _SelectedMins;

		[DataMember]
		public int SelectedMins { 
			get { return _SelectedMins.Value; }
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

		static double ConvertToCelcius (double farenheit)
		{
			return (farenheit - 32.0) * (5.0 / 9.0);
		}

	}
}

