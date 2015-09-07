var cpu = {},
	exec = require('child_process').exec,
	Q = require('q');

cpu.exec = Q.denodeify(exec);
cpu.data = {};

cpu.display = function () {
	
};

exports.cpu = cpu;

