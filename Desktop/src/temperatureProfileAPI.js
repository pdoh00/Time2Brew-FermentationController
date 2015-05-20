//Returns profile names as string[]
function getAllProfiles() {
  var newPromise = get(baseApiAddress + 'profile', 'text')
    .then(function(response) {

      //CR/LF is delimeter
      var profileNames = response.split("\r\n")
        .filter(function(arg) {
          return arg !== "";
        });

      return profileNames;
    });

  return newPromise;
}

function createProfile(profile) {

}

function deleteProfile(profileName) {
  var url = baseAddress + 'deleteprofile?name=' + profileName;
  return put(url); //a promise
}

function terminateProfile() {
  var url = baseAddress + 'terminateprofile';
  return put(url); //a promise
}

function truncateProfile(profileSteps) {

}

function executeProfile(profileName) {
  var url = baseAddress + 'executeprofile?name=' + profileName;
  return put(url); //a promise
}

//Returns Date[]. One date for each instance
function getProfileInstances(profileName) {
  var url = baseApiAddress + 'runhistory?name=' + profileName;
  var newPromise = get(url, 'text')
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
    });

  return newPromise;
}

function deleteProfileInstance(profileName, profileInstanceDate) {
  var url = baseAddress + 'deleteinstance?name=' + profileName + '&instance=' + profileInstanceDate;
  return put(url); //a promise
}

//profileInstance is num seconds since epoch
function getTrendData(profileName, profileInstance) {
  var url = baseApiAddress + 'temperaturetrend?name=' +
    profileName + '&instance=' + profileInstance;

  var newPromise = get(url, 'arraybuffer').then(function(response) {
    var trendData = parseTrendData(response);
    return trendData;
  });

  return newPromise;
}

function parseTrendData(response) {
  var dv = new DataView(response);
  var aryOffset = 0;

  var trendData = {};
  trendData.records = [];

  for (var i = 0; i < response.byteLength / 8; i++) {
    var probe0Temp = dv.getInt16(aryOffset, true);
    aryOffset += 2;

    var probe1Temp = dv.getInt16(aryOffset, true);
    aryOffset += 2;

    var setpointTemp = dv.getInt16(aryOffset, true);
    aryOffset += 2;

    var outputPercent = dv.getInt8(aryOffset, true);
    outputPercent = outputPercent >= 0 && outputPercent <= 100 ? outputPercent * 0.01 : (outputPercent - 256) * 0.01;
    aryOffset += 1;

    var dummy = dv.getInt8(aryOffset, true);
    aryOffset += 1;

    var trendRecord = {
      probe0Temp: probe0Temp,
      probe1Temp: probe1Temp,
      setpointTemp: setpointTemp,
      outputPercent: outputPercent
    };

    console.log('Probe0: ' + Math.round(CelciusToFahrenheit(probe0Temp / 10.0) * 100) / 100);
    console.log('Probe1: ' + Math.round(CelciusToFahrenheit(probe1Temp / 10.0) * 100) / 100);
    console.log('SetPoint temp: ' + Math.round(CelciusToFahrenheit(setpointTemp / 10.0) * 100) / 100);


    trendData.records.push(trendRecord);
  }
  return trendData;
}
