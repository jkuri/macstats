import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface Sensor {
  name: string;
  value: number;
}

export interface SensorData {
  temperatures: Sensor[];
  voltages: Sensor[];
  currents: Sensor[];
}

export async function getSensorData(): Promise<SensorData> {
  return {
    temperatures: smc.getAllTemperatureSensors() as Sensor[],
    voltages: smc.getAllVoltageSensors() as Sensor[],
    currents: smc.getAllCurrentSensors() as Sensor[]
  };
}

export function getSensorDataSync(): SensorData {
  return {
    temperatures: smc.getAllTemperatureSensors() as Sensor[],
    voltages: smc.getAllVoltageSensors() as Sensor[],
    currents: smc.getAllCurrentSensors() as Sensor[]
  };
}
