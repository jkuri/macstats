import { describe, it, expect } from 'vitest';
import { getCPUUsage, getCPUUsageSync } from '../src/cpu.js';

describe('CPU Usage', () => {
  it('should return CPU usage information', async () => {
    const usage = await getCPUUsage();
    
    expect(usage).toBeDefined();
    expect(usage).toHaveProperty('userLoad');
    expect(usage).toHaveProperty('systemLoad');
    expect(usage).toHaveProperty('idleLoad');
    expect(usage).toHaveProperty('totalUsage');
    expect(usage).toHaveProperty('totalUsagePercent');
    expect(usage).toHaveProperty('loadAvg1');
    expect(usage).toHaveProperty('loadAvg5');
    expect(usage).toHaveProperty('loadAvg15');
  });

  it('should have valid load values (0.0 - 1.0)', async () => {
    const usage = await getCPUUsage();
    
    expect(usage.userLoad).toBeGreaterThanOrEqual(0);
    expect(usage.userLoad).toBeLessThanOrEqual(1);
    
    expect(usage.systemLoad).toBeGreaterThanOrEqual(0);
    expect(usage.systemLoad).toBeLessThanOrEqual(1);
    
    expect(usage.idleLoad).toBeGreaterThanOrEqual(0);
    expect(usage.idleLoad).toBeLessThanOrEqual(1);
    
    expect(usage.totalUsage).toBeGreaterThanOrEqual(0);
    expect(usage.totalUsage).toBeLessThanOrEqual(1);
  });

  it('should have valid usage percentage (0 - 100)', async () => {
    const usage = await getCPUUsage();
    
    expect(usage.totalUsagePercent).toBeGreaterThanOrEqual(0);
    expect(usage.totalUsagePercent).toBeLessThanOrEqual(100);
  });

  it('should have valid load averages', async () => {
    const usage = await getCPUUsage();
    
    expect(usage.loadAvg1).toBeGreaterThanOrEqual(0);
    expect(usage.loadAvg5).toBeGreaterThanOrEqual(0);
    expect(usage.loadAvg15).toBeGreaterThanOrEqual(0);
  });

  it('should have loads that sum to approximately 1.0', async () => {
    const usage = await getCPUUsage();
    
    const sum = usage.userLoad + usage.systemLoad + usage.idleLoad;
    // Allow small floating point error
    expect(sum).toBeGreaterThan(0.99);
    expect(sum).toBeLessThan(1.01);
  });

  it('should have totalUsage equal to 1 - idleLoad', async () => {
    const usage = await getCPUUsage();
    
    const expectedUsage = 1.0 - usage.idleLoad;
    // Allow small floating point error
    expect(Math.abs(usage.totalUsage - expectedUsage)).toBeLessThan(0.01);
  });

  it('should return same data for sync version', async () => {
    const asyncUsage = await getCPUUsage();
    const syncUsage = getCPUUsageSync();
    
    // Values should be very close (within 10% since CPU usage can change quickly)
    expect(Math.abs(asyncUsage.totalUsagePercent - syncUsage.totalUsagePercent)).toBeLessThan(10);
    
    // Load averages should be identical
    expect(asyncUsage.loadAvg1).toBe(syncUsage.loadAvg1);
    expect(asyncUsage.loadAvg5).toBe(syncUsage.loadAvg5);
    expect(asyncUsage.loadAvg15).toBe(syncUsage.loadAvg15);
  });

  it('should have reasonable load average values', async () => {
    const usage = await getCPUUsage();
    
    // Load averages should typically be less than 100 (unless system is heavily loaded)
    expect(usage.loadAvg1).toBeLessThan(100);
    expect(usage.loadAvg5).toBeLessThan(100);
    expect(usage.loadAvg15).toBeLessThan(100);
  });
});

