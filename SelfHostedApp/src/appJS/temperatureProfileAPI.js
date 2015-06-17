var temperatureProfileAPI = (function (promiseAPI, baseAPIAddress, utils) {

    //Returns profile names as string[]
    var getAllProfiles = function () {
        return Rx.Observable.fromPromise(
          promiseAPI.get(baseAPIAddress + 'profile', 'text')
          .then(function (response) {

              //CR/LF is delimeter
              var profiles = response.split("\r\n")
                .filter(function (arg) {
                    return arg !== "";
                }).map(function (profileName) {
                    var segmented = profileName.split(",")
                    var profile = {
                        id: segmented[0],
                        name: segmented[1]
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
    var getInstanceSteps = function (profileID, instanceId) {
        var url = baseAPIAddress + 'instancesteps?id=' + profileID +
          '&instance=' + instanceId;

        return Rx.Observable.fromPromise(
          promiseAPI.get(url, 'arraybuffer')
          .then(function (response) {
              return parseProfileInstanceSteps(response);
          }));
    };

    /*
    getProfileSteps
      Gets the currently configured profile steps
      Returns array of step: {startTemp, endTemp, duration}
    */
    var getProfileSteps = function (profileID) {
        var url = baseAPIAddress + 'profile?id=' + profileID;

        return Rx.Observable.fromPromise(
          promiseAPI.get(url, 'arraybuffer')
          .then(function (response) {
              return parseProfileSteps(response);
          }));
    };

    var createProfile = function (profileName, steps) {
        var fileOffset = 0;

        var bufferSize = 12 * steps.length; //12 bytes per step
        var contentBuffer = new ArrayBuffer(bufferSize);
        var dataview = new DataView(contentBuffer);

        for (var i = 0; i < steps.length; i++) {
            var offset = 12 * i;
            dataview.setFloat32(offset, steps[i].startTemp, true);
            dataview.setFloat32(offset + 2, steps[i].endTemp, true);
            dataview.setInt32(offset + 4, steps[i].duration, true);
        }

        //now read 512 bytes at a time and send them.
        var base64String = btoa(String.fromCharCode.apply(null, new Uint8Array(contentBuffer)));
        var chksum = fletcherChecksum(sendData, 0, length);

        var url = baseAddress + '/api/profile?name=' + profileName + '&offset=' +
          fileOffset + '&chksum=' + chksum + '&content=' + base64String;

        return Rx.Observable.fromPromise(promiseAPI.put(url));
    };

    var deleteProfile = function (profileID) {
        var url = baseAddress + 'deleteprofile?id=' + profileID;
        return Rx.Observable.fromPromise(promiseAPI.put(url));
    };

    var terminateProfile = function () {
        var url = baseAddress + 'terminateprofile';
        return Rx.Observable.fromPromise(promiseAPI.put(url));
    };

    var truncateProfile = function (profileSteps) {

    };

    var executeProfile = function (profileID) {
        var url = baseAddress + 'executeprofile?id=' + profileID;
        return Rx.Observable.fromPromise(promiseAPI.put(url));
    };

    //Returns Date[]. One date for each instance
    var getProfileInstances = function (profileID) {
        var url = baseAPIAddress + 'instancesteps?id=' + profileID;
        return Rx.Observable.fromPromise(
          promiseAPI.get(url, 'text')
          .then(function (response) {
              //CR/LF is delimeter
              //seconds from epoch in hex

              var instances = response.split("\r\n")
                .filter(function (arg) {
                    return arg !== "";
                }).map(function (secondsFromEpoch) {
                    //var convertedFromHex = parseInt("0x" + secondsFromEpoch);
                    //return new Date(convertedFromHex * 1000);
                    return new Date(secondsFromEpoch * 1000);
                });

              return instances;
          }));
    };

    var deleteProfileInstance = function (profileID, profileInstanceDate) {
        var url = baseAddress + 'deleteinstance?id=' + profileID + '&instance=' + profileInstanceDate;
        return Rx.Observable.fromPromise(promiseAPI.put(url));
    };

    //profileInstance is num seconds since epoch
    var getTrendData = function (profileID, profileInstance) {
        var url = baseAPIAddress + 'instancetrend?id=' +
          profileID + '&instance=' + profileInstance + '&reloadCacheHack=' + new Date().getTime();

        return Rx.Observable.fromPromise(
          promiseAPI.get(url, 'arraybuffer').then(function (response) {
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
    function parseProfileSteps(response) {
        var steps = [];

        var dv = new DataView(response);
        var aryOffset = 64;

        for (var i = 0; i < (response.byteLength - 64) / 12; i++) {
            var step = {};

            var startInCelcius = dv.getFloat32(aryOffset, true);
            step.startTemp = Math.round(utils.celciusToFahrenheit(startInCelcius) * 100) / 100;
            aryOffset += 4;

            var endInCelcius = dv.getFloat32(aryOffset, true);
            step.endTemp = Math.round(utils.celciusToFahrenheit(endInCelcius) * 100) / 100;
            aryOffset += 4;

            step.duration = dv.getInt32(aryOffset, true);
            aryOffset += 4;

            steps.push(step);
        }
        return steps;
    }

    /*
      Parses a binary response into an array of step objects
  
      step: {startTemp, endTemp, duration}
  
      startTemp: degrees F
      endTemp: degrees F
      duration: seconds
    */
    function parseProfileInstanceSteps(response) {
        var steps = [];

        var dv = new DataView(response);
        var aryOffset = 0;

        var blen = response.byteLength;

        for (var i = 0; i < response.byteLength / 12; i++) {
            var step = {};

            var startInCelcius = dv.getFloat32(aryOffset, true);
            step.startTemp = Math.round(utils.celciusToFahrenheit(startInCelcius) * 100) / 100;
            aryOffset += 4;

            var endInCelcius = dv.getFloat32(aryOffset, true);
            step.endTemp = Math.round(utils.celciusToFahrenheit(endInCelcius) * 100) / 100;
            aryOffset += 4;

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
        var sampleIdx = 0;

        var trendData = [];

        var probe0Temp_C;
        var probe1Temp_C;
        var Packed;

        for (var i = 0; i < response.byteLength / 7; i++) {
            var record = {};
            Packed = dv.getUint32(aryOffset, true);
            aryOffset += 4;
           

            if (Packed != 0xFFFFFFFF) {
                var RelayStatus = Packed & 3;
                Packed >>= 2;
                var OutputPercent = Packed & 1023;
                Packed >>= 10;
                var p1_temp = Packed & 1023;
                Packed >>= 10;
                var p0_temp = Packed & 1023;

                probe0Temp_C = (p0_temp * 0.146628) - 25;
                probe1Temp_C = (p1_temp * 0.146628) - 25;
                OutputPercent -= 100;

                record.probe0Temp = Math.round(utils.celciusToFahrenheit(probe0Temp_C) * 100) / 100;
                record.probe1Temp = Math.round(utils.celciusToFahrenheit(probe1Temp_C) * 100) / 100;
                record.outputPercent = OutputPercent;
                record.RelayStatus = RelayStatus;
            } else {
                record.probe0Temp = 0;
                record.probe1Temp = 0;
                record.outputPercent = 0;
                record.RelayStatus = 0;
            }          
            sampleIdx += 1;
            record.SampleIdx = sampleIdx;
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
