import { describe, it, expect } from 'vitest';
import { getSystemData, getSystemDataSync } from '../src/system.js';

describe('System Information', () => {
  describe('getSystemData', () => {
    it('should return system information', async () => {
      const data = await getSystemData();

      expect(data).toBeDefined();
      expect(data.totalMemory).toBeGreaterThan(0);
      expect(data.totalMemoryGB).toBeGreaterThan(0);
      expect(data.uptime).toBeGreaterThan(0);
      expect(data.uptimeFormatted).toBeDefined();
      expect(data.osVersion).toBeDefined();
      expect(data.osCodename).toBeDefined();
      expect(data.modelName).toBeDefined();
      expect(data.hardwareModel).toBeDefined();
      expect(data.serialNumber).toBeDefined();
      // screenSize and releaseYear may be empty for unknown models
    });

    it('should have valid total memory', async () => {
      const data = await getSystemData();

      // Total memory should be at least 4GB and less than 1TB
      expect(data.totalMemory).toBeGreaterThan(4 * 1024 * 1024 * 1024);
      expect(data.totalMemory).toBeLessThan(1024 * 1024 * 1024 * 1024);

      // GB conversion should be reasonable
      expect(data.totalMemoryGB).toBeGreaterThan(4);
      expect(data.totalMemoryGB).toBeLessThan(1024);
    });

    it('should have valid uptime', async () => {
      const data = await getSystemData();

      // Uptime should be positive
      expect(data.uptime).toBeGreaterThan(0);

      // Uptime should be less than 1 year (in seconds)
      expect(data.uptime).toBeLessThan(365 * 24 * 60 * 60);

      // Formatted uptime should not be empty
      expect(data.uptimeFormatted).toBeTruthy();
      expect(data.uptimeFormatted.length).toBeGreaterThan(0);
    });

    it('should have valid OS version', async () => {
      const data = await getSystemData();

      // OS version should start with "macOS"
      expect(data.osVersion).toMatch(/^macOS/);

      // Should contain a version number
      expect(data.osVersion).toMatch(/\d+/);
    });

    it('should have valid model name', async () => {
      const data = await getSystemData();

      // Model name should not be empty
      expect(data.modelName).toBeTruthy();
      expect(data.modelName.length).toBeGreaterThan(0);

      // Should be either Intel or Apple Silicon
      const isIntel = data.modelName === 'Intel';
      const isAppleSilicon = data.modelName.startsWith('Apple M');
      expect(isIntel || isAppleSilicon).toBe(true);
    });

    it('should format uptime correctly', async () => {
      const data = await getSystemData();

      // Uptime format should contain time units
      const hasValidFormat = /\d+[dhm]/.test(data.uptimeFormatted);
      expect(hasValidFormat).toBe(true);
    });

    it('should have valid OS codename', async () => {
      const data = await getSystemData();

      // OS codename should not be empty
      expect(data.osCodename).toBeTruthy();
      expect(data.osCodename.length).toBeGreaterThan(0);

      // Should be a known codename or "Unknown"
      const knownCodenames = ['Tahoe', 'Sequoia', 'Sonoma', 'Ventura', 'Monterey', 'Big Sur', 'Catalina', 'Mojave', 'High Sierra', 'Unknown'];
      expect(knownCodenames).toContain(data.osCodename);
    });

    it('should have valid hardware model', async () => {
      const data = await getSystemData();

      // Hardware model should not be empty
      expect(data.hardwareModel).toBeTruthy();
      expect(data.hardwareModel.length).toBeGreaterThan(0);

      // Should match Mac pattern (e.g., Mac14,6)
      expect(data.hardwareModel).toMatch(/^Mac\d+,\d+$/);
    });

    it('should have valid serial number', async () => {
      const data = await getSystemData();

      // Serial number should not be empty
      expect(data.serialNumber).toBeTruthy();
      expect(data.serialNumber.length).toBeGreaterThan(0);

      // Serial number should be alphanumeric
      expect(data.serialNumber).toMatch(/^[A-Z0-9]+$/);
    });
  });

  describe('getSystemDataSync', () => {
    it('should return the same data as async version', async () => {
      const asyncData = await getSystemData();
      const syncData = getSystemDataSync();

      expect(syncData.totalMemory).toBe(asyncData.totalMemory);
      expect(syncData.totalMemoryGB).toBe(asyncData.totalMemoryGB);
      expect(syncData.osVersion).toBe(asyncData.osVersion);
      expect(syncData.modelName).toBe(asyncData.modelName);

      // Uptime might differ by a second or two
      expect(Math.abs(syncData.uptime - asyncData.uptime)).toBeLessThan(5);
    });
  });
});

