import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface RawRAMUsage {
  total: number;
  used: number;
  free: number;
  active: number;
  inactive: number;
  wired: number;
  compressed: number;
  app: number;
  cache: number;
  swap_total: number;
  swap_used: number;
  swap_free: number;
  pressure_level: number;
}

export interface RAMUsage {
  total: number;           // Total RAM in bytes
  totalGB: number;         // Total RAM in GB
  used: number;            // Used RAM in bytes
  usedGB: number;          // Used RAM in GB
  free: number;            // Free RAM in bytes
  freeGB: number;          // Free RAM in GB
  usagePercent: number;    // Usage percentage
  active: number;          // Active RAM in bytes
  activeGB: number;        // Active RAM in GB
  inactive: number;        // Inactive RAM in bytes
  inactiveGB: number;      // Inactive RAM in GB
  wired: number;           // Wired RAM in bytes
  wiredGB: number;         // Wired RAM in GB
  compressed: number;      // Compressed RAM in bytes
  compressedGB: number;    // Compressed RAM in GB
  app: number;             // App memory in bytes
  appGB: number;           // App memory in GB
  cache: number;           // Cache memory in bytes
  cacheGB: number;         // Cache memory in GB
  swapTotal: number;       // Total swap in bytes
  swapTotalGB: number;     // Total swap in GB
  swapUsed: number;        // Used swap in bytes
  swapUsedGB: number;      // Used swap in GB
  swapFree: number;        // Free swap in bytes
  swapFreeGB: number;      // Free swap in GB
  pressureLevel: number;   // Memory pressure level (1=normal, 2=warning, 4=critical)
  pressureStatus: string;  // Memory pressure status string
}

export interface Memory {
  voltage: number;
  usage?: RAMUsage;
}

function bytesToGB(bytes: number): number {
  return Math.round((bytes / (1024 ** 3)) * 10) / 10;
}

function parseRAMUsage(raw: RawRAMUsage): RAMUsage {
  const usagePercent = raw.total > 0 ? Math.round((raw.used / raw.total) * 100) : 0;

  let pressureStatus = 'Normal';
  if (raw.pressure_level === 2) {
    pressureStatus = 'Warning';
  } else if (raw.pressure_level === 4) {
    pressureStatus = 'Critical';
  }

  return {
    total: raw.total,
    totalGB: bytesToGB(raw.total),
    used: raw.used,
    usedGB: bytesToGB(raw.used),
    free: raw.free,
    freeGB: bytesToGB(raw.free),
    usagePercent,
    active: raw.active,
    activeGB: bytesToGB(raw.active),
    inactive: raw.inactive,
    inactiveGB: bytesToGB(raw.inactive),
    wired: raw.wired,
    wiredGB: bytesToGB(raw.wired),
    compressed: raw.compressed,
    compressedGB: bytesToGB(raw.compressed),
    app: raw.app,
    appGB: bytesToGB(raw.app),
    cache: raw.cache,
    cacheGB: bytesToGB(raw.cache),
    swapTotal: raw.swap_total,
    swapTotalGB: bytesToGB(raw.swap_total),
    swapUsed: raw.swap_used,
    swapUsedGB: bytesToGB(raw.swap_used),
    swapFree: raw.swap_free,
    swapFreeGB: bytesToGB(raw.swap_free),
    pressureLevel: raw.pressure_level,
    pressureStatus
  };
}

export async function getMemoryData(): Promise<Memory> {
  return {
    voltage: smc.memoryVoltage()
  };
}

export function getMemoryDataSync(): Memory {
  return {
    voltage: smc.memoryVoltage()
  };
}

export async function getRAMUsage(): Promise<RAMUsage> {
  const raw: RawRAMUsage = smc.getRAMUsageData();
  return parseRAMUsage(raw);
}

export function getRAMUsageSync(): RAMUsage {
  const raw: RawRAMUsage = smc.getRAMUsageData();
  return parseRAMUsage(raw);
}
