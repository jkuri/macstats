# macstats

Node module for macOS system statistics. Get real-time CPU, GPU, battery, memory, disk, and sensor data from your Mac.

## Table of Contents

- [Installation](#installation)
- [CLI Usage](#cli-usage)
- [API Reference](#api-reference)
  - [CPU](#cpu)
  - [GPU](#gpu)
  - [Battery](#battery)
  - [Memory & RAM](#memory--ram)
  - [Disk](#disk)
  - [Fans](#fans)
  - [Power](#power)
  - [Sensors](#sensors)
  - [System](#system)

## Example output

<img width="1738" height="1134" alt="Screenshot 2025-11-09 at 01 23 33" src="https://github.com/user-attachments/assets/93154027-646a-426f-9dcc-bfa93753ff6d" />

## Installation

```shell
npm i macstats -g
```

## CLI Usage

```shell
# Show system stats (single snapshot)
macstats

# Show all disks including system volumes
macstats --detailed
macstats -d

# Launch real-time dashboard with live updates
macstats --watch
macstats -w

# Launch real-time dashboard with all disks
macstats -w -d

# Show help
macstats --help
```

### Controls (in watch mode)

- `q` or `Ctrl+C` - Exit
- `r` - Force refresh

## API Reference

### Importing

```typescript
import {
  getCpuData,
  getGpuData,
  getBatteryData,
  getRAMUsage,
  getDiskInfo,
  getFanData,
  getPowerData,
  getSensorData,
  getSystemData
} from 'macstats';

// Or use synchronous versions
import {
  getCpuDataSync,
  getGpuDataSync,
  getBatteryDataSync,
  getRAMUsageSync,
  getDiskInfoSync,
  getFanDataSync,
  getPowerDataSync,
  getSensorDataSync,
  getSystemDataSync
} from 'macstats';
```

### CPU

#### `getCpuData()` / `getCpuDataSync()`

Get CPU temperature, die temperature, power consumption, and voltage.

**Returns:** `Promise<CPU>` / `CPU`

| Property         | Type     | Description                    |
| ---------------- | -------- | ------------------------------ |
| `temperature`    | `number` | CPU temperature in °C          |
| `temperatureDie` | `number` | CPU die temperature in °C      |
| `power`          | `number` | CPU power consumption in Watts |
| `voltage`        | `number` | CPU voltage in Volts           |

**Example:**

```typescript
const cpu = await getCpuData();
console.log(`CPU Temp: ${cpu.temperature}°C`);
console.log(`CPU Power: ${cpu.power}W`);
```

#### `getCPUUsage()` / `getCPUUsageSync()`

Get CPU usage statistics including load averages.

**Returns:** `Promise<CPUUsage>` / `CPUUsage`

| Property            | Type     | Description                             |
| ------------------- | -------- | --------------------------------------- |
| `userLoad`          | `number` | User CPU load (0.0 - 1.0)               |
| `systemLoad`        | `number` | System CPU load (0.0 - 1.0)             |
| `idleLoad`          | `number` | Idle CPU load (0.0 - 1.0)               |
| `totalUsage`        | `number` | Total CPU usage (0.0 - 1.0)             |
| `totalUsagePercent` | `number` | Total CPU usage as percentage (0 - 100) |
| `loadAvg1`          | `number` | 1 minute load average                   |
| `loadAvg5`          | `number` | 5 minute load average                   |
| `loadAvg15`         | `number` | 15 minute load average                  |

**Example:**

```typescript
const usage = await getCPUUsage();
console.log(`CPU Usage: ${usage.totalUsagePercent}%`);
console.log(`Load Average (1m): ${usage.loadAvg1}`);
```

### GPU

#### `getGpuData()` / `getGpuDataSync()`

Get GPU temperature, power consumption, and voltage.

**Returns:** `Promise<GPU>` / `GPU`

| Property      | Type     | Description                    |
| ------------- | -------- | ------------------------------ |
| `temperature` | `number` | GPU temperature in °C          |
| `power`       | `number` | GPU power consumption in Watts |
| `voltage`     | `number` | GPU voltage in Volts           |

**Example:**

```typescript
const gpu = await getGpuData();
console.log(`GPU Temp: ${gpu.temperature}°C`);
console.log(`GPU Power: ${gpu.power}W`);
```

### Battery

#### `getBatteryData()` / `getBatteryDataSync()`

Get comprehensive battery information including charge, health, and time remaining.

**Returns:** `Promise<Battery>` / `Battery`

| Property                   | Type      | Description                                |
| -------------------------- | --------- | ------------------------------------------ |
| `external_connected`       | `boolean` | Is external power connected                |
| `battery_installed`        | `boolean` | Is battery installed                       |
| `is_charging`              | `boolean` | Is battery currently charging              |
| `fully_charged`            | `boolean` | Is battery fully charged                   |
| `voltage`                  | `number`  | Battery voltage in mV                      |
| `cycle_count`              | `number`  | Number of charge cycles                    |
| `design_capacity`          | `number`  | Design capacity in mAh                     |
| `max_capacity`             | `number`  | Maximum capacity in mAh                    |
| `current_capacity`         | `number`  | Current capacity in mAh                    |
| `design_cycle_count`       | `number`  | Design cycle count                         |
| `time_remaining`           | `number`  | Time remaining in seconds                  |
| `time_remaining_formatted` | `string`  | Formatted time remaining (e.g., "6:30:45") |
| `temperature`              | `number`  | Battery temperature in °C                  |
| `percentage`               | `number`  | Battery health percentage (0-100)          |
| `cycle_percentage`         | `number`  | Cycle count percentage (0-100)             |
| `amperage`                 | `number`  | Current amperage in mA                     |
| `power`                    | `number`  | Power in Watts                             |
| `charge_percent`           | `number`  | Current charge percentage (0-100)          |
| `health_percent`           | `number`  | Battery health percentage (0-100)          |

**Example:**

```typescript
const battery = await getBatteryData();
console.log(`Charge: ${battery.charge_percent}%`);
console.log(`Health: ${battery.health_percent}%`);
console.log(`Time Remaining: ${battery.time_remaining_formatted}`);
console.log(`Cycles: ${battery.cycle_count}/${battery.design_cycle_count}`);
```

### Memory & RAM

#### `getMemoryData()` / `getMemoryDataSync()`

Get memory voltage information.

**Returns:** `Promise<Memory>` / `Memory`

| Property  | Type     | Description             |
| --------- | -------- | ----------------------- |
| `voltage` | `number` | Memory voltage in Volts |

#### `getRAMUsage()` / `getRAMUsageSync()`

Get detailed RAM usage statistics.

**Returns:** `Promise<RAMUsage>` / `RAMUsage`

| Property         | Type     | Description                                                 |
| ---------------- | -------- | ----------------------------------------------------------- |
| `total`          | `number` | Total RAM in bytes                                          |
| `totalGB`        | `number` | Total RAM in GB                                             |
| `used`           | `number` | Used RAM in bytes                                           |
| `usedGB`         | `number` | Used RAM in GB                                              |
| `free`           | `number` | Free RAM in bytes                                           |
| `freeGB`         | `number` | Free RAM in GB                                              |
| `usagePercent`   | `number` | Usage percentage (0-100)                                    |
| `active`         | `number` | Active RAM in bytes                                         |
| `activeGB`       | `number` | Active RAM in GB                                            |
| `inactive`       | `number` | Inactive RAM in bytes                                       |
| `inactiveGB`     | `number` | Inactive RAM in GB                                          |
| `wired`          | `number` | Wired RAM in bytes                                          |
| `wiredGB`        | `number` | Wired RAM in GB                                             |
| `compressed`     | `number` | Compressed RAM in bytes                                     |
| `compressedGB`   | `number` | Compressed RAM in GB                                        |
| `app`            | `number` | App memory in bytes                                         |
| `appGB`          | `number` | App memory in GB                                            |
| `cache`          | `number` | Cache memory in bytes                                       |
| `cacheGB`        | `number` | Cache memory in GB                                          |
| `swapTotal`      | `number` | Total swap in bytes                                         |
| `swapTotalGB`    | `number` | Total swap in GB                                            |
| `swapUsed`       | `number` | Used swap in bytes                                          |
| `swapUsedGB`     | `number` | Used swap in GB                                             |
| `swapFree`       | `number` | Free swap in bytes                                          |
| `swapFreeGB`     | `number` | Free swap in GB                                             |
| `pressureLevel`  | `number` | Memory pressure level (1=normal, 2=warning, 4=critical)     |
| `pressureStatus` | `string` | Memory pressure status ("Normal", "Warning", or "Critical") |

**Example:**

```typescript
const ram = await getRAMUsage();
console.log(`RAM Usage: ${ram.usedGB}GB / ${ram.totalGB}GB (${ram.usagePercent}%)`);
console.log(`Memory Pressure: ${ram.pressureStatus}`);
console.log(`Swap: ${ram.swapUsedGB}GB / ${ram.swapTotalGB}GB`);
```

### Disk

#### `getDiskInfo()` / `getDiskInfoSync()`

Get information about all mounted disks.

**Returns:** `Promise<DiskInfo[]>` / `DiskInfo[]`

| Property       | Type      | Description                |
| -------------- | --------- | -------------------------- |
| `name`         | `string`  | Disk name                  |
| `mountPoint`   | `string`  | Mount point path           |
| `fileSystem`   | `string`  | File system type           |
| `totalSize`    | `number`  | Total size in bytes        |
| `totalSizeGB`  | `number`  | Total size in GB           |
| `freeSize`     | `number`  | Free size in bytes         |
| `freeSizeGB`   | `number`  | Free size in GB            |
| `usedSize`     | `number`  | Used size in bytes         |
| `usedSizeGB`   | `number`  | Used size in GB            |
| `usagePercent` | `number`  | Usage percentage (0-100)   |
| `bsdName`      | `string`  | BSD name (e.g., "disk0s1") |
| `isRemovable`  | `boolean` | Is removable media         |

**Example:**

```typescript
const disks = await getDiskInfo();
disks.forEach(disk => {
  console.log(`${disk.name}: ${disk.usedSizeGB}GB / ${disk.totalSizeGB}GB (${disk.usagePercent}%)`);
});
```

### Fans

#### `getFanData()` / `getFanDataSync()`

Get fan speed information for all fans.

**Returns:** `Promise<Fan>` / `Fan`

The `Fan` object is a dictionary where keys are fan indices and values are `FanInfo` objects:

| Property | Type     | Description              |
| -------- | -------- | ------------------------ |
| `rpm`    | `number` | Current fan speed in RPM |
| `min`    | `number` | Minimum fan speed in RPM |
| `max`    | `number` | Maximum fan speed in RPM |

**Example:**

```typescript
const fans = await getFanData();
Object.entries(fans).forEach(([index, fan]) => {
  console.log(`Fan ${index}: ${fan.rpm} RPM (${fan.min}-${fan.max})`);
});
```

### Power

#### `getPowerData()` / `getPowerDataSync()`

Get power consumption for CPU, GPU, and system.

**Returns:** `Promise<Power>` / `Power`

| Property | Type     | Description                       |
| -------- | -------- | --------------------------------- |
| `cpu`    | `number` | CPU power consumption in Watts    |
| `gpu`    | `number` | GPU power consumption in Watts    |
| `system` | `number` | System power consumption in Watts |

**Example:**

```typescript
const power = await getPowerData();
console.log(`CPU Power: ${power.cpu}W`);
console.log(`GPU Power: ${power.gpu}W`);
console.log(`System Power: ${power.system}W`);
```

### Sensors

#### `getSensorData()` / `getSensorDataSync()`

Get all available sensor readings including temperatures, voltages, and currents.

**Returns:** `Promise<SensorData>` / `SensorData`

| Property       | Type       | Description                  |
| -------------- | ---------- | ---------------------------- |
| `temperatures` | `Sensor[]` | Array of temperature sensors |
| `voltages`     | `Sensor[]` | Array of voltage sensors     |
| `currents`     | `Sensor[]` | Array of current sensors     |

**Sensor Object:**

| Property | Type     | Description            |
| -------- | -------- | ---------------------- |
| `name`   | `string` | Sensor name/identifier |
| `value`  | `number` | Sensor reading value   |

**Example:**

```typescript
const sensors = await getSensorData();
sensors.temperatures.forEach(sensor => {
  console.log(`${sensor.name}: ${sensor.value}°C`);
});
```

### System

#### `getSystemData()` / `getSystemDataSync()`

Get system information including OS version, model, and hardware details.

**Returns:** `Promise<SystemInfo>` / `SystemInfo`

| Property          | Type     | Description                               |
| ----------------- | -------- | ----------------------------------------- |
| `totalMemory`     | `number` | Total RAM in bytes                        |
| `totalMemoryGB`   | `number` | Total RAM in GB                           |
| `uptime`          | `number` | System uptime in seconds                  |
| `uptimeFormatted` | `string` | Formatted uptime (e.g., "5d 3h 42m")      |
| `osVersion`       | `string` | macOS version (e.g., "14.1")              |
| `osCodename`      | `string` | macOS codename (e.g., "Sonoma")           |
| `modelName`       | `string` | Model name (e.g., "Apple M3 Pro")         |
| `hardwareModel`   | `string` | Hardware model (e.g., "Mac14,6")          |
| `serialNumber`    | `string` | Serial number                             |
| `screenSize`      | `string` | Screen size (e.g., "14-inch")             |
| `releaseYear`     | `string` | Release year and month (e.g., "Nov 2023") |

**Example:**

```typescript
const system = await getSystemData();
console.log(`Model: ${system.modelName}`);
console.log(`OS: macOS ${system.osVersion} (${system.osCodename})`);
console.log(`Uptime: ${system.uptimeFormatted}`);
console.log(`RAM: ${system.totalMemoryGB}GB`);
```

## Complete Example

```typescript
import {
  getCpuData,
  getGpuData,
  getBatteryData,
  getRAMUsage,
  getDiskInfo,
  getFanData,
  getPowerData,
  getSensorData,
  getSystemData
} from 'macstats';

async function main() {
  try {
    const [system, cpu, gpu, battery, ram, disks, fans, power, sensors] = await Promise.all([
      getSystemData(),
      getCpuData(),
      getGpuData(),
      getBatteryData(),
      getRAMUsage(),
      getDiskInfo(),
      getFanData(),
      getPowerData(),
      getSensorData()
    ]);

    console.log('=== System ===');
    console.log(`Model: ${system.modelName}`);
    console.log(`OS: macOS ${system.osVersion}`);
    console.log(`Uptime: ${system.uptimeFormatted}`);

    console.log('\n=== CPU ===');
    console.log(`Temperature: ${cpu.temperature}°C`);
    console.log(`Power: ${power.cpu}W`);

    console.log('\n=== GPU ===');
    console.log(`Temperature: ${gpu.temperature}°C`);
    console.log(`Power: ${power.gpu}W`);

    console.log('\n=== Memory ===');
    console.log(`Usage: ${ram.usedGB}GB / ${ram.totalGB}GB (${ram.usagePercent}%)`);

    console.log('\n=== Battery ===');
    console.log(`Charge: ${battery.charge_percent}%`);
    console.log(`Health: ${battery.health_percent}%`);

    console.log('\n=== Disks ===');
    disks.forEach(disk => {
      console.log(`${disk.name}: ${disk.usedSizeGB}GB / ${disk.totalSizeGB}GB`);
    });

    console.log('\n=== Fans ===');
    Object.entries(fans).forEach(([index, fan]) => {
      console.log(`Fan ${index}: ${fan.rpm} RPM`);
    });
  } catch (error) {
    console.error('Error:', error);
  }
}

main();
```

## Synchronous vs Asynchronous

All functions have both async and sync versions:

- **Async:** `getCpuData()`, `getGpuData()`, etc. - Returns `Promise`
- **Sync:** `getCpuDataSync()`, `getGpuDataSync()`, etc. - Returns value directly

Use synchronous versions for simple scripts, async versions for better performance in applications.

## Requirements

- macOS 10.12 or later
- Node.js 18.0 or later

## License

MIT
