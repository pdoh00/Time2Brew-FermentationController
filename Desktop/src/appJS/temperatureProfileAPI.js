var rx = require('../bower_components/rxjs/dist/rx.lite.js');

var temperatureProfileAPI = (function(promiseAPI, baseAPIAddress, utils) {

  //Returns profile names as string[]
  var getAllProfiles = function() {
    return Rx.Observable.fromPromise(
      promiseAPI.get(baseAPIAddress + 'profile', 'text')
      .then(function(response) {

        //CR/LF is delimeter
        var profiles = response.split("\r\n")
          .filter(function(arg) {
            return arg !== "";
          }).map(function(profileName) {
            var profile = {
              name: profileName
            };
            return profile;
          });

        return profiles;
      }));
  };

  /*
    getInstanceSteps
    Gets the profile steps as configured at instance runtime
    Returns array of step: {startTemp, endTemp, duration}
  */
  var getInstanceSteps = function(profileName, instanceId) {
    var url = baseAPIAddress + 'profile?name=' + profileName +
      '&instance=' + instanceId;

    return Rx.Observable.fromPromise(
      promiseAPI.get(url, 'arraybuffer')
      .then(function(response) {
        return parseProfileSteps(response);
      }));
  };

  /*
  getProfileSteps
    Gets the currently configured profile steps
    Returns array of step: {startTemp, endTemp, duration}
  */
  var getProfileSteps = function(profileName) {
    var url = baseAPIAddress + 'profile?name=' + profileName;

    return Rx.Observable.fromPromise(
      promiseAPI.get(url, 'arraybuffer')
      .then(function(response) {
        return parseProfileSteps(response);
      }));
  };

  var createProfile = function(profileName, steps) {
    var fileOffset = 0;

    var bufferSize = 8 * steps.length; //8 bytes per step
    var contentBuffer = new ArrayBuffer(bufferSize);
    var dataview = new DataView(contentBuffer);

    for (var i = 0; i < steps.length; i++) {
      var offset = 8 * i;
      dataview.setInt16(offset, steps[i].startTemp, true);
      dataview.setInt16(offset + 2, steps[i].endTemp, true);
      dataview.setInt32(offset + 4, steps[i].duration, true);
    }

    //now read 512 bytes at a time and send them.
    var base64String = btoa(String.fromCharCode.apply(null, new Uint8Array(contentBuffer)));
    var chksum = fletcherChecksum(sendData, 0, length);

    var url = baseAddress + '/api/profile?name=' + profileName + '&offset=' +
      fileOffset + '&chksum=' + chksum + '&content=' + base64String;

    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  var deleteProfile = function(profileName) {
    var url = baseAddress + 'deleteprofile?name=' + profileName;
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  var terminateProfile = function() {
    var url = baseAddress + 'terminateprofile';
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  var truncateProfile = function(profileSteps) {

  };

  var executeProfile = function(profileName) {
    var url = baseAddress + 'executeprofile?name=' + profileName;
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  //Returns Date[]. One date for each instance
  var getProfileInstances = function(profileName) {
    var url = baseAPIAddress + 'runhistory?name=' + profileName;
    return Rx.Observable.fromPromise(
      promiseAPI.get(url, 'text')
      .then(function(response) {
        //CR/LF is delimeter
        //seconds from epoch in hex

        var instances = response.split("\r\n")
          .filter(function(arg) {
            return arg !== "";
          }).map(function(secondsFromEpoch) {
            //var convertedFromHex = parseInt("0x" + secondsFromEpoch);
            //return new Date(convertedFromHex * 1000);
            return new Date(secondsFromEpoch * 1000);
          });

        return instances;
      }));
  };

  var deleteProfileInstance = function(profileName, profileInstanceDate) {
    var url = baseAddress + 'deleteinstance?name=' + profileName + '&instance=' + profileInstanceDate;
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  //profileInstance is num seconds since epoch
  var getTrendData = function(profileName, profileInstance) {
    var url = baseAPIAddress + 'temperaturetrend?name=' +
      profileName + '&instance=' + profileInstance + '&reloadCacheHack=' + new Date().getTime();

    return Rx.Observable.fromPromise(
      promiseAPI.get(url, 'arraybuffer').then(function(response) {
        var trendData = parseTrendData(response);
        return trendData;
      }));
  };

  /*
    Parses a binary response into an array of step objects

    step: {startTemp, endTemp, duration}

    startTemp: degrees F
    endTemp: degrees F
    duration: seconds
  */
  function parseProfileSteps(response){
    var steps = [];

    var dv = new DataView(response);
    var aryOffset = 0;

    for (var i = 0; i < response.byteLength / 8; i++) {
      var step = {};

      var startInCelcius = dv.getInt16(aryOffset, true);
      step.startTemp = Math.round(utils.celciusToFahrenheit(startInCelcius / 10.0) * 100) / 100;
      aryOffset += 2;

      var endInCelcius = dv.getInt16(aryOffset, true);
      step.endTemp = Math.round(utils.celciusToFahrenheit(endInCelcius / 10.0) * 100) / 100;
      aryOffset += 2;

      step.duration = dv.getInt32(aryOffset, true);
      aryOffset += 4;

      steps.push(step);
    }
    return steps;
  }

  /*
    Parses a binary response into an array of trendData objects

    trendData: {probe0Temp, probe1Temp, setpointTemp, outputPercent}

    probe0Temp: degrees F
    probe1Temp: degrees F
    setpointTemp: degrees F
    outputPercent: percent
  */
  function parseTrendData(response) {
    var dv = new DataView(response);
    var aryOffset = 0;

    var trendData = [];

    for (var i = 0; i < response.byteLength / 8; i++) {
      var record = {};
      record.probe0Temp = Math.round(utils.celciusToFahrenheit(dv.getInt16(aryOffset, true) / 10.0) * 100) / 100;
      aryOffset += 2;

      record.probe1Temp = Math.round(utils.celciusToFahrenheit(dv.getInt16(aryOffset, true) / 10.0) * 100) / 100;
      aryOffset += 2;

      record.setpointTemp = Math.round(utils.celciusToFahrenheit(dv.getInt16(aryOffset, true) / 10.0) * 100) / 100;
      aryOffset += 2;

      var tempOutputPercent = dv.getInt8(aryOffset, true);
      record.outputPercent = tempOutputPercent >= 0 && tempOutputPercent <= 100 ?
        tempOutputPercent * 0.01 :
        (tempOutputPercent - 256) * 0.01;
      aryOffset += 1;

      //var dummy = dv.getInt8(aryOffset, true);
      aryOffset += 1;

      trendData.push(record);
    }
    return trendData;
  }

  return {
    getAllProfiles: getAllProfiles,
    getProfileSteps: getProfileSteps,
    getInstanceSteps: getInstanceSteps,
    createProfile: createProfile,
    deleteProfile: deleteProfile,
    terminateProfile: terminateProfile,
    truncateProfile: truncateProfile,
    executeProfile: executeProfile,
    getProfileInstances: getProfileInstances,
    deleteProfileInstance: deleteProfileInstance,
    getTrendData: getTrendData
  };

}(promiseAPI, baseAPIAddress, utils));
