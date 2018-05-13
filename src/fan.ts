const smc = require('../build/Release/smc.node');

export interface Fan {
  [key: string]: number;
}

export function getFanData(): Promise<Fan> {
  return new Promise((resolve, reject) => {
    resolve(fanData());
  });
}

export function getFanDataSync(): Fan {
  return fanData();
}

function fanData(): Fan {
  return Array.from(Array(smc.fans()), (_, i) => smc.fanRpm(i))
    .reduce((prev, curr, i) => {
      prev[i] = curr;
      return prev;
    }, {} as Fan);
}
