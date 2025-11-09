import { describe, test, expect } from 'vitest';
import { getPowerData, getPowerDataSync } from '../src/power';

describe('Power Module', () => {
  test('should return power data asynchronously', async () => {
    const data = await getPowerData();

    expect(data).toBeDefined();
    expect(typeof data).toBe('object');
    expect(data).toHaveProperty('cpu');
    expect(data).toHaveProperty('gpu');
    expect(data).toHaveProperty('ane');
    expect(data).toHaveProperty('all');
    expect(data).toHaveProperty('system');
    expect(data).toHaveProperty('ram');
    expect(data).toHaveProperty('gpu_ram');

    // Check types
    expect(typeof data.cpu).toBe('number');
    expect(typeof data.gpu).toBe('number');
    expect(typeof data.ane).toBe('number');
    expect(typeof data.all).toBe('number');
    expect(typeof data.system).toBe('number');
    expect(typeof data.ram).toBe('number');
    expect(typeof data.gpu_ram).toBe('number');

    // Check reasonable values (power in watts should be non-negative)
    expect(data.cpu).toBeGreaterThanOrEqual(0);
    expect(data.gpu).toBeGreaterThanOrEqual(0);
    expect(data.ane).toBeGreaterThanOrEqual(0);
    expect(data.all).toBeGreaterThanOrEqual(0);
    expect(data.system).toBeGreaterThanOrEqual(0);
    expect(data.ram).toBeGreaterThanOrEqual(0);
    expect(data.gpu_ram).toBeGreaterThanOrEqual(0);
  });

  test('should return power data synchronously', () => {
    const data = getPowerDataSync();

    expect(data).toBeDefined();
    expect(typeof data).toBe('object');
    expect(data).toHaveProperty('cpu');
    expect(data).toHaveProperty('gpu');
    expect(data).toHaveProperty('ane');
    expect(data).toHaveProperty('all');
    expect(data).toHaveProperty('system');
    expect(data).toHaveProperty('ram');
    expect(data).toHaveProperty('gpu_ram');

    // Check types
    expect(typeof data.cpu).toBe('number');
    expect(typeof data.gpu).toBe('number');
    expect(typeof data.ane).toBe('number');
    expect(typeof data.all).toBe('number');
    expect(typeof data.system).toBe('number');
    expect(typeof data.ram).toBe('number');
    expect(typeof data.gpu_ram).toBe('number');

    // Check reasonable values (power in watts should be non-negative)
    expect(data.cpu).toBeGreaterThanOrEqual(0);
    expect(data.gpu).toBeGreaterThanOrEqual(0);
    expect(data.ane).toBeGreaterThanOrEqual(0);
    expect(data.all).toBeGreaterThanOrEqual(0);
    expect(data.system).toBeGreaterThanOrEqual(0);
    expect(data.ram).toBeGreaterThanOrEqual(0);
    expect(data.gpu_ram).toBeGreaterThanOrEqual(0);
  });

  test('CPU power should be less than or equal to system power when system power is available', async () => {
    const data = await getPowerData();

    // If system power is available, CPU power should not exceed it
    if (data.system > 0) {
      expect(data.cpu).toBeLessThanOrEqual(data.system);
    }
  });

  test('GPU power should be less than or equal to system power when system power is available', async () => {
    const data = await getPowerData();

    // If system power is available, GPU power should not exceed it
    if (data.system > 0) {
      expect(data.gpu).toBeLessThanOrEqual(data.system);
    }
  });

  test('all_power should equal cpu + gpu + ane', async () => {
    const data = await getPowerData();

    // all_power should be the sum of cpu, gpu, and ane power
    const expectedAll = data.cpu + data.gpu + data.ane;
    expect(data.all).toBeCloseTo(expectedAll, 2);
  });

  test('all_power should be less than or equal to system power when system power is available', async () => {
    const data = await getPowerData();

    // If system power is available, all_power should not exceed it
    if (data.system > 0) {
      expect(data.all).toBeLessThanOrEqual(data.system);
    }
  });

  test('async and sync power data should have same structure', async () => {
    const asyncData = await getPowerData();
    const syncData = getPowerDataSync();

    expect(asyncData).toHaveProperty('cpu');
    expect(asyncData).toHaveProperty('gpu');
    expect(asyncData).toHaveProperty('ane');
    expect(asyncData).toHaveProperty('all');
    expect(asyncData).toHaveProperty('system');
    expect(asyncData).toHaveProperty('ram');
    expect(asyncData).toHaveProperty('gpu_ram');

    expect(syncData).toHaveProperty('cpu');
    expect(syncData).toHaveProperty('gpu');
    expect(syncData).toHaveProperty('ane');
    expect(syncData).toHaveProperty('all');
    expect(syncData).toHaveProperty('system');
    expect(syncData).toHaveProperty('ram');
    expect(syncData).toHaveProperty('gpu_ram');
  });
});

