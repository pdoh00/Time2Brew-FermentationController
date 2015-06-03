var rx = require('../bower_components/rxjs/dist/rx.lite.js');

var statusAPI = (function(promiseAPI, baseAPIAddress, utils) {

  function getStatus() {
    return Rx.Observable.fromPromise(
      promiseAPI.get(baseAPIAddress + 'status', 'arraybuffer')
      .then(function(response) {
        return parseStatusReponse(response);
      }));
  }

  function parseStatusReponse(response) {
    var dv = new DataView(response);
    var aryOffset = 0;
    var status = {};

    status.systemTime = new Date(dv.getUint32(aryOffset, true) * 1000);
    aryOffset += 4;

    status.systemMode = dv.getInt8(aryOffset, true);
    aryOffset += 1;

    status.regulationMode = dv.getInt8(aryOffset, true);
    aryOffset += 1;

    status.probe0Assign = dv.getInt8(aryOffset, true);
    aryOffset += 1;

    status.probe0Temp = dv.getInt16(aryOffset, true);
    aryOffset += 2;

    status.probe1Assign = dv.getInt8(aryOffset, true);
    aryOffset += 1;

    status.probe1Temp = dv.getInt16(aryOffset, true);
    aryOffset += 2;

    status.heatRelayOn = dv.getInt8(aryOffset, true);
    aryOffset += 1;

    status.coolRelayOn = dv.getInt8(aryOffset, true);
    aryOffset += 1;

    status.activeProfileName = utils.readUTF8String(response, aryOffset, 64);
    aryOffset += 64;

    status.currentStepIndex = dv.getInt16(aryOffset, true);
    aryOffset += 2;

    status.currentStepTemp = dv.getInt16(aryOffset, true);
    aryOffset += 2;

    status.currentStepRemainingSeconds = dv.getUint32(aryOffset, true);
    aryOffset += 4;

    status.manualSetPointTemp = dv.getInt16(aryOffset, true);
    aryOffset += 2;

    status.profileStartTime = new Date(dv.getUint32(aryOffset, true) * 1000);
    aryOffset += 4;

    status.equipmentProfileName = utils.readUTF8String(response, aryOffset, 64);
    //aryOffset += 64;

    return status;
  }



  return {
    getStatus: getStatus
  };
}(promiseAPI, baseAPIAddress, utils));
