import { createRequire } from 'node:module';

const requireNative = createRequire(import.meta.url);
const smc = requireNative('../build/Release/smc.node');

export interface RawDiskInfo {
  name: string;
  mount_point: string;
  file_system: string;
  total_size: number;
  free_size: number;
  used_size: number;
  bsd_name: string;
  is_removable: boolean;
}

export interface DiskInfo {
  name: string;              // Disk name
  mountPoint: string;        // Mount point path
  fileSystem: string;        // File system type
  totalSize: number;         // Total size in bytes
  totalSizeGB: number;       // Total size in GB
  freeSize: number;          // Free size in bytes
  freeSizeGB: number;        // Free size in GB
  usedSize: number;          // Used size in bytes
  usedSizeGB: number;        // Used size in GB
  usagePercent: number;      // Usage percentage (0-100)
  bsdName: string;           // BSD name (e.g., disk0s1)
  isRemovable: boolean;      // Is removable media
}

function bytesToGB(bytes: number): number {
  return Math.round((bytes / (1024 * 1024 * 1024)) * 10) / 10;
}

function parseDiskInfo(raw: RawDiskInfo): DiskInfo {
  const usagePercent = raw.total_size > 0 
    ? Math.round((raw.used_size / raw.total_size) * 100)
    : 0;

  return {
    name: raw.name,
    mountPoint: raw.mount_point,
    fileSystem: raw.file_system,
    totalSize: raw.total_size,
    totalSizeGB: bytesToGB(raw.total_size),
    freeSize: raw.free_size,
    freeSizeGB: bytesToGB(raw.free_size),
    usedSize: raw.used_size,
    usedSizeGB: bytesToGB(raw.used_size),
    usagePercent,
    bsdName: raw.bsd_name,
    isRemovable: raw.is_removable,
  };
}

export async function getDiskInfo(): Promise<DiskInfo[]> {
  return new Promise((resolve, reject) => {
    try {
      const rawDisks: RawDiskInfo[] = smc.getDiskData();
      const disks = rawDisks.map(parseDiskInfo);
      resolve(disks);
    } catch (error) {
      reject(error);
    }
  });
}

export function getDiskInfoSync(): DiskInfo[] {
  const rawDisks: RawDiskInfo[] = smc.getDiskData();
  return rawDisks.map(parseDiskInfo);
}

