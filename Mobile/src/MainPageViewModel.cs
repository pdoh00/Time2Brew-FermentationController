using System;
using ReactiveUI;
using System.Runtime.Serialization;
using Polly;
using System.Net;
using System.Reactive.Linq;
using System.IO;
using System.Reactive;

namespace FermentationController
{
	public class MainPageViewModel :ReactiveObject, IRoutableViewModel
	{
		public MainPageViewModel (IFermentationControllerAPI fermApi)
		{
			var retry = Polly.Policy.Handle<WebException>().WaitAndRetryAsync(1, x=> TimeSpan.FromSeconds(Math.Pow(2, x)));

			SetTimeToNow = ReactiveCommand.CreateAsyncTask (async _=> {
				var secFromEpoch = DateTimeMixins.SecondsFromEpoch();
				await retry.ExecuteAsync(()=> fermApi.SetTime(secFromEpoch));
			});

			Echo = ReactiveCommand.CreateAsyncTask (async _=> {
				
					var response = await retry.ExecuteAsync(() => fermApi.Echo(this.EchoText));
					return response;
			});

			Echo.ToProperty (this, vm => vm.EchoResponse, out _EchoResponse);

			GetStatus = ReactiveCommand.CreateAsyncTask(async _=>{
				string output = string.Empty;
				var response = await retry.ExecuteAsync (fermApi.GetStatus);
				var theData = Convert.FromBase64String(response);
				using(var ms = new MemoryStream(theData))
				{
					var systemTime = ms.ReadUInt32();
					var systemMode = ms.ReadByte();
					var regMode = ms.ReadByte();
					var probe0Assignment = ms.ReadByte();
					var probe0Temp = ms.ReadUInt16(); 
					var probe1Assignment = ms.ReadByte();
					var probe1Temp = ms.ReadUInt16();
					var heatRelayStatus = ms.ReadByte();
					var coolRelayStatus = ms.ReadByte();
					var runningProfile = ms.ReadString(64);
					var profileStepIdx = ms.ReadUInt16();
					var profileStepTemp = ms.ReadUInt16();
					var profileStepTimeRemaining = ms.ReadUInt32();
					var manualSetpointTemp = ms.ReadUInt16();
					var profileStartTime = ms.ReadUInt32();

					output += string.Format("System Time:{0}\n", DateTimeMixins.TimeFromEpoch(systemTime));
					output += string.Format("System Mode:{0}\n", systemMode);
					output += string.Format("Regulation Mode:{0}\n", regMode);

					output += string.Format("Probe0 Assign:{0}\n", probe0Assignment);
					output += string.Format("Probe0 Temp C:{0}\n", probe0Temp / 10.0);

					output += string.Format("Probe1 Assign:{0}\n", probe1Assignment);
					output += string.Format("Probe1 Temp C:{0}\n", probe1Temp/ 10.0);

					output += string.Format("Heat Relay Status:{0}\n", heatRelayStatus);
					output += string.Format("Cool Relay Status:{0}\n", coolRelayStatus);

					output += string.Format("Running Profile:{0}\n", runningProfile);

					output += string.Format("Profile Step Index:{0}\n", profileStepIdx);
					output += string.Format("Profile Step Temperature C:{0}\n", profileStepTemp/ 10.0);
					output += string.Format("Profile Step Time Remaining:{0}\n", profileStepTimeRemaining);
					output += string.Format("Manual Setpoint Temp C:{0}\n", manualSetpointTemp/ 10.0);
					output += string.Format("Profile Start Time:{0}\n", DateTimeMixins.TimeFromEpoch(profileStartTime));
				}

					return output;
			});

			GetStatus.ThrownExceptions
				.Select (x => new UserError ("Status cannot be retrieved", "Check your connected to the TEMPERATURE wifi"))
				.SelectMany (UserError.Throw);

			GetStatus.ToProperty (this, vm => vm.StatusResponse, out _StatusResponse);

		}

		[IgnoreDataMember]
		public IScreen HostScreen {
			get;
			protected set;
		}

		[IgnoreDataMember]
		public string UrlPathSegment {
			get { return "Fermentation Controller"; }
		}

		[IgnoreDataMember] public ReactiveCommand<string> Echo { get; private set; }
		[IgnoreDataMember] public ReactiveCommand<string> GetStatus { get; private set; }
		[IgnoreDataMember] public ReactiveCommand<Unit> SetTimeToNow { get; protected set; }

		private string _EchoText;
		[DataMember]
		public string EchoText
		{
			get { return _EchoText; }
			set { this.RaiseAndSetIfChanged(ref _EchoText, value); }
		}

		private ObservableAsPropertyHelper<string> _EchoResponse;
		[DataMember]
		public string EchoResponse { 
			get { return _EchoResponse.Value; }
		}

		private ObservableAsPropertyHelper<string> _StatusResponse;
		[DataMember]
		public string StatusResponse { 
			get { return _StatusResponse.Value; }
		}

	}
}

