import { getBatteryData } from './battery.js';
import { getFanData } from './fan.js';
import { getCpuData, getCPUUsage } from './cpu.js';
import { getGpuData } from './gpu.js';
import { getPowerData } from './power.js';
import { getMemoryData, getRAMUsage } from './memory.js';
import { getSystemData } from './system.js';
import { getDiskInfo } from './disk.js';
import chalk from 'chalk';
import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

// Types for SMC native module
interface Sensor {
  name: string;
  value: number;
}

interface SMCModule {
  getModelInfo(): string;
  getAllTemperatureSensors(): Sensor[];
  getAllVoltageSensors(): Sensor[];
  getAllCurrentSensors(): Sensor[];
}

const smcModule = smc as SMCModule;

// Check for detailed flag
const args = process.argv.slice(2);
const detailedMode = args.includes('--detailed') || args.includes('-d');
const helpMode = args.includes('--help') || args.includes('-h');

if (helpMode) {
  console.log(chalk.cyan.bold('macstats - macOS System Monitor'));
  console.log('');
  console.log('Usage:');
  console.log('  macstats              Show summary view');
  console.log('  macstats --detailed   Show detailed sensor readings');
  console.log('  macstats -d           Show detailed sensor readings (short)');
  console.log('  macstats --help       Show this help message');
  console.log('');
  process.exit(0);
}

// Helper function to format values
function formatValue(value: number, unit: string, decimals: number = 1): string {
  if (value === 0) {
    return chalk.gray('N/A');
  }
  return chalk.green(value.toFixed(decimals) + unit);
}

// Helper function to group sensors by category
interface SensorGroup {
  [category: string]: Array<{ name: string; value: number }>;
}

function groupSensors(sensors: Array<{ name: string; value: number }>): SensorGroup {
  const groups: SensorGroup = {};

  sensors.forEach(sensor => {
    let category = 'Other';

    if (sensor.name.includes('PMU tdie')) {
      category = 'CPU Die';
    } else if (sensor.name.includes('PMU tdev')) {
      category = 'Device';
    } else if (sensor.name.includes('gas gauge battery')) {
      category = 'Battery';
    } else if (sensor.name.includes('NAND')) {
      category = 'Storage';
    } else if (sensor.name.includes('PMU vbuck')) {
      category = 'Buck Converter';
    } else if (sensor.name.includes('PMU vldo')) {
      category = 'LDO Regulator';
    } else if (sensor.name.includes('PMU ibuck')) {
      category = 'Buck Current';
    } else if (sensor.name.includes('PMU ildo')) {
      category = 'LDO Current';
    }

    if (!groups[category]) {
      groups[category] = [];
    }
    groups[category].push(sensor);
  });

  return groups;
}

// Helper function to calculate statistics
function calculateStats(values: number[]): { min: number; max: number; avg: number } {
  const validValues = values.filter(v => v > 0);
  if (validValues.length === 0) {
    return { min: 0, max: 0, avg: 0 };
  }
  return {
    min: Math.min(...validValues),
    max: Math.max(...validValues),
    avg: validValues.reduce((a, b) => a + b, 0) / validValues.length
  };
}

// Helper function to print a sensor group summary
function printSensorSummary(
  title: string,
  sensors: Array<{ name: string; value: number }>,
  unit: string,
  decimals: number = 1
): void {
  const validSensors = sensors.filter(s => s.value > 0);
  if (validSensors.length === 0) {
    return; // Skip empty sensor groups
  }

  if (detailedMode) {
    // Detailed mode: show all sensors
    console.log(`  ${chalk.bold(title)}:`);
    validSensors.forEach(sensor => {
      const value = chalk.green(sensor.value.toFixed(decimals) + unit);
      console.log(`    ${chalk.gray(sensor.name + ':')} ${value}`);
    });
  } else {
    // Summary mode: show statistics
    const stats = calculateStats(validSensors.map(s => s.value));
    const avgStr = chalk.green.bold(stats.avg.toFixed(decimals) + unit);
    const minStr = chalk.cyan(stats.min.toFixed(decimals) + unit);
    const maxStr = chalk.yellow(stats.max.toFixed(decimals) + unit);
    const countStr = chalk.gray(`(${validSensors.length})`);

    console.log(`  ${chalk.gray(title + ':')} avg ${avgStr} ${chalk.gray('│')} min ${minStr} ${chalk.gray('│')} max ${maxStr} ${countStr}`);
  }
}

async function displaySystemInfo(): Promise<void> {
  const systemData = await getSystemData();
  console.log('');
  console.log(chalk.cyan.bold('┌─────────────────────────────────────────────────────────────────┐'));
  console.log(chalk.cyan.bold('│              macOS System Monitor - macstats                   │'));
  console.log(chalk.cyan.bold('└─────────────────────────────────────────────────────────────────┘'));
  console.log('');
  console.log(chalk.white.bold('🖥️  System Information'));
  console.log(`  ${chalk.gray('Model:')}      ${chalk.green.bold(systemData.modelName)}`);

  // Show hardware model and screen size if available
  if (systemData.hardwareModel) {
    let hardwareInfo = systemData.hardwareModel;
    if (systemData.screenSize && systemData.releaseYear) {
      hardwareInfo += ` (${systemData.screenSize}, ${systemData.releaseYear})`;
    } else if (systemData.screenSize) {
      hardwareInfo += ` (${systemData.screenSize})`;
    } else if (systemData.releaseYear) {
      hardwareInfo += ` (${systemData.releaseYear})`;
    }
    console.log(`  ${chalk.gray('Hardware:')}   ${chalk.green(hardwareInfo)}`);
  }

  console.log(`  ${chalk.gray('OS:')}         ${chalk.green(systemData.osVersion)} ${chalk.gray('(' + systemData.osCodename + ')')}`);

  if (systemData.serialNumber) {
    console.log(`  ${chalk.gray('Serial:')}     ${chalk.green(systemData.serialNumber)}`);
  }

  console.log(`  ${chalk.gray('RAM:')}        ${chalk.green(systemData.totalMemoryGB + ' GB')}`);
  console.log(`  ${chalk.gray('Uptime:')}     ${chalk.green(systemData.uptimeFormatted)}`);
  console.log('');
}

async function displayTemperatureSensors(): Promise<void> {
  const tempSensors = smcModule.getAllTemperatureSensors();
  if (tempSensors && tempSensors.length > 0) {
    console.log(chalk.white.bold('🌡️  Temperature Sensors'));
    const groups = groupSensors(tempSensors);

    // Sort categories in a logical order
    const categoryOrder = ['CPU Die', 'Device', 'Battery', 'Storage', 'Other'];
    const sortedCategories = Object.keys(groups).sort((a, b) => {
      const indexA = categoryOrder.indexOf(a);
      const indexB = categoryOrder.indexOf(b);
      if (indexA === -1 && indexB === -1) return a.localeCompare(b);
      if (indexA === -1) return 1;
      if (indexB === -1) return -1;
      return indexA - indexB;
    });

    sortedCategories.forEach(category => {
      printSensorSummary(category, groups[category], '°C', 1);
    });

    console.log('');
  }
}

async function displayVoltageSensors(): Promise<void> {
  const voltageSensors = smcModule.getAllVoltageSensors();
  if (voltageSensors && voltageSensors.length > 0) {
    console.log(chalk.white.bold('⚡ Voltage Sensors'));
    const groups = groupSensors(voltageSensors);

    const categoryOrder = ['Buck Converter', 'LDO Regulator', 'Other'];
    const sortedCategories = Object.keys(groups).sort((a, b) => {
      const indexA = categoryOrder.indexOf(a);
      const indexB = categoryOrder.indexOf(b);
      if (indexA === -1 && indexB === -1) return a.localeCompare(b);
      if (indexA === -1) return 1;
      if (indexB === -1) return -1;
      return indexA - indexB;
    });

    sortedCategories.forEach(category => {
      printSensorSummary(category, groups[category], 'V', 3);
    });

    console.log('');
  }
}

async function displayCurrentSensors(): Promise<void> {
  const currentSensors = smcModule.getAllCurrentSensors();
  if (currentSensors && currentSensors.length > 0) {
    console.log(chalk.white.bold('🔌 Current Sensors'));
    const groups = groupSensors(currentSensors);

    const categoryOrder = ['Buck Current', 'LDO Current', 'Other'];
    const sortedCategories = Object.keys(groups).sort((a, b) => {
      const indexA = categoryOrder.indexOf(a);
      const indexB = categoryOrder.indexOf(b);
      if (indexA === -1 && indexB === -1) return a.localeCompare(b);
      if (indexA === -1) return 1;
      if (indexB === -1) return -1;
      return indexA - indexB;
    });

    sortedCategories.forEach(category => {
      printSensorSummary(category, groups[category], 'A', 3);
    });

    console.log('');
  }
}

async function displayCpuInfo(): Promise<void> {
  const data = await getCpuData();
  const usage = await getCPUUsage();
  const hasData = data.temperature > 0 || data.temperatureDie > 0 || data.power > 0 || data.voltage > 0;

  if (hasData || usage.totalUsage > 0) {
    console.log(chalk.white.bold('💻 CPU'));

    // Display usage first
    if (usage.totalUsage > 0) {
      const usageColor = usage.totalUsagePercent > 80 ? chalk.red : usage.totalUsagePercent > 50 ? chalk.yellow : chalk.green;
      console.log(`  ${chalk.gray('Usage:      ')} ${usageColor(usage.totalUsagePercent + '%')}`);
      console.log(`  ${chalk.gray('Load Avg:   ')} ${chalk.green(usage.loadAvg1 + ', ' + usage.loadAvg5 + ', ' + usage.loadAvg15)}`);
    }

    if (data.temperature > 0) {
      console.log(`  ${chalk.gray('Temperature:')} ${formatValue(data.temperature, '°C')}`);
    }
    if (data.temperatureDie > 0) {
      console.log(`  ${chalk.gray('Die Temp:   ')} ${formatValue(data.temperatureDie, '°C')}`);
    }
    if (data.power > 0) {
      console.log(`  ${chalk.gray('Power:      ')} ${formatValue(data.power, 'W', 2)}`);
    }
    if (data.voltage > 0) {
      console.log(`  ${chalk.gray('Voltage:    ')} ${formatValue(data.voltage, 'V', 3)}`);
    }
    console.log('');
  }
}

async function displayGpuInfo(): Promise<void> {
  const data = await getGpuData();
  if (data.temperature > 0 || data.power > 0 || data.voltage > 0) {
    console.log(chalk.white.bold('🎮 GPU'));
    if (data.temperature > 0) {
      console.log(`  ${chalk.gray('Temperature:')} ${formatValue(data.temperature, '°C')}`);
    }
    if (data.power > 0) {
      console.log(`  ${chalk.gray('Power:      ')} ${formatValue(data.power, 'W', 2)}`);
    }
    if (data.voltage > 0) {
      console.log(`  ${chalk.gray('Voltage:    ')} ${formatValue(data.voltage, 'V', 3)}`);
    }
    console.log('');
  }
}

async function displayPowerInfo(): Promise<void> {
  const data = await getPowerData();
  if (data.cpu > 0 || data.gpu > 0 || data.system > 0) {
    console.log(chalk.white.bold('⚙️  Power Consumption'));
    if (data.cpu > 0) {
      console.log(`  ${chalk.gray('CPU:        ')} ${formatValue(data.cpu, 'W', 2)}`);
    }
    if (data.gpu > 0) {
      console.log(`  ${chalk.gray('GPU:        ')} ${formatValue(data.gpu, 'W', 2)}`);
    }
    if (data.system > 0) {
      console.log(`  ${chalk.gray('System:     ')} ${formatValue(data.system, 'W', 2)}`);
    }
    console.log('');
  }
}

async function displayMemoryInfo(): Promise<void> {
  const data = await getMemoryData();
  if (data.voltage > 0) {
    console.log(chalk.white.bold('🧠 Memory'));
    console.log(`  ${chalk.gray('Voltage:    ')} ${formatValue(data.voltage, 'V', 3)}`);
    console.log('');
  }
}

async function displayRAMUsage(): Promise<void> {
  const usage = await getRAMUsage();
  console.log(chalk.white.bold('💾 RAM Usage'));
  console.log(`  ${chalk.gray('Total:      ')} ${chalk.green(usage.totalGB + ' GB')}`);
  console.log(`  ${chalk.gray('Used:       ')} ${chalk.green(usage.usedGB + ' GB')} ${chalk.gray('(' + usage.usagePercent + '%)')}`);
  console.log(`  ${chalk.gray('Free:       ')} ${chalk.green(usage.freeGB + ' GB')}`);
  console.log(`  ${chalk.gray('App:        ')} ${chalk.green(usage.appGB + ' GB')}`);
  console.log(`  ${chalk.gray('Wired:      ')} ${chalk.green(usage.wiredGB + ' GB')}`);
  console.log(`  ${chalk.gray('Compressed: ')} ${chalk.green(usage.compressedGB + ' GB')}`);

  if (usage.swapUsed > 0) {
    console.log(`  ${chalk.gray('Swap Used:  ')} ${chalk.yellow(usage.swapUsedGB + ' GB')} ${chalk.gray('of ' + usage.swapTotalGB + ' GB')}`);
  }

  // Display memory pressure
  let pressureColor = chalk.green;
  if (usage.pressureLevel === 2) {
    pressureColor = chalk.yellow;
  } else if (usage.pressureLevel === 4) {
    pressureColor = chalk.red;
  }
  console.log(`  ${chalk.gray('Pressure:   ')} ${pressureColor(usage.pressureStatus)}`);
  console.log('');
}

async function displayDiskInfo(): Promise<void> {
  const disks = await getDiskInfo();

  // In detailed mode, show all disks. Otherwise, filter to main user-visible disks
  const displayDisks = detailedMode ? disks : disks.filter(disk => {
    // Show root volume
    if (disk.mountPoint === '/') return true;

    // Skip system volumes
    if (disk.mountPoint.startsWith('/System/Volumes/')) return false;
    if (disk.mountPoint.startsWith('/Library/Developer/CoreSimulator/')) return false;

    // Show removable disks that are not simulators
    if (disk.isRemovable && !disk.mountPoint.includes('Simulator')) return true;

    return false;
  });

  if (displayDisks.length > 0) {
    console.log(chalk.white.bold('💿 Disks'));

    displayDisks.forEach(disk => {
      const usageColor = disk.usagePercent > 90 ? chalk.red : disk.usagePercent > 75 ? chalk.yellow : chalk.green;
      const diskName = disk.name || disk.mountPoint;

      console.log(`  ${chalk.cyan(diskName)}`);
      console.log(`    ${chalk.gray('Total:      ')} ${chalk.green(disk.totalSizeGB + ' GB')}`);
      console.log(`    ${chalk.gray('Used:       ')} ${usageColor(disk.usedSizeGB + ' GB')} ${chalk.gray('(' + disk.usagePercent + '%)')}`);
      console.log(`    ${chalk.gray('Free:       ')} ${chalk.green(disk.freeSizeGB + ' GB')}`);
      console.log(`    ${chalk.gray('Mount:      ')} ${chalk.gray(disk.mountPoint)}`);
      console.log(`    ${chalk.gray('Type:       ')} ${chalk.gray(disk.fileSystem)}`);
      if (detailedMode && disk.bsdName) {
        console.log(`    ${chalk.gray('BSD Name:   ')} ${chalk.gray(disk.bsdName)}`);
      }
    });

    console.log('');
  }
}

async function displayFanInfo(): Promise<void> {
  const data = await getFanData();
  const fanEntries = Object.entries(data);

  // Only display if there are fans with actual data
  const activeFans = fanEntries.filter(([_, fanInfo]) =>
    fanInfo.rpm > 0 || fanInfo.min > 0 || fanInfo.max > 0
  );

  if (activeFans.length > 0) {
    console.log(chalk.white.bold('🌀 Fans'));
    activeFans.forEach(([index, fanInfo]) => {
      const rpm = fanInfo.rpm;
      const min = fanInfo.min;
      const max = fanInfo.max;
      const rpmStr = rpm > 0 ? chalk.green(String(rpm) + ' RPM') : chalk.gray('N/A');
      const rangeStr = chalk.gray(`(${min}-${max} RPM)`);
      console.log(`  ${chalk.gray('Fan #' + index + ':')} ${rpmStr} ${rangeStr}`);
    });
    console.log('');
  }
}

async function displayBatteryInfo(): Promise<void> {
  const data = await getBatteryData();

  if (data.battery_installed) {
    console.log(chalk.white.bold('🔋 Battery'));

    // Determine battery status
    let chargeStatus: string;
    if (data.is_charging) {
      chargeStatus = chalk.yellow('⚡ Charging');
    } else if (data.external_connected) {
      if (data.fully_charged) {
        chargeStatus = chalk.green('🔌 Fully Charged');
      } else {
        chargeStatus = chalk.cyan('🔌 Plugged In - Not Charging');
      }
    } else {
      chargeStatus = chalk.green('🔋 On Battery');
    }

    const healthPercent = data.health_percent;
    const healthColor = healthPercent >= 80 ? chalk.green : healthPercent >= 60 ? chalk.yellow : chalk.red;
    const chargePercent = data.charge_percent;
    const chargeColor = chargePercent >= 50 ? chalk.green : chargePercent >= 20 ? chalk.yellow : chalk.red;

    console.log(`  ${chalk.gray('Status:     ')} ${chargeStatus}`);
    console.log(`  ${chalk.gray('Health:     ')} ${healthColor(String(healthPercent) + '%')} ${chalk.gray(`(${data.max_capacity}/${data.design_capacity} mAh)`)}`);
    console.log(`  ${chalk.gray('Charge:     ')} ${chargeColor(String(chargePercent) + '%')} ${chalk.gray(`(${data.current_capacity} mAh)`)}`);
    console.log(`  ${chalk.gray('Cycles:     ')} ${chalk.cyan(String(data.cycle_count))} ${chalk.gray(`of ${data.design_cycle_count} (${data.cycle_percentage}%)`)}`);
    console.log(`  ${chalk.gray('Temperature:')} ${chalk.green(String(data.temperature) + '°C')}`);

    // Show power consumption
    if (data.power > 0) {
      const powerColor = data.is_charging ? chalk.yellow : chalk.cyan;
      const powerLabel = data.is_charging ? 'Charging at:' : 'Power Draw:';
      console.log(`  ${chalk.gray(powerLabel)} ${powerColor(data.power.toFixed(2) + 'W')}`);
    }

    // Show voltage and amperage
    if (data.voltage > 0) {
      const voltageV = (data.voltage / 1000).toFixed(2);
      console.log(`  ${chalk.gray('Voltage:    ')} ${chalk.cyan(voltageV + 'V')}`);
    }

    if (data.amperage !== 0) {
      const amperageA = (Math.abs(data.amperage) / 1000).toFixed(2);
      const amperageLabel = data.amperage < 0 ? 'Discharge:  ' : 'Charge Rate:';
      console.log(`  ${chalk.gray(amperageLabel)} ${chalk.cyan(amperageA + 'A')}`);
    }

    // Show time remaining only when on battery and available
    if (!data.is_charging && data.time_remaining > 0) {
      console.log(`  ${chalk.gray('Remaining:  ')} ${chalk.cyan(data.time_remaining_formatted)}`);
    }

    console.log('');
  }
}

async function main(): Promise<void> {
  try {
    await displaySystemInfo();
    await displayRAMUsage();
    await displayDiskInfo();
    await displayTemperatureSensors();
    await displayVoltageSensors();
    await displayCurrentSensors();
    await displayCpuInfo();
    await displayGpuInfo();
    await displayPowerInfo();
    await displayMemoryInfo();
    await displayFanInfo();
    await displayBatteryInfo();
  } catch (err) {
    const error = err as Error;
    console.error(chalk.red('Error:'), error.message);
    process.exit(1);
  }
}

// Run the main function
main();
