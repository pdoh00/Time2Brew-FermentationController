using System;
using ReactiveUI;
using System.Runtime.Serialization;
using Polly;
using System.Net;

namespace FermentationController
{
	public class MainPageViewModel :ReactiveObject, IRoutableViewModel
	{
		public MainPageViewModel (IFermentationControllerAPI fermApi)
		{
			var retry = Polly.Policy.Handle<WebException>().WaitAndRetryAsync(3, x=> TimeSpan.FromSeconds(Math.Pow(2, x)));

			Echo = ReactiveCommand.CreateAsyncTask (async _=> {
				try
				{
					var response = await retry.ExecuteAsync(() => fermApi.Echo(this.EchoText));
					return response;
				}
				catch(Exception ex)
				{
					return "Cannot echo!  " + ex.Message;
				}
			});

			Echo.ToProperty (this, vm => vm.EchoResponse, out _EchoResponse);
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


	}
}

