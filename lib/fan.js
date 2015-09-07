var fan = {},
    exec = require('child_process').exec,
    Q = require('q'),
    smc = require('../build/Release/smc.node');

fan.fans = smc.fans();


fan.display = function () {
	var self = this;

	console.log('\n--- Fans Stats ---');
	for (var i = 0; i < self.fans; i += 1) {
		console.log('Fan ' + i + ' speed:     ' + smc.fanRpm(i) + ' RPM');
	}
};

exports.fan = fan;