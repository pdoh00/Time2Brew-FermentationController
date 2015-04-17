using System;
using ReactiveUI;
using System.Reactive;
using System.Reactive.Linq;

namespace FermentationController
{
	public class ProfileStepTileViewModel : ReactiveObject
	{
		public ProfileStepTileViewModel (int stepId, StepData data)
		{
			if(data.IsRampStep)
				StepDescription = string.Format ("{0}. {1} from {2:0}F to {3:0}F over {4}", 
					stepId, 
					"Ramp", 
					TemperatureConvert.ConvertToFarentheit(data.StartingTemp), 
					TemperatureConvert.ConvertToFarentheit(data.EndingTemp), 
					StringifyTimeSpan(data.Duration));
			else
				StepDescription = string.Format ("{0}. {1} {2:0}F for {3}", 
					stepId, 
					"Hold", 
					TemperatureConvert.ConvertToFarentheit(data.StartingTemp), 
					StringifyTimeSpan(data.Duration));

			EditStep = ReactiveCommand.Create ();
			EditStepFired = this.WhenAnyObservable (x => x.EditStep).Select(x=> stepId);

			DeleteStep = ReactiveCommand.Create ();
			DeleteStepFired = this.WhenAnyObservable (x => x.DeleteStep).Select (x => stepId);


		}

		public ReactiveCommand<object> EditStep { get; private set; }

		public ReactiveCommand<object> DeleteStep { get; private set; }

		public string StepDescription {
			get;
			private set;
		}

		public IObservable<int> EditStepFired {
			get;
			private set;
		}

		public IObservable<int> DeleteStepFired {
			get;
			private set;
		}

		static string StringifyTimeSpan(TimeSpan ts)
		{
			var time = string.Empty;

			if (ts.Days > 0)
				time = ts.Days + " days";
			if (ts.Hours > 0)
				time = time + " " + ts.Hours + " hours";
			if (ts.Minutes > 0)
				time = time + " " + ts.Minutes + " mins";

			return time;
		}
	}
}

