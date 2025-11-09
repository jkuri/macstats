import { describe, test, expect } from 'vitest';
import { getBatteryData } from '../src/battery';

describe('Battery Module', () => {
  test('should return battery data', async () => {
    const data = await getBatteryData();

    expect(data).toBeDefined();
    expect(typeof data).toBe('object');
    expect(data).toHaveProperty('battery_installed');
    expect(typeof data.battery_installed).toBe('boolean');

    if (data.battery_installed) {
      // If battery is installed, check all properties
      expect(data).toHaveProperty('is_charging');
      expect(data).toHaveProperty('percentage');
      expect(data).toHaveProperty('cycle_count');
      expect(data).toHaveProperty('cycle_percentage');
      expect(data).toHaveProperty('design_cycle_count');
      expect(data).toHaveProperty('current_capacity');
      expect(data).toHaveProperty('max_capacity');
      expect(data).toHaveProperty('design_capacity');
      expect(data).toHaveProperty('time_remaining');
      expect(data).toHaveProperty('time_remaining_formatted');
      expect(data).toHaveProperty('temperature');
      expect(data).toHaveProperty('voltage');
      expect(data).toHaveProperty('amperage');
      expect(data).toHaveProperty('power');

      // Check types
      expect(typeof data.is_charging).toBe('boolean');
      expect(typeof data.percentage).toBe('number');
      expect(typeof data.cycle_count).toBe('number');
      expect(typeof data.cycle_percentage).toBe('number');
      expect(typeof data.design_cycle_count).toBe('number');
      expect(typeof data.current_capacity).toBe('number');
      expect(typeof data.max_capacity).toBe('number');
      expect(typeof data.design_capacity).toBe('number');
      expect(typeof data.time_remaining).toBe('number');
      expect(typeof data.time_remaining_formatted).toBe('string');
      expect(typeof data.temperature).toBe('number');
      expect(typeof data.voltage).toBe('number');
      expect(typeof data.amperage).toBe('number');
      expect(typeof data.power).toBe('number');

      // Check reasonable values
      expect(data.percentage).toBeGreaterThanOrEqual(0);
      expect(data.percentage).toBeLessThanOrEqual(100);
      expect(data.cycle_count).toBeGreaterThanOrEqual(0);
      expect(data.design_cycle_count).toBeGreaterThan(0);
      expect(data.current_capacity).toBeGreaterThanOrEqual(0);
      expect(data.max_capacity).toBeGreaterThan(0);
      expect(data.design_capacity).toBeGreaterThan(0);
      expect(data.temperature).toBeGreaterThan(0);
      expect(data.temperature).toBeLessThan(100);
      expect(data.voltage).toBeGreaterThanOrEqual(0);
      expect(data.power).toBeGreaterThanOrEqual(0);
    }
  });
});

