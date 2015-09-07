# macstatsJS
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

## Insallation

```shell
npm i macstatsjs
cd macstatsjs
node-gyp configure
node-gyp build
```