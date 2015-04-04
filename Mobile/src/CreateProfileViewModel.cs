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
			AddStep = ReactiveCommand.Create ();
			HostScreen = hostScreen;
			SomeTime = TimeSpan.FromMinutes (125);

			for (int i = 0; i <= 168; i++) {
				Hours.Add (i);
			}

			for (int i = 0; i <= 60; i++) {
				Mins.Add (i);
			}

			for (int i = 0; i <= 60; i++) {
				Secs.Add (i);
			}

			this.WhenAnyValue (x => x.SelectedHours, x => x.SelectedMins, x => x.SelectedSecs)
				.Select (x => {return x.Item1.ToString() + ":" + x.Item2.ToString() + ":" + x.Item3.ToString();})
				.ToProperty (this, x => x.SelectedStepTime, out _SelectedStepTime);

		}

		public ReactiveCommand<object> AddStep {get; protected set;}
		public ReactiveCommand<object> RemoveStep {get; protected set;}
		public ReactiveCommand<object> SaveProfile {get; protected set;}

		public ReactiveList<int> Hours = new ReactiveList<int>();
		public ReactiveList<int> Mins = new ReactiveList<int>();
		public ReactiveList<int> Secs = new ReactiveList<int>();

		private int _SelectedHours;
		public int SelectedHours
		{
			get { return _SelectedHours; }
			set { this.RaiseAndSetIfChanged(ref _SelectedHours, value); }
		}

		private int _SelectedMins;
		public int SelectedMins
		{
			get { return _SelectedMins; }
			set { this.RaiseAndSetIfChanged(ref _SelectedMins, value); }
		}

		private int _SelectedSecs;
		public int SelectedSecs
		{
			get { return _SelectedSecs; }
			set { this.RaiseAndSetIfChanged(ref _SelectedSecs, value); }
		}

		private ObservableAsPropertyHelper<string> _SelectedStepTime;
		[DataMember]
		public string SelectedStepTime { 
			get { return _SelectedStepTime .Value; }
		}

		private TimeSpan _SomeTime;
		public TimeSpan SomeTime
		{
			get { return _SomeTime; }
			set { this.RaiseAndSetIfChanged(ref _SomeTime, value); }
		}

		public string UrlPathSegment {
			get {
				return "Create Profile";
			}
		}

		public IScreen HostScreen { get; protected set;}
	}
}

