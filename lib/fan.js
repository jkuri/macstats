'use strict';

const fan     = {},
      exec    = require('child_process').exec,
      Q       = require('q'),
      smc     = require('../build/Release/smc.node'),
      chalk   = require('chalk');

fan.getData = () => {
  let f = {};
  f.fans = smc.fans();
  
  let data = {};
  data.num = f.fans;
  data.fans = [];
  for (let i = 0; i < f.fans; i++) {
    let tmpFan = { id: i, rpm: smc.fanRpm(i) };
    data.fans.push(tmpFan);
  }

  return data;
};

fan.display = () => {
  let f = {};
  f.fans = smc.fans();

  console.log(chalk.yellow('\n--- Fans Stats ---'));
  for (let i = 0; i < f.fans; i++) {
		console.log('Fan ' + i + ' speed:     ' + chalk.green(smc.fanRpm(i)) + chalk.green(' RPM'));
  }
};

module.exports = fan;