import { describe, it, expect } from 'vitest';
import { getRAMUsage, getRAMUsageSync } from '../src/memory.js';

describe('RAM Usage', () => {
  describe('getRAMUsage', () => {
    it('should return RAM usage information', async () => {
      const usage = await getRAMUsage();
      
      expect(usage).toBeDefined();
      expect(usage.total).toBeGreaterThan(0);
      expect(usage.totalGB).toBeGreaterThan(0);
      expect(usage.used).toBeGreaterThan(0);
      expect(usage.usedGB).toBeGreaterThan(0);
      expect(usage.free).toBeGreaterThan(0);
      expect(usage.freeGB).toBeGreaterThan(0);
      expect(usage.usagePercent).toBeGreaterThan(0);
      expect(usage.usagePercent).toBeLessThanOrEqual(100);
    });

    it('should have valid total memory', async () => {
      const usage = await getRAMUsage();
      
      // Total memory should be at least 4GB and less than 1TB
      expect(usage.total).toBeGreaterThan(4 * 1024 * 1024 * 1024);
      expect(usage.total).toBeLessThan(1024 * 1024 * 1024 * 1024);
      
      // GB conversion should be reasonable
      expect(usage.totalGB).toBeGreaterThan(4);
      expect(usage.totalGB).toBeLessThan(1024);
    });

    it('should have valid used and free memory', async () => {
      const usage = await getRAMUsage();
      
      // Used + free should approximately equal total (within 1GB tolerance for rounding)
      const sum = usage.used + usage.free;
      const tolerance = 1024 * 1024 * 1024; // 1GB
      expect(Math.abs(sum - usage.total)).toBeLessThan(tolerance);
      
      // Used should be less than total
      expect(usage.used).toBeLessThan(usage.total);
      
      // Free should be less than total
      expect(usage.free).toBeLessThan(usage.total);
    });

    it('should have valid memory breakdown', async () => {
      const usage = await getRAMUsage();
      
      // All memory types should be non-negative
      expect(usage.active).toBeGreaterThanOrEqual(0);
      expect(usage.inactive).toBeGreaterThanOrEqual(0);
      expect(usage.wired).toBeGreaterThanOrEqual(0);
      expect(usage.compressed).toBeGreaterThanOrEqual(0);
      expect(usage.app).toBeGreaterThanOrEqual(0);
      expect(usage.cache).toBeGreaterThanOrEqual(0);
      
      // GB conversions should be non-negative
      expect(usage.activeGB).toBeGreaterThanOrEqual(0);
      expect(usage.inactiveGB).toBeGreaterThanOrEqual(0);
      expect(usage.wiredGB).toBeGreaterThanOrEqual(0);
      expect(usage.compressedGB).toBeGreaterThanOrEqual(0);
      expect(usage.appGB).toBeGreaterThanOrEqual(0);
      expect(usage.cacheGB).toBeGreaterThanOrEqual(0);
    });

    it('should have valid swap information', async () => {
      const usage = await getRAMUsage();
      
      // Swap values should be non-negative
      expect(usage.swapTotal).toBeGreaterThanOrEqual(0);
      expect(usage.swapUsed).toBeGreaterThanOrEqual(0);
      expect(usage.swapFree).toBeGreaterThanOrEqual(0);
      
      // GB conversions should be non-negative
      expect(usage.swapTotalGB).toBeGreaterThanOrEqual(0);
      expect(usage.swapUsedGB).toBeGreaterThanOrEqual(0);
      expect(usage.swapFreeGB).toBeGreaterThanOrEqual(0);
      
      // If swap is configured, used + free should equal total
      if (usage.swapTotal > 0) {
        const swapSum = usage.swapUsed + usage.swapFree;
        const tolerance = 1024 * 1024; // 1MB tolerance
        expect(Math.abs(swapSum - usage.swapTotal)).toBeLessThan(tolerance);
      }
    });

    it('should have valid memory pressure', async () => {
      const usage = await getRAMUsage();
      
      // Pressure level should be 1 (normal), 2 (warning), or 4 (critical)
      expect([1, 2, 4]).toContain(usage.pressureLevel);
      
      // Pressure status should match level
      if (usage.pressureLevel === 1) {
        expect(usage.pressureStatus).toBe('Normal');
      } else if (usage.pressureLevel === 2) {
        expect(usage.pressureStatus).toBe('Warning');
      } else if (usage.pressureLevel === 4) {
        expect(usage.pressureStatus).toBe('Critical');
      }
    });

    it('should have valid usage percentage', async () => {
      const usage = await getRAMUsage();
      
      // Usage percentage should be between 0 and 100
      expect(usage.usagePercent).toBeGreaterThanOrEqual(0);
      expect(usage.usagePercent).toBeLessThanOrEqual(100);
      
      // Calculate expected percentage
      const expectedPercent = Math.round((usage.used / usage.total) * 100);
      expect(usage.usagePercent).toBe(expectedPercent);
    });
  });

  describe('getRAMUsageSync', () => {
    it('should return the same data as async version', async () => {
      const asyncUsage = await getRAMUsage();
      const syncUsage = getRAMUsageSync();
      
      // Total should be exactly the same
      expect(syncUsage.total).toBe(asyncUsage.total);
      expect(syncUsage.totalGB).toBe(asyncUsage.totalGB);
      
      // Used and free might differ slightly due to timing
      const tolerance = 100 * 1024 * 1024; // 100MB tolerance
      expect(Math.abs(syncUsage.used - asyncUsage.used)).toBeLessThan(tolerance);
      expect(Math.abs(syncUsage.free - asyncUsage.free)).toBeLessThan(tolerance);
      
      // Pressure level should be the same or close
      expect([asyncUsage.pressureLevel - 1, asyncUsage.pressureLevel, asyncUsage.pressureLevel + 1])
        .toContain(syncUsage.pressureLevel);
    });
  });
});

