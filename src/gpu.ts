import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface GPU {
  temperature: number;
  voltage: number;
  usage: number;
}

export async function getGpuData(): Promise<GPU> {
  return new Promise((resolve, reject) => {
    try {
      resolve({
        temperature: Math.round(smc.gpuTemperature()),
        voltage: smc.gpuVoltage(),
        usage: Math.round(smc.gpuUsage() * 100)
      });
    } catch (error) {
      reject(error);
    }
  });
}

export function getGpuDataSync(): GPU {
  return {
    temperature: smc.gpuTemperature(),
    voltage: smc.gpuVoltage(),
    usage: Math.round(smc.gpuUsage() * 100)
  };
}

