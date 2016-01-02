'use strict';

const Q       = require('q');
const battery = require('./battery');
const cpu     = require('./cpu');
const fan     = require('./fan');

const api = () => {
  let deferred = Q.defer();

  battery.getData().then((batteryData) => {
    let cpuData = cpu.getData();
    let fanData = fan.getData();

    deferred.resolve({
      battery: batteryData,
      cpu: cpuData,
      fan: fanData
    });

  });

  return deferred.promise;
};

module.exports = api;