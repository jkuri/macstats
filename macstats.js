#! /usr/bin/env node

//	macstats
//	Description: Node script for displaying you mac stats
//	Author: Jan Kuri <jkuri88@gmail.com>
//	Licence: MIT

const cpu      = require('./lib/cpu');
const fan      = require('./lib/fan');
const battery  = require('./lib/battery');

cpu.display();
fan.display();
battery.display();
