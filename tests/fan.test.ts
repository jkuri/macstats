import { describe, test, expect } from 'vitest';
import { getFanData } from '../src/fan';

describe('Fan Module', () => {
  test('should return fan data', async () => {
    const data = await getFanData();

    expect(data).toBeDefined();
    expect(typeof data).toBe('object');

    // Check structure
    Object.keys(data).forEach(key => {
      const fan = data[key];
      expect(fan).toHaveProperty('rpm');
      expect(fan).toHaveProperty('min');
      expect(fan).toHaveProperty('max');

      expect(typeof fan.rpm).toBe('number');
      expect(typeof fan.min).toBe('number');
      expect(typeof fan.max).toBe('number');

      expect(fan.rpm).toBeGreaterThanOrEqual(0);
      expect(fan.min).toBeGreaterThanOrEqual(0);
      expect(fan.max).toBeGreaterThanOrEqual(0);
    });
  });
});

