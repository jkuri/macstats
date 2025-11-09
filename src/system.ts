import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface RawSystemData {
  total_memory: number;
  uptime: number;
  os_version: string;
  os_codename: string;
  model_name: string;
  hardware_model: string;
  serial_number: string;
  screen_size: string;
  release_year: string;
}

export interface SystemInfo {
  totalMemory: number;      // Total RAM in bytes
  totalMemoryGB: number;    // Total RAM in GB
  uptime: number;           // System uptime in seconds
  uptimeFormatted: string;  // Formatted uptime string
  osVersion: string;        // macOS version
  osCodename: string;       // macOS codename (e.g., "Tahoe")
  modelName: string;        // Model name (e.g., "Apple M3 Pro")
  hardwareModel: string;    // Hardware model (e.g., "Mac14,6")
  serialNumber: string;     // Serial number
  screenSize: string;       // Screen size (e.g., "14-inch")
  releaseYear: string;      // Release year and month (e.g., "Nov 2023")
}

function formatUptime(seconds: number): string {
  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);

  const parts: string[] = [];
  if (days > 0) parts.push(`${days}d`);
  if (hours > 0) parts.push(`${hours}h`);
  if (minutes > 0) parts.push(`${minutes}m`);

  return parts.length > 0 ? parts.join(' ') : '0m';
}

function parseData(raw: RawSystemData): SystemInfo {
  return {
    totalMemory: raw.total_memory,
    totalMemoryGB: Math.round((raw.total_memory / (1024 ** 3)) * 10) / 10,
    uptime: raw.uptime,
    uptimeFormatted: formatUptime(raw.uptime),
    osVersion: raw.os_version,
    osCodename: raw.os_codename,
    modelName: raw.model_name,
    hardwareModel: raw.hardware_model,
    serialNumber: raw.serial_number,
    screenSize: raw.screen_size,
    releaseYear: raw.release_year
  };
}

export async function getSystemData(): Promise<SystemInfo> {
  return new Promise((resolve, reject) => {
    try {
      const raw: RawSystemData = smc.getSystemData();
      resolve(parseData(raw));
    } catch (error) {
      reject(error);
    }
  });
}

export function getSystemDataSync(): SystemInfo {
  const raw: RawSystemData = smc.getSystemData();
  return parseData(raw);
}

