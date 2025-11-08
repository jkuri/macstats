import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface Power {
  cpu: number;
  gpu: number;
  system: number;
}

export async function getPowerData(): Promise<Power> {
  return {
    cpu: smc.cpuPower(),
    gpu: smc.gpuPower(),
    system: smc.systemPower()
  };
}

export function getPowerDataSync(): Power {
  return {
    cpu: smc.cpuPower(),
    gpu: smc.gpuPower(),
    system: smc.systemPower()
  };
}

