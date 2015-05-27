var rx = require('./bower_components/rxjs/dist/rx.lite.js');

var baseApiAddress = 'http://10.10.1.148/api/';
var epoch = new Date('1 January 1970 00:00:00 UTC');

var temperatureProfileAPI = (function() {

  //Returns profile names as string[]
  var getAllProfiles = function() {
    return Rx.Observable.fromPromise(
      get(baseApiAddress + 'profile', 'text')
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

  var getProfileSteps = function(profileName) {
    var url = baseApiAddress + 'profile?name=' + profileName;

    return Rx.Observable.fromPromise(
      get(url, 'arraybuffer')
      .then(function(response) {
        var steps = [];

        var dv = new DataView(response);
        var aryOffset = 0;

        for (var i = 0; i < response.byteLength / 8; i++) {
          var step = {};
          var startInCelcius = dv.getInt16(aryOffset, true);
          step.startTemp = Math.round(CelciusToFahrenheit(startInCelcius / 10.0) * 100) / 100;
          aryOffset += 2;
          var endInCelcius = dv.getInt16(aryOffset, true);
          step.endTemp = Math.round(CelciusToFahrenheit(endInCelcius / 10.0) * 100) / 100;
          aryOffset += 2;
          step.duration = dv.getInt32(aryOffset);
          steps.push(step);
        }
        return steps;
      }));
  };

  var createProfile = function(profile) {

  };

  var deleteProfile = function(profileName) {
    var url = baseAddress + 'deleteprofile?name=' + profileName;
    return put(url); //a promise
  };

  var terminateProfile = function() {
    var url = baseAddress + 'terminateprofile';
    return put(url); //a promise
  };

  var truncateProfile = function(profileSteps) {

  };

  var executeProfile = function(profileName) {
    var url = baseAddress + 'executeprofile?name=' + profileName;
    return put(url); //a promise
  };

  //Returns Date[]. One date for each instance
  var getProfileInstances = function(profileName) {
    var url = baseApiAddress + 'runhistory?name=' + profileName;
    return Rx.Observable.fromPromise(
      get(url, 'text')
      .then(function(response) {
        //CR/LF is delimeter
        //seconds from epoch in hex

        var instances = response.split("\r\n")
          .filter(function(arg) {
            return arg !== "";
          }).map(function(secondsFromEpoch) {
            var convertedFromHex = parseInt("0x" + secondsFromEpoch);
            return new Date(convertedFromHex * 1000);
          });

        return instances;
      }));
  };

  var deleteProfileInstance = function(profileName, profileInstanceDate) {
    var url = baseAddress + 'deleteinstance?name=' + profileName + '&instance=' + profileInstanceDate;
    return put(url); //a promise
  };

  //profileInstance is num seconds since epoch
  var getTrendData = function(profileName, profileInstance) {
    var url = baseApiAddress + 'temperaturetrend?name=' +
      profileName + '&instance=' + profileInstance;

    var newPromise = get(url, 'arraybuffer').then(function(response) {
      var trendData = parseTrendData(response);
      return trendData;
    });

    return newPromise;
  };

  function parseTrendData(response) {
    var dv = new DataView(response);
    var aryOffset = 0;

    var trendData = {};
    trendData.records = [];

    for (var i = 0; i < response.byteLength / 8; i++) {
      var record = {};
      record.probe0Temp = Math.round(CelciusToFahrenheit(dv.getInt16(aryOffset, true) / 10.0) * 100) / 100;
      aryOffset += 2;

      record.probe1Temp = Math.round(CelciusToFahrenheit(dv.getInt16(aryOffset, true) / 10.0) * 100) / 100;
      aryOffset += 2;

      record.setpointTemp = Math.round(CelciusToFahrenheit(dv.getInt16(aryOffset, true) / 10.0) * 100) / 100;
      aryOffset += 2;

      var tempOutputPercent = dv.getInt8(aryOffset, true);
      record.outputPercent = tempOutputPercent >= 0 && tempOutputPercent <= 100 ?
        tempOutputPercent * 0.01 :
        (tempOutputPercent - 256) * 0.01;
      aryOffset += 1;

      //var dummy = dv.getInt8(aryOffset, true);
      aryOffset += 1;

      trendData.records.push(record);
    }
    return trendData;
  }

  function CelciusToFahrenheit(degreesC) {
    return degreesC * (9 / 5) + 32.0;
  }

  function FahernheitToCelcius(degreesF) {
    return (degreesF - 32.0) * (5 / 9);
  }


  return {
    getAllProfiles: getAllProfiles,
    getProfileSteps: getProfileSteps,
    createProfile: createProfile,
    deleteProfile: deleteProfile,
    terminateProfile: terminateProfile,
    truncateProfile: truncateProfile,
    executeProfile: executeProfile,
    getProfileInstances: getProfileInstances,
    deleteProfileInstance: deleteProfileInstance,
    getTrendData: getTrendData
  };

})();
