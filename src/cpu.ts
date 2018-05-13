const smc = require('../build/Release/smc.node');

export interface CPU {
  temperature: number;
}

export function getCpuData(): Promise<CPU> {
  return new Promise((resolve, reject) => {
    resolve({ temperature: smc.temperature() });
  });
}

export function getCpuDataSync(): CPU {
  return { temperature: smc.temperature() } as CPU;
}
