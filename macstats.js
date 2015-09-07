#! /usr/bin/env node

//	macstats
//	Description: Node script for displaying you mac stats
//	Author: Jan Kuri <jkuri88@gmail.com>
//	Licence: MIT

var cpu = require('./lib/cpu.js').cpu,
	fan = require('./lib/fan.js').fan,
	battery = require('./lib/battery.js').battery;

cpu.display();
fan.display();
battery.display();
