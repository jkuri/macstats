import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface Power {
  cpu: number;         // CPU power in Watts
  gpu: number;         // GPU power in Watts
  ane: number;         // ANE (Apple Neural Engine) power in Watts
  all: number;         // Combined CPU + GPU + ANE power in Watts
  system: number;      // Total system power in Watts
  ram: number;         // RAM power in Watts
  gpu_ram: number;     // GPU RAM power in Watts
}

export async function getPowerData(): Promise<Power> {
  return new Promise((resolve, reject) => {
    try {
      // Use getAllPower() to get all metrics in a single call
      // This is more efficient and ensures all values are from the same measurement
      const power = smc.getAllPower();
      resolve(power);
    } catch (error) {
      reject(error);
    }
  });
}

export function getPowerDataSync(): Power {
  // Use getAllPower() to get all metrics in a single call
  // This is more efficient and ensures all values are from the same measurement
  return smc.getAllPower();
}

