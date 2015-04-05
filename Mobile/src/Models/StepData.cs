using System;

namespace FermentationController
{
	public class StepData
	{
		/// <summary>
		/// Initializes a new instance of the <see cref="FermentationController.StepData"/> class.
		/// </summary>
		/// <param name="startingTemp">Starting temperature in degrees Celcius.</param>
		/// <param name="endingTemp">Ending temperature in degrees Celcius.</param>
		/// <param name="duration">Duration in seconds.</param>
		public StepData (double startingTemp, double endingTemp, int duration)
		{
			this.StartingTemp = startingTemp;
			this.EndingTemp = endingTemp;
			this.Duration = duration;
		}

		public double StartingTemp {
			get;
			private set;
		}

		public double EndingTemp {
			get;
			private set;
		}

		/// <summary>
		/// Gets the duration.
		/// </summary>
		/// <value>The duration.</value>
		public int Duration {
			get;
			private set;
		}
	}
}

