var baseApiAddress = 'http://10.10.1.148/api/';
var epoch = new Date('1 January 1970 00:00:00 UTC');

function getStatus() {
  var newPromise = get(baseApiAddress + 'status', 'arraybuffer')
    .then(function(response) {
      return parseStatusReponse(response);
    }).catch(function(error) {
      alert(error);
    });

  return newPromise;
}

function parseStatusReponse(response) {
  var dv = new DataView(response);
  var aryOffset = 0;

  var systemTime = new Date(dv.getUint32(aryOffset, true) * 1000);
  aryOffset += 4;

  var systemMode = dv.getInt8(aryOffset, true);
  aryOffset += 1;

  var regulationMode = dv.getInt8(aryOffset, true);
  aryOffset += 1;

  var probe0Assign = dv.getInt8(aryOffset, true);
  aryOffset += 1;

  var probe0Temp = dv.getInt16(aryOffset, true);
  aryOffset += 2;

  var probe1Assign = dv.getInt8(aryOffset, true);
  aryOffset += 1;

  var probe1Temp = dv.getInt16(aryOffset, true);
  aryOffset += 2;

  var heatRelayOn = dv.getInt8(aryOffset, true);
  aryOffset += 1;

  var coolRelayOn = dv.getInt8(aryOffset, true);
  aryOffset += 1;

  var activeProfileName = readUTF8String(response, aryOffset, 64);
  aryOffset += 64;

  var currentStepIndex = dv.getInt16(aryOffset, true);
  aryOffset += 2;

  var currentStepTemp = dv.getInt16(aryOffset, true);
  aryOffset += 2;

  var currentStepRemainingSeconds = dv.getUint32(aryOffset, true);
  aryOffset += 4;

  var manualSetPointTemp = dv.getInt16(aryOffset, true);
  aryOffset += 2;

  var profileStartTime = new Date(dv.getUint32(aryOffset, true) * 1000);
  aryOffset += 4;

  var equipmentProfileName = readUTF8String(response, aryOffset, 64);
  //aryOffset += 64;

  return {
    systemTime: systemTime.toString(),
    systemMode: systemMode,
    regulationMode: regulationMode,
    probe0Assign: probe0Assign,
    probe0Temp: probe0Temp,
    probe1Assign: probe1Assign,
    probe1Temp: probe1Temp,
    heatRelayOn: heatRelayOn,
    coolRelayOn: coolRelayOn,
    activeProfileName: activeProfileName,
    currentStepIndex: currentStepIndex,
    currentStepTemp: currentStepTemp,
    currentStepRemainingSeconds: currentStepRemainingSeconds,
    manualSetPointTemp: manualSetPointTemp,
    profileStartTime: profileStartTime.toString(),
    equipmentProfileName: equipmentProfileName
  };
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
