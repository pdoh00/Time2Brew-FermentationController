var app = require('app');
var BrowswerWindow = require('browser-window');

// Report crashes to our server.
require('crash-reporter').start();

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the javascript object is GCed.
var mainWindow = null;

// Quit when all windows are closed.
app.on('window-all-closed', function() {
  if (process.platform != 'darwin')
    app.quit();
});

app.on('ready', function(){
  mainWindow = new BrowswerWindow({
    width: 1400,
    height: 800
  });

  mainWindow.loadUrl('file://' + __dirname + '/index.html');

  mainWindow.on('closed', function(){
    mainWindow = null;
  });
});

var devToolsOpen = false;
function toggleDevTools(){
  var focusedWindow = BrowswerWindow.getFocusedWindow();
  if (devToolsOpen)
  {
    focusedWindow.closeDevTools();
  }else{
    focusedWindow.openDevTools({
      detach: false
    });
  }

}
