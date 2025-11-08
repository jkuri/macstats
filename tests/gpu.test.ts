/**
 * Tests for GPU module
 */

import { getGpuData } from '../src/gpu';

describe('GPU Module', () => {
  test('should return GPU data', async () => {
    const data = await getGpuData();
    
    expect(data).toBeDefined();
    expect(typeof data).toBe('object');
    expect(data).toHaveProperty('temperature');
    expect(data).toHaveProperty('power');
    expect(data).toHaveProperty('voltage');
    
    // Check types
    expect(typeof data.temperature).toBe('number');
    expect(typeof data.power).toBe('number');
    expect(typeof data.voltage).toBe('number');
    
    // Check reasonable values
    expect(data.temperature).toBeGreaterThanOrEqual(0);
    if (data.temperature > 0) {
      expect(data.temperature).toBeLessThan(100);
    }
    
    expect(data.power).toBeGreaterThanOrEqual(0);
    expect(data.voltage).toBeGreaterThanOrEqual(0);
  });
});

