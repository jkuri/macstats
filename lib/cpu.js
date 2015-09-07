var cpu = {},
    exec = require('child_process').exec,
    Q = require('q'),
    smc = require('../build/Release/smc.node');

cpu.display = function () {
	console.log('--- CPU Stats ---');
	console.log('CPU Temp:        ' + smc.temperature() + String.fromCharCode(0x00B0) + 'C');
};

exports.cpu = cpu;

