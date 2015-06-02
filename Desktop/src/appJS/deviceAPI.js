var rx = require('../bower_components/rxjs/dist/rx.lite.js');
var deviceAPI = (function(promiseAPI) {

  var setCredentials = function(userName, password) {
    var url = baseAddress + 'updatecredentials?username=' + username + '&password=' + password;
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  var setTime = function(time) {
    var totalSecondsFromEpoch = Math.round(time.getTime() / 1000);
    var url = baseAddress + '/api/time?time=' + totalSecondsFromEpoch;
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  var trimFileSystem = function() {
    var url = baseAddress + 'trimfilesystem';
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  var updateFirmware = function(firmwareFileStream) {
    //TODO: can I do require('fs') here?
    //might not be able to from browser
  };

  var getFirmwareVersion = function() {
    var url = baseAddress + 'version';
    return Rx.Observable.fromPromise(
      promiseAPI.get(url, 'text').then(function(response) {
        return response;
      }));
  };

  var reboot = function(userName, password) {
    var url = baseAddress + 'restart?confirm=restart';
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  var resetToFactoryDefault = function(userName, password) {
    var url = baseAddress + 'format?confirm=format';
    return Rx.Observable.fromPromise(promiseAPI.put(url));
  };

  return {
    setCredentials: setCredentials,
    setTime: setTime,
    trimFileSystem: trimFileSystem,
    updateFirmware: updateFirmware,
    getFirmwareVersion: getFirmwareVersion,
    reboot: reboot,
    resetToFactoryDefault: resetToFactoryDefault
  };
})(promiseAPI);
