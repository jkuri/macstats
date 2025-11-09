import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface FanInfo {
  rpm: number;
  min: number;
  max: number;
}

export interface Fan {
  [key: string]: FanInfo;
}

export async function getFanData(): Promise<Fan> {
  return new Promise((resolve, reject) => {
    try {
      resolve(fanData());
    } catch (error) {
      reject(error);
    }
  });
}

export function getFanDataSync(): Fan {
  return fanData();
}

function fanData(): Fan {
  return Array.from(Array(smc.fans()), (_, i) => ({
    rpm: smc.fanRpm(i),
    min: smc.fanMin(i),
    max: smc.fanMax(i)
  })).reduce((prev, curr, i) => {
    prev[i] = curr;
    return prev;
  }, {} as Fan);
}
