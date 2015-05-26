var Chart = require('./bower_components/chartjs/Chart.js');

// var Chartist = require('./bower_components/chartist/dist/chartist.min.css');

function getChart() {
  var request = new XMLHttpRequest();
  request.open('GET', 'http://127.0.0.1:1337', true);

  request.onload = function() {
    if (request.status >= 200 && request.status < 400) {

      var ctx = myChart.getContext("2d");
      var options = {
        responsive: true
      };
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

function loadChart(canvas, labels, datasets) {
  var ctx = canvas.getContext("2d");
  var options = {
    responsive: true,
    showXLabels: 25
  };
  var myLineChart = new Chart(ctx).Line({
    labels: labels,
    datasets: datasets
  }, options);
}

function loadChartist(data){
  // new Chartist.Line('.ct-chart', data);
}
