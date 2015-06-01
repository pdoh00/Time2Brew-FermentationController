var Chart = require('./bower_components/chartjs/Chart.js');
var $ = require('./bower_components/jquery/dist/jquery.min.js');
var flot = require('./bower_components/flot/jquery.flot.js');
var flotTime = require('./bower_components/flot/jquery.flot.time.js');

var chartAPI = (function() {

  var createChart = function(placeholder, data, options) {
    var plot = $.plot(placeholder, data, options);
  };


  return {
    createChart: createChart
  };

})();
