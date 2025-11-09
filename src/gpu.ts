import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface GPU {
  temperature: number;
  power: number;
  voltage: number;
  usage: number;
}

export async function getGpuData(): Promise<GPU> {
  return {
    temperature: Math.round(smc.gpuTemperature()),
    power: smc.gpuPower(),
    voltage: smc.gpuVoltage(),
    usage: Math.round(smc.gpuUsage() * 100)
  };
}

export function getGpuDataSync(): GPU {
  return {
    temperature: smc.gpuTemperature(),
    power: smc.gpuPower(),
    voltage: smc.gpuVoltage(),
    usage: Math.round(smc.gpuUsage() * 100)
  };
}

