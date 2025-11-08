import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface GPU {
  temperature: number;
  power: number;
  voltage: number;
}

export async function getGpuData(): Promise<GPU> {
  return {
    temperature: Math.round(smc.gpuTemperature()),
    power: smc.gpuPower(),
    voltage: smc.gpuVoltage()
  };
}

export function getGpuDataSync(): GPU {
  return {
    temperature: smc.gpuTemperature(),
    power: smc.gpuPower(),
    voltage: smc.gpuVoltage()
  };
}

