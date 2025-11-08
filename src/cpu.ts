import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface CPU {
  temperature: number;
  temperatureDie: number;
  power: number;
  voltage: number;
}

export interface RawCPUUsage {
  user_load: number;
  system_load: number;
  idle_load: number;
  total_usage: number;
  load_avg_1: number;
  load_avg_5: number;
  load_avg_15: number;
}

export interface CPUUsage {
  userLoad: number;          // User CPU load (0.0 - 1.0)
  systemLoad: number;        // System CPU load (0.0 - 1.0)
  idleLoad: number;          // Idle CPU load (0.0 - 1.0)
  totalUsage: number;        // Total CPU usage (0.0 - 1.0)
  totalUsagePercent: number; // Total CPU usage as percentage (0 - 100)
  loadAvg1: number;          // 1 minute load average
  loadAvg5: number;          // 5 minute load average
  loadAvg15: number;         // 15 minute load average
}

export async function getCpuData(): Promise<CPU> {
  return {
    temperature: Math.round(smc.temperature()),
    temperatureDie: Math.round(smc.cpuTemperatureDie()),
    power: smc.cpuPower(),
    voltage: smc.cpuVoltage()
  };
}

export function getCpuDataSync(): CPU {
  return {
    temperature: smc.temperature(),
    temperatureDie: smc.cpuTemperatureDie(),
    power: smc.cpuPower(),
    voltage: smc.cpuVoltage()
  };
}

function parseCPUUsage(raw: RawCPUUsage): CPUUsage {
  return {
    userLoad: raw.user_load,
    systemLoad: raw.system_load,
    idleLoad: raw.idle_load,
    totalUsage: raw.total_usage,
    totalUsagePercent: Math.round(raw.total_usage * 100),
    loadAvg1: Math.round(raw.load_avg_1 * 100) / 100,
    loadAvg5: Math.round(raw.load_avg_5 * 100) / 100,
    loadAvg15: Math.round(raw.load_avg_15 * 100) / 100,
  };
}

export async function getCPUUsage(): Promise<CPUUsage> {
  return new Promise((resolve, reject) => {
    try {
      const raw: RawCPUUsage = smc.getCPUUsageData();
      resolve(parseCPUUsage(raw));
    } catch (error) {
      reject(error);
    }
  });
}

export function getCPUUsageSync(): CPUUsage {
  const raw: RawCPUUsage = smc.getCPUUsageData();
  return parseCPUUsage(raw);
}
