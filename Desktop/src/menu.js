var remote = require('remote');
var BrowserWindow = remote.require('browser-window');
var app = remote.require('app');
var Menu = remote.require('menu');
var MenuItem = remote.require('menu-item');

var template = [{
  label: 'Electron',
  submenu: [{
    label: 'Toggle DevTools',
    accelerator: 'CommandOrControl+D',
    click: function() {
      BrowserWindow.getFocusedWindow().toggleDevTools();
    }
  },
  {
    label: 'Quit',
    accelerator: 'CommandOrControl+Q',
    click: function() {
      app.quit();
    }
  }]
}];
var menu = new Menu();
menu = Menu.buildFromTemplate(template);


Menu.setApplicationMenu(menu);
