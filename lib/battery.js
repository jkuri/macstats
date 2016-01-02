'use strict';

const battery 	= {},
    	exec 			= require('child_process').exec,
    	Q 				= require('q'),
    	chalk 		= require('chalk');

battery.exec = Q.denodeify(exec);
battery.data = {};

battery.readData = function () {
	let self = this;
	return self.exec('ioreg -rn AppleSmartBattery');
};

battery.parseData = function (data) {
	let self = this,
	lines = data.toString().split('\n');

	lines.forEach(function (line) {
		line = line.trim();

		if (/"BatteryInstalled"/.test(line)) {
			self.data.battery_installed = line.split('=')[1].trim() === 'Yes' ? true : false;
		}

		if (/"CycleCount"/.test(line)) {
			self.data.cycle_count = parseInt(line.split('=')[1].trim(), 10);
		}

		if (/"DesignCapacity"/.test(line)) {
			self.data.design_capacity = parseInt(line.split('=')[1].trim(), 10);
		}

		if (/"MaxCapacity"/.test(line)) {
			self.data.max_capacity = parseInt(line.split('=')[1].trim(), 10);
		}

		if (/"CurrentCapacity"/.test(line)) {
			self.data.current_capacity = parseInt(line.split('=')[1].trim(), 10);
		}

		if (/"DesignCycleCount9C"/.test(line)) {
			self.data.design_cycle_count = parseInt(line.split('=')[1].trim(), 10);
		}

		if (/"TimeRemaining"/.test(line)) {
			self.data.time_remaining = parseInt(line.split('=')[1].trim(), 10);
		}

		if (/"Temperature"/.test(line)) {
			self.data.temparature = parseInt(line.split('=')[1].trim(), 10);
		}

	});
};

battery.getData = function () {
	let self = this;
	let deferred = Q.defer();

	self.readData().then(function(resp) {
		self.parseData(resp);
		self.calculate();
		deferred.resolve(self.data);
	});

	return deferred.promise;
};

battery.calculate = function () {
	let self = this;

	self.data.percentage = ((parseFloat(self.data.max_capacity) / parseFloat(self.data.design_capacity)) * 100).toFixed(0);
	self.data.charged = ((parseFloat(self.data.current_capacity) / parseFloat(self.data.max_capacity)) * 100).toFixed(0);
	self.data.cycle_percentage = ((parseFloat(self.data.cycle_count) / parseFloat(self.data.design_cycle_count)) * 100).toFixed(0);
	self.data.time_remaining_hours = Math.floor(self.data.time_remaining / 60);
	self.data.time_remaining_minutes = self.data.time_remaining % 60;
	self.data.temparature = parseFloat(self.data.temparature / 100.0).toFixed(2);
};

battery.display = function () {
	let self = this;

	self.readData().then(function(resp) {
		self.parseData(resp);
		self.calculate();
		console.log(chalk.yellow('\n--- Battery Stats ---'));
		if (self.data.battery_installed) {
			console.log('Charged:         ' + chalk.green(self.data.charged + '%'));
			console.log('Capacity:        ' + chalk.green(self.data.percentage + '%'));
			console.log('Cycle Count:     ' + chalk.green(self.data.cycle_count + ' (' + self.data.cycle_percentage + '%)'));
			console.log('Max Cycle Count: ' + chalk.green(self.data.design_cycle_count));
			console.log('Current Charge:  ' + chalk.green(self.data.current_capacity + ' mAh'));
			console.log('Maximum Charge:  ' + chalk.green(self.data.max_capacity + ' mAh'));
			console.log('Design Capacity: ' + chalk.green(self.data.design_capacity + ' mAh'));
			console.log('Time Remaining:  ' + chalk.green(self.data.time_remaining_hours + '.' + self.data.time_remaining_minutes + ' h'));
			console.log('Temperature:     ' + chalk.green(self.data.temparature + String.fromCharCode(0x00B0) + 'C'));
		} else {
			console.log(chalk.red('Battery not installed.'));
		}
	});

};

module.exports = battery;
