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

		[Get("/profile?{id}")]
		Task<string> GetProfile(string id);

		[Put("/profile")]
		Task Profile([Body]string payload);

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

