/** 

macstatsJS

Description: Node script for displaying you mac stats

Author: Jan Kuri <jkuri88@gmail.com>
Licence: MIT
*/

var cpu = require('./lib/cpu.js').cpu;
	battery = require('./lib/battery.js').battery;

battery.display();