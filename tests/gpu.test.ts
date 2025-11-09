import { describe, test, expect } from 'vitest';
import { getGpuData } from '../src/gpu';

describe('GPU Module', () => {
  test('should return GPU data', async () => {
    const data = await getGpuData();

    expect(data).toBeDefined();
    expect(typeof data).toBe('object');
    expect(data).toHaveProperty('temperature');
    expect(data).toHaveProperty('voltage');
    expect(data).toHaveProperty('usage');

    // Check types
    expect(typeof data.temperature).toBe('number');
    expect(typeof data.voltage).toBe('number');
    expect(typeof data.usage).toBe('number');

    // Check reasonable values
    expect(data.temperature).toBeGreaterThanOrEqual(0);
    if (data.temperature > 0) {
      expect(data.temperature).toBeLessThan(100);
    }

    expect(data.voltage).toBeGreaterThanOrEqual(0);
    expect(data.usage).toBeGreaterThanOrEqual(0);
    expect(data.usage).toBeLessThanOrEqual(100);
  });
});

