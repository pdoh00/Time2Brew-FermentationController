
var utils = (function() {

  var celciusToFahrenheit = function(degreesC) {
    return degreesC * (9 / 5) + 32.0;
  };

  var fahrenheitToCelcius = function(degreesF) {
    return (degreesF - 32.0) * (5 / 9);
  };

  var formatTime = function(seconds) {
    var days = 0;
    var hours = 0;
    var minutes = 0;
    var remainingSeconds = seconds;
    var returnString = "";

    if (remainingSeconds >= 86400) {
      returnString += Math.floor(remainingSeconds / 86400) + " days ";
      remainingSeconds = remainingSeconds % 86400;
    }
    if (remainingSeconds >= 3600) {
      returnString += Math.floor(remainingSeconds / 3600) + " hours ";
      remainingSeconds = remainingSeconds % 3600;
    }
    if (remainingSeconds >= 60) {
      returnString += minutes = Math.floor(remainingSeconds / 60) + " min ";
      remainingSeconds = remainingSeconds % 60;
    }
    if (remainingSeconds > 0){
      returnString += remainingSeconds + " sec";
    }

    return returnString;
  };

  var readUTF8String = function(responseData, offset, length) {
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
  };

  return {
    celciusToFahrenheit: celciusToFahrenheit,
    fahrenheitToCelcius: fahrenheitToCelcius,
    formatTime: formatTime,
    readUTF8String: readUTF8String
  };

}(utils));
