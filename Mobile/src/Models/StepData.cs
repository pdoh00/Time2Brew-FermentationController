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

		/// <summary>
		/// Gets the starting temp in Celcius.
		/// </summary>
		/// <value>The starting temp in Celcius.</value>
		public double StartingTemp {
			get;
			private set;
		}

		/// <summary>
		/// Gets the ending temp in Celcius.
		/// </summary>
		/// <value>The ending temp in Celcius.</value>
		public double EndingTemp {
			get;
			private set;
		}

		/// <summary>
		/// Gets the duration in seconds.
		/// </summary>
		/// <value>The duration in seconds.</value>
		public int Duration {
			get;
			private set;
		}
	}
}

