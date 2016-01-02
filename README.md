# macstats
node script for you mac stats

## Example output:

```shell
--- CPU Stats ---
CPU Temp:        33.75°C

--- Fans Stats ---
Fan 0 speed:     1996 RPM
Fan 1 speed:     2003 RPM

--- Battery Stats ---
Charged:         82%
Capacity:        92%
Cycle Count:     692 (69%)
Max Cycle Count: 1000
Current Charge:  5189 mAh
Maximum Charge:  6316 mAh
Design Capacity: 6900 mAh
Time Remaining:  6.54 h
Temperature:     30.06°C
```

## Installation

```shell
npm i macstats -g
```

## Run

```shell
macstats
```

## API

Include `/lib/api` in your script if you want to use the API. API returns a promise with object of data.
Example:

````JavaScript
'use strict';

let API = require('../lib/api');

API().then((api) => {
  console.log(api.battery);
  console.log(api.cpu);
  console.log(api.fan);
});
````

## API Reference

Data                              | Description
:---------------------------------|:-------------------------------------------------------------------------------------------------------------
battery.battery_installed         | Is battery installed, true|false [Boolean]
battery.design_capacity           | Battery design capacity in mAh [Int]
battery.max_capacity              | Max battery capacity in mAh [Int]
battery.current_capacity          | Current battery capacuty in mAh [Int]
battery.percentage                | Current capacity percentage [Int]
battery.design_cycle_count        | Design cycle count [Int]
battery.cycle_count               | Current battery cycle count [Int]
battery.cycle_percentage          | Cycle count percentage [Int]
battery.temparature               | Battery temperature in °C [Float]
battery.charged                   | Current battery charge percentage [Int]
battery.time_remaining            | Remaining time in minutes, if plugged in time until full, otherwise time until empty [Int]
battery.time_remaining_hours      | Time remaining in hours. Use in a combination with *time_remaining_minutes* [Int]
battery.time_remaining_minutes    | Time remaining in minutes. Use in a combination with *time_remaining_hours* [Int]
cpu.temp                          | CPU Temperature in °C [Float]
fan.num                           | Total number of fans [Int]
fan.fans                          | Array of Objects (properties: id [Int], rpm [Int]), example: [ { id: 0, rpm: 2007 }, { id: 1, rpm: 1999 } ] }

## Author

[Jan Kuri](http://www.jankuri.com)

## Licence

This project is licensed under the MIT license. See the [LICENSE](LICENSE) file for more info.

## Thanks

Thanks [Massimiliano Marcon](https://github.com/mmarcon) for SMC C++ code in v8.
