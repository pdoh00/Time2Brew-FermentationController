using System;
using Refit;
using System.Threading.Tasks;

namespace FermentationController
{
	public interface IFermentationControllerAPI
	{

		//get api/profile
		[Get("/ECHO?{data}")]
		Task<string> Echo(string data);

		//get api/profile
		[Get("/profile")]
		Task<string> GetAllProfiles();

		//Response BODY=application/octet-stream (*See Profile Data Structure)
		[Get("/profile?name={profileName}")]
		Task<string> GetProfile(string profileName);

//		*offset if not provided defaults to zero (0).  Used in case of profiles longer than 512 bytes.
//			Request BODY=application/octet-stream (*See Profile Data Structure)
				
		[Put("/profile?name={profileName}&offset={offset}")]
		Task Profile(string profileName, [Body]string payload, int offset = 0);

		[Put("/executeProfile?name={profileName}")]
		Task ExecuteProfile (string profileName, int offset = 0);

		[Put("/terminateProfile?name={profileName}")]
		Task TerminateProfile (string profileName, int offset = 0);

		//  Request BODY=application/octet-stream (*See TRUNCATION BINARY DATA STRUCTURE)
		[Put("/truncateProfile")]
		Task TruncateProfile();

		[Get("/runhistory?name={profileName}")]
		Task GetRunHistory (string profileName);

		//*Note: Start and either End OR Length is required.\n  Response BODY=application/octet-stream (*See Temperature Trend Data Structure)\n
		//&length={length (s)}
		[Get("/temperatureTrend?start={startSecondsFromEpoch}&end={endSecondsFromEpoch}")]
		Task GetTemperatureTrend (long startSecondsFromEpoch, long endSecondsFromEpoch);

//		Response BODY=text/plain
//		----------------------
//		{Probe 0 Temperature in C}\r\n
//		{Probe 1 Temperature in C}\r\n	
		[Get("/temperature")]
		Task GetTemperature();

		//probeId = 0,1
		[Get("/temperature?probe={probeId}")]
		Task GetTemperatureForProbe (int probeId);

//		get api/profile?{id}
//		get api/profile/log?{profileId}
//		put api/profile
//		application/text
//		Name (maxLength = 64)
//		Mode {SimpleThreshold, SimplePID, ComplexThreshold, ComplexPID},{Probe0Status},{Probe1Status}
//		time(ms), temp(C)
//			...
//			time(ms), temp(C)
//
//			put api/profile/execute?{id}
//
//		get api/temperature? 0 || 1 || none for both
//			put api/temperature?{temp(C)},{SimpleThreshold, SimplePID, ComplexThreshold, ComplexPID},{Probe0Status},{Probe1Status}
//
//		get api/status
//		application/text
//		----------------------
//		Mode:{SimpleThreshold, SimplePID, ComplexThreshold, ComplexPID}
//		Probe 0: {Temp(C)}
//		Probe 0 Status: {Process, Target, Sensor}
//		Probe 1: {Temp(C)}
//		Probe 1 Status: {Process, Target, Sensor}
//		RelayStatusHeat: {On/Off}
//		RelayStatusCool: {On/Off}
//		Running Profile: {id}
//		Profile Step Idx: {0 to Count -1}
//		Profile Step Time Remaining: {time(ms)}
//		Profile Step Temp: {temp(C)}
//
//
//		//For aaron
//		get api/log?{id},{timeSpan(ms)} timespan is from end of file
	}
}

