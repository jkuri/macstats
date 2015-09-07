var battery = {},
    exec = require('child_process').exec,
    Q = require('q');

battery.exec = Q.denodeify(exec);
battery.data = {};

battery.readData = function () {
	var self = this;

	return self.exec('ioreg -rn AppleSmartBattery');
};

battery.parseData = function (data) {
	var self = this,
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

battery.calculate = function () {
	var self = this;

	self.data.percentage = ((parseFloat(self.data.max_capacity) / parseFloat(self.data.design_capacity)) * 100).toFixed(0);
	self.data.charged = ((parseFloat(self.data.current_capacity) / parseFloat(self.data.max_capacity)) * 100).toFixed(0);
	self.data.cycle_percentage = ((parseFloat(self.data.cycle_count) / parseFloat(self.data.design_cycle_count)) * 100).toFixed(0);
	self.data.time_remaining_hours = Math.floor(self.data.time_remaining / 60);
	self.data.time_remaining_minutes = self.data.time_remaining % 60;
	self.data.temparature = parseFloat(self.data.temparature / 100.0).toFixed(2);
};

battery.display = function () {
	var self = this;

	self.readData().then(function(resp) {
		self.parseData(resp);
		self.calculate();
		console.log('\n--- Battery Stats ---');
		if (self.data.battery_installed) {
			console.log('Charged:         ' + self.data.charged + '%');
			console.log('Capacity:        ' + self.data.percentage + '%');
			console.log('Cycle Count:     ' + self.data.cycle_count + ' (' + self.data.cycle_percentage + '%)');
			console.log('Max Cycle Count: ' + self.data.design_cycle_count);
			console.log('Current Charge:  ' + self.data.current_capacity + ' mAh');
			console.log('Maximum Charge:  ' + self.data.max_capacity + ' mAh');
			console.log('Design Capacity: ' + self.data.design_capacity + ' mAh');
			console.log('Time Remaining:  ' + self.data.time_remaining_hours + '.' + self.data.time_remaining_minutes + ' h');
			console.log('Temperature:     ' + self.data.temparature + String.fromCharCode(0x00B0) + 'C');
		} else {
			console.log('Battery not installed.');
		}
	});

};

exports.battery = battery;
