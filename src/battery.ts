import { execSync, exec } from 'child_process';

export interface Battery {
  external_connected: boolean;
  battery_installed: boolean;
  is_charging: boolean;
  fully_charged: boolean;
  voltage: number;
  cycle_count: number;
  design_capacity: number;
  max_capacity: number;
  current_capacity: number;
  design_cycle_count: number;
  time_remaining: number;
  time_remaining_formatted: string;
  temperature: number;
  percentage: number;
  cycle_percentage: number;
}

const cmd = `ioreg -rn AppleSmartBattery`;

export async function getBatteryData(): Promise<Battery> {
  const data = await readData();
  return parseData(data);
}

export function getBatteryDataSync(): Battery {
  return parseData(readDataSync());
}

async function readData(): Promise<string> {
  return new Promise((resolve, reject) => {
    exec(cmd, { encoding: 'utf8' }, (err: Error, stdout: string, stderr: string) => {
      if (err || stderr !== '') {
        reject(err || stderr);
      } else {
        resolve(stdout.toString());
      }
    });
  });
}

function readDataSync(): string {
  return execSync(cmd, { encoding: 'utf8' }).toString();
}

function parseData(data: string): Battery {
  const stats: Battery = data.split('\n').reduce((prev, curr) => {
    curr = curr.trim();

    if (curr.includes('ExternalConnected')) {
      prev.external_connected = curr.split('=')[1].trim() === 'Yes' ? true : false;
    }

    if (curr.includes('BatteryInstalled')) {
      prev.battery_installed = curr.split('=')[1].trim() === 'Yes' ? true : false;
    }

    if (curr.includes('FullyCharged')) {
      prev.fully_charged = curr.split('=')[1].trim() === 'Yes' ? true : false;
    }

    if (curr.includes('IsCharging')) {
      prev.is_charging = curr.split('=')[1].trim() === 'Yes' ? true : false;
    }

    if (curr.includes('"Voltage"') && !curr.includes('LegacyBatteryInfo') && !curr.includes('BatteryData')) {
      prev.voltage = Number(curr.split('=')[1].trim());
    }

    if (curr.includes('"CycleCount"') && !curr.includes('BatteryData')) {
      prev.cycle_count = Number(curr.split('=')[1].trim());
    }

    if (curr.includes('DesignCapacity') && !curr.includes('BatteryData')) {
      prev.design_capacity = Number(curr.split('=')[1].trim());
    }

    if (curr.includes('MaxCapacity')) {
      prev.max_capacity = Number(curr.split('=')[1].trim());
    }

    if (curr.includes('CurrentCapacity')) {
      prev.current_capacity = Number(curr.split('=')[1].trim());
    }

    if (curr.includes('DesignCycleCount9C')) {
      prev.design_cycle_count = Number(curr.split('=')[1].trim());
    }

    if (curr.includes('TimeRemaining')) {
      prev.time_remaining = Number(curr.split('=')[1].trim());
    }

    if (curr.includes('Temperature')) {
      prev.temperature = Number(curr.split('=')[1].trim());
    }

    return prev;
  }, {} as Battery);

  return Object.assign({}, stats, {
    percentage: Math.round(Number((stats.max_capacity / stats.design_capacity) * 100)),
    cycle_percentage: Math.round(Number((stats.cycle_count / stats.design_cycle_count) * 100)),
    temperature: Math.round(Number(stats.temperature / 100)),
    time_remaining_formatted: secondsToHms(stats.time_remaining || 0)
  });
}

function secondsToHms(s: number): string {
  return s === 0 ? '/' : new Date(1000 * s).toISOString().substr(11, 8);
}
