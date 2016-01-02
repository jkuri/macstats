'use strict';

const cpu   = {},
      exec  = require('child_process').exec,
      Q     = require('q'),
      smc   = require('../build/Release/smc.node'),
      chalk = require('chalk');

cpu.getData = function () {
  let data = {};
  data.temp = smc.temperature();
  return data;
};

cpu.display = function () {
	console.log(chalk.yellow('--- CPU Stats ---'));
	console.log('CPU Temp:        ' + chalk.green(smc.temperature() + String.fromCharCode(0x00B0) + 'C'));
};

module.exports = cpu;
