/**
 * Tests for CPU module
 */

import { getCpuData } from '../src/cpu';

describe('CPU Module', () => {
  test('should return CPU data', async () => {
    const data = await getCpuData();

    expect(data).toBeDefined();
    expect(typeof data).toBe('object');
    expect(data).toHaveProperty('temperature');
    expect(data).toHaveProperty('temperatureDie');
    expect(data).toHaveProperty('voltage');

    // Check types
    expect(typeof data.temperature).toBe('number');
    expect(typeof data.temperatureDie).toBe('number');
    expect(typeof data.voltage).toBe('number');

    // Check reasonable values
    expect(data.temperature).toBeGreaterThanOrEqual(0);
    if (data.temperature > 0) {
      expect(data.temperature).toBeLessThan(100);
    }

    expect(data.temperatureDie).toBeGreaterThanOrEqual(0);
    if (data.temperatureDie > 0) {
      expect(data.temperatureDie).toBeLessThan(100);
    }

    expect(data.voltage).toBeGreaterThanOrEqual(0);
  });
});

