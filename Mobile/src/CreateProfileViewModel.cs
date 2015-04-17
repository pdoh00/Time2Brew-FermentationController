using System;
using ReactiveUI;
using System.Reactive.Linq;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using System.Reactive;

namespace FermentationController
{
	
	public class CreateProfileViewModel :ReactiveObject, IRoutableViewModel
	{
		public CreateProfileViewModel (IScreen hostScreen, IFermentationControllerAPI api)
		{
			HostScreen = hostScreen;

			AllSteps = new ReactiveList<StepData> ();

			StepTiles = AllSteps.CreateDerivedCollection (x => new ProfileStepTileViewModel (x.StepId, x));

			var canAddStep = this.WhenAnyValue (x => x.StepDays, x => x.StepHours, x => x.StepMins)
				.Select (x => x.Item1 > 0 || x.Item2 > 0 || x.Item3 > 0);
			
			AddStep = ReactiveCommand.Create (canAddStep);
			AddStep
				.ObserveOn(RxApp.MainThreadScheduler)
				.Select(x=> new StepData (TemperatureConvert.ConvertToCelsius (StartingTemp), 
					TemperatureConvert.ConvertToCelsius (EndingTemp), 
					new TimeSpan (StepDays, 
						StepHours, 
						StepMins, 
						0),
					AllSteps.Count + 1,
					IsRampStep))
				.Subscribe (AllSteps.Add);

			var canCommit = this.WhenAnyValue (x => x.Name, x => x.AllSteps).Select (x => !string.IsNullOrWhiteSpace (x.Item1) && x.Item2.Count > 0);
			CommitProfile = ReactiveCommand.CreateAsyncTask (canCommit, (x, ct) => {
				//TODO: create the profile dto
				return api.StoreProfile(Name, new byte[0]);
			}, RxApp.MainThreadScheduler);

			//on non-ramp steps the starting and ending temp are the same
			this.WhenAnyValue (x => x.StartingTemp, y => y.IsRampStep)
				.Where (x => !x.Item2)
				.Subscribe (x => EndingTemp = StartingTemp);
			
		}

		public ReactiveCommand<object> AddStep { get; protected set; }

		public ReactiveCommand<object> RemoveStep { get; protected set; }

		public ReactiveCommand<Unit> CommitProfile { get; protected set; }

		ReactiveList<StepData> AllSteps { get; set; }
	
		public IReactiveDerivedList<ProfileStepTileViewModel> StepTiles { get; private set; }

		private string _Name;
		[DataMember]
		public string Name { 
			get { return _Name; }
			set { this.RaiseAndSetIfChanged (ref _Name, value); }	
		}

		private double _StartingTemp;
		[DataMember]
		public double StartingTemp { 
			get { return _StartingTemp; }
			set { this.RaiseAndSetIfChanged (ref _StartingTemp, value); }	
		}

		private double _EndingTemp;
		[DataMember]
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

		private int _StepMins;

		public int StepMins { 
			get { return _StepMins; }
			set { this.RaiseAndSetIfChanged (ref _StepMins, value); }	
		}

		private bool _IsRampStep;
		[DataMember]
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

