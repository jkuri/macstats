'use strict';

let API = require('../lib/api');

API().then((api) => {
  console.log(api.battery);
  console.log(api.cpu);
  console.log(api.fan);
});