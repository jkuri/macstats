const smc = require('../build/Release/smc.node');

export interface CPU {
  temperature: number;
}

export function getCpuData(): Promise<CPU> {
  return Promise.resolve().then(() => ({ temperature: Math.round(smc.temperature()) }));
}

export function getCpuDataSync(): CPU {
  return { temperature: smc.temperature() } as CPU;
}
