function get(url, responseType) {
  return new Promise(function(resolve, reject) {
    var request = new XMLHttpRequest();
    request.open('GET', url);
    request.responseType = responseType;

    request.onload = function() {
      if (request.status == 200) {
        resolve(request.response);
      } else {
        reject(Error(request.statusText));
      }
    };

    request.onerror = function() {
      reject(Error("Network Error"));
    };

    request.send();
  });
}

function put(url){
  return new Promise(function(resolve, reject){
    var request = new XMLHttpRequest();
    request.open('PUT', url);
    // request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8');
    request.onload = function() {
      if (request.status == 200) {
        resolve(request.response);
      } else {
        reject(Error(request.statusText));
      }
    };

    request.onerror = function() {
      reject(Error("Network Error"));
    };

    request.send();
  });
}
