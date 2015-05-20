var Chart = require('./bower_components/chartjs/Chart.js');

function getChart() {
  var request = new XMLHttpRequest();
  request.open('GET', 'http://127.0.0.1:1337', true);

  request.onload = function() {
    if (request.status >= 200 && request.status < 400) {

      var ctx = myChart.getContext("2d");
      var options = {responsive: true};
      var myLineChart = new Chart(ctx).Line(JSON.parse(request.responseText), options);

    } else {
      alert("ERROR");
    }
  };

  request.onerror = function() {
    alert("CONNECTION ERROR");
  };

  request.send();
}
