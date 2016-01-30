'use strict';

const battery = require('./battery');
const cpu     = require('./cpu');
const fan     = require('./fan');

module.exports = {
  battery : battery.getData(),
  cpu     : cpu.getData(),
  fan     : fan.getData()
};