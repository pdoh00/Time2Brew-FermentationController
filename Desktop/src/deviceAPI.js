function setCredentials(userName, password) {
  var url = baseAddress + 'updatecredentials?username=' + username + '&password=' + password;
  var newPromise = put(url);
  return newPromise;
}

function setTime(time){
  var totalSecondsFromEpoch = Math.round(time.getTime() / 1000);
  var url = baseAddress + '/api/time?time=' + totalSecondsFromEpoch;
  var newPromise = put(url);
  return newPromise;
}

function trimFileSystem() {
  var url = baseAddress + 'trimfilesystem';
  return put(url);
}

function updateFirmware(firmwareFileStream) {
  //TODO: can I do require('fs') here?
}

function getFirmwareVersion() {
  var url = baseAddress + 'version';
  var newPromise = get(url, 'text').then(function(response) {
    return response;
  });
  return newPromise;
}

function reboot(userName, password) {
  var url = baseAddress + 'restart?confirm=restart';
  return put(url);
}

function resetToFactoryDefault(userName, password) {
  var url = baseAddress + 'format?confirm=format';
  return put(url); //a promise
}
