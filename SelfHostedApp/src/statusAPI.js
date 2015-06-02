var baseApiAddress = './api/';
var epoch = new Date('1 January 1970 00:00:00 UTC');

var statusAPI = (function() {

  function getStatus() {
    var newPromise = get(baseApiAddress + 'status', 'arraybuffer')
      .then(function(response) {
        return parseStatusReponse(response);
      }).catch(function(error) {
        alert(error);
      });

    return newPromise;
  }


  return {
    getStatus: getStatus
  };
})();


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

  status.activeProfileName = readUTF8String(response, aryOffset, 64);
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

  status.equipmentProfileName = readUTF8String(response, aryOffset, 64);
  //aryOffset += 64;

  return status;
}

function readUTF8String(responseData, offset, length) {
  var dataAry = new Int8Array(responseData, offset, length);
  var retString = "";
  for (var j = 0; j < dataAry.length; j++) {
    if (dataAry[j] === 0) {
      break;
    } else {
      retString += String.fromCharCode(dataAry[j]);
    }
  }
  return retString;
}

function CelciusToFahrenheit(degreesC) {
  return degreesC * (9 / 5) + 32.0;
}

function FahernheitToCelcius(degreesF) {
  return (degreesF - 32.0) * (5 / 9);
}
