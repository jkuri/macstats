import { describe, it, expect } from 'vitest';
import { getDiskInfo, getDiskInfoSync } from '../src/disk.js';

describe('Disk Information', () => {
  it('should return disk information array', async () => {
    const disks = await getDiskInfo();
    
    expect(disks).toBeDefined();
    expect(Array.isArray(disks)).toBe(true);
    expect(disks.length).toBeGreaterThan(0);
  });

  it('should have valid disk properties', async () => {
    const disks = await getDiskInfo();
    const disk = disks[0];
    
    expect(disk).toHaveProperty('name');
    expect(disk).toHaveProperty('mountPoint');
    expect(disk).toHaveProperty('fileSystem');
    expect(disk).toHaveProperty('totalSize');
    expect(disk).toHaveProperty('totalSizeGB');
    expect(disk).toHaveProperty('freeSize');
    expect(disk).toHaveProperty('freeSizeGB');
    expect(disk).toHaveProperty('usedSize');
    expect(disk).toHaveProperty('usedSizeGB');
    expect(disk).toHaveProperty('usagePercent');
    expect(disk).toHaveProperty('bsdName');
    expect(disk).toHaveProperty('isRemovable');
  });

  it('should have valid size values', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      expect(disk.totalSize).toBeGreaterThan(0);
      expect(disk.freeSize).toBeGreaterThanOrEqual(0);
      expect(disk.usedSize).toBeGreaterThanOrEqual(0);
      
      // Used + Free should approximately equal Total (allowing for some rounding)
      const calculatedTotal = disk.usedSize + disk.freeSize;
      const difference = Math.abs(calculatedTotal - disk.totalSize);
      const percentDiff = (difference / disk.totalSize) * 100;
      expect(percentDiff).toBeLessThan(1); // Less than 1% difference
    });
  });

  it('should have valid GB conversions', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      expect(disk.totalSizeGB).toBeGreaterThan(0);
      expect(disk.freeSizeGB).toBeGreaterThanOrEqual(0);
      expect(disk.usedSizeGB).toBeGreaterThanOrEqual(0);
      
      // Check GB conversion is reasonable (within 10% due to rounding)
      const expectedTotalGB = disk.totalSize / (1024 * 1024 * 1024);
      const difference = Math.abs(disk.totalSizeGB - expectedTotalGB);
      const percentDiff = (difference / expectedTotalGB) * 100;
      expect(percentDiff).toBeLessThan(10);
    });
  });

  it('should have valid usage percentage', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      expect(disk.usagePercent).toBeGreaterThanOrEqual(0);
      expect(disk.usagePercent).toBeLessThanOrEqual(100);
      
      // Verify usage percentage calculation
      const expectedPercent = Math.round((disk.usedSize / disk.totalSize) * 100);
      expect(disk.usagePercent).toBe(expectedPercent);
    });
  });

  it('should have valid mount points', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      expect(disk.mountPoint).toBeTruthy();
      expect(disk.mountPoint).toMatch(/^\//); // Should start with /
    });
  });

  it('should have valid file system types', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      expect(disk.fileSystem).toBeTruthy();
      expect(typeof disk.fileSystem).toBe('string');
      expect(disk.fileSystem.length).toBeGreaterThan(0);
    });
  });

  it('should have at least one root volume', async () => {
    const disks = await getDiskInfo();
    
    const rootVolume = disks.find(disk => disk.mountPoint === '/');
    expect(rootVolume).toBeDefined();
    expect(rootVolume?.totalSize).toBeGreaterThan(0);
  });

  it('should have valid BSD names', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      // BSD name might be empty for some volumes, but if present should be valid
      if (disk.bsdName) {
        expect(disk.bsdName).toMatch(/^disk\d+/); // Should start with "disk" followed by numbers
      }
    });
  });

  it('should have valid removable flag', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      expect(typeof disk.isRemovable).toBe('boolean');
    });
  });

  it('should return same data for sync version', async () => {
    const asyncDisks = await getDiskInfo();
    const syncDisks = getDiskInfoSync();
    
    expect(syncDisks.length).toBe(asyncDisks.length);
    
    // Compare first disk
    if (asyncDisks.length > 0 && syncDisks.length > 0) {
      expect(syncDisks[0].name).toBe(asyncDisks[0].name);
      expect(syncDisks[0].mountPoint).toBe(asyncDisks[0].mountPoint);
      expect(syncDisks[0].totalSize).toBe(asyncDisks[0].totalSize);
    }
  });

  it('should have consistent size calculations', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      // Total size should be greater than or equal to used size
      expect(disk.totalSize).toBeGreaterThanOrEqual(disk.usedSize);
      
      // Free size should be less than or equal to total size
      expect(disk.freeSize).toBeLessThanOrEqual(disk.totalSize);
      
      // Used size should be less than or equal to total size
      expect(disk.usedSize).toBeLessThanOrEqual(disk.totalSize);
    });
  });

  it('should have reasonable disk sizes', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      // Total size should be at least 1 MB and less than 100 TB
      expect(disk.totalSize).toBeGreaterThan(1024 * 1024); // > 1 MB
      expect(disk.totalSize).toBeLessThan(100 * 1024 * 1024 * 1024 * 1024); // < 100 TB
    });
  });

  it('should have valid disk names', async () => {
    const disks = await getDiskInfo();
    
    disks.forEach(disk => {
      expect(disk.name).toBeTruthy();
      expect(typeof disk.name).toBe('string');
      expect(disk.name.length).toBeGreaterThan(0);
    });
  });
});

