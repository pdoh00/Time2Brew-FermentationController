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
		/// <param name = "stepId">The step id. Also the ordered step number</param>
		/// <param name = "isRampStep">True if ramp step. False if hold step</param>
		public StepData (double startingTemp, double endingTemp, TimeSpan duration, int stepId, bool isRampStep)
		{
			StartingTemp = startingTemp;
			EndingTemp = endingTemp;
			Duration = duration;
			StepId = stepId;
			IsRampStep = isRampStep;
		}

		/// <summary>
		/// Gets the step identifier. this also
		/// is used to order the step appropriately.
		/// </summary>
		/// <value>The step identifier.</value>
		public int StepId {
			get;
			private set;
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
		/// Gets the step duration.
		/// </summary>
		/// <value>The duration</value>
		public TimeSpan Duration {
			get;
			private set;
		}

		/// <summary>
		/// Gets a value indicating whether this instance is ramp step. 
		/// Ramp indicates start temperature is different than end temp.
		/// </summary>
		/// <value><c>true</c> if this instance is ramp step; otherwise, <c>false</c>.</value>
		public bool IsRampStep {
			get;
			private set;
		}
	}
}

