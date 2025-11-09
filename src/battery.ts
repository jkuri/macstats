import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

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
  amperage: number;
  power: number;
  charge_percent: number;
  health_percent: number;
}

export async function getBatteryData(): Promise<Battery> {
  return new Promise((resolve, reject) => {
    try {
      resolve(getBatteryDataSync());
    } catch (error) {
      reject(error);
    }
  });
}

export function getBatteryDataSync(): Battery {
  const data = smc.getBatteryData();
  return parseData(data);
}

interface RawBatteryData {
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
  temperature: number;
  amperage: number;
  charge_percent: number;
  health_percent: number;
}

function parseData(data: RawBatteryData): Battery {
  // Calculate power (Watts) = Voltage (mV) * Amperage (mA) / 1,000,000
  const power = data.voltage && data.amperage
    ? Math.abs((data.voltage * data.amperage) / 1000000)
    : 0;

  return {
    external_connected: data.external_connected,
    battery_installed: data.battery_installed,
    is_charging: data.is_charging,
    fully_charged: data.fully_charged,
    voltage: data.voltage,
    cycle_count: data.cycle_count,
    design_capacity: data.design_capacity,
    max_capacity: data.max_capacity,
    current_capacity: data.current_capacity,
    design_cycle_count: data.design_cycle_count,
    time_remaining: data.time_remaining,
    temperature: Math.round(data.temperature / 100),
    percentage: Math.round((data.max_capacity / data.design_capacity) * 100),
    cycle_percentage: Math.round((data.cycle_count / data.design_cycle_count) * 100),
    time_remaining_formatted: secondsToHms(data.time_remaining),
    power: Math.round(power * 100) / 100,
    amperage: data.amperage,
    charge_percent: data.charge_percent,
    health_percent: data.health_percent
  };
}

function secondsToHms(s: number): string {
  if (s === 0 || s === 65535) {
    return 'Calculating...';
  }
  const hours = Math.floor(s / 3600);
  const minutes = Math.floor((s % 3600) / 60);
  const seconds = s % 60;
  return `${hours}:${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
}
