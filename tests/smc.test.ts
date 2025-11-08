/**
 * Tests for SMC native module
 */

describe('SMC Native Module', () => {
  let smc: any;

  beforeAll(() => {
    smc = require('../build/Release/smc.node');
  });

  describe('Model Detection', () => {
    test('should return a valid model name', () => {
      const modelName = smc.getModelInfo();
      expect(modelName).toBeDefined();
      expect(typeof modelName).toBe('string');
      expect(modelName.length).toBeGreaterThan(0);

      // Should be one of the known models
      const validModels = [
        'Intel',
        'Apple M1', 'Apple M1 Pro', 'Apple M1 Max', 'Apple M1 Ultra',
        'Apple M2', 'Apple M2 Pro', 'Apple M2 Max', 'Apple M2 Ultra',
        'Apple M3', 'Apple M3 Pro', 'Apple M3 Max', 'Apple M3 Ultra',
        'Apple M4', 'Apple M4 Pro', 'Apple M4 Max', 'Apple M4 Ultra',
        'Unknown'
      ];
      expect(validModels).toContain(modelName);
    });
  });

  describe('Temperature Sensors', () => {
    test('should return CPU temperature', () => {
      const temp = smc.temperature();
      expect(typeof temp).toBe('number');
      expect(temp).toBeGreaterThanOrEqual(0);

      // Temperature should be reasonable (0-100°C)
      if (temp > 0) {
        expect(temp).toBeLessThan(100);
      }
    });

    test('should return all temperature sensors (IOKit)', () => {
      const sensors = smc.getAllTemperatureSensors();
      expect(Array.isArray(sensors)).toBe(true);

      // On Apple Silicon, should have multiple sensors
      const modelName = smc.getModelInfo();
      if (modelName.includes('Apple M')) {
        expect(sensors.length).toBeGreaterThan(0);

        // Check sensor structure
        sensors.forEach((sensor: any) => {
          expect(sensor).toHaveProperty('name');
          expect(sensor).toHaveProperty('value');
          expect(typeof sensor.name).toBe('string');
          expect(typeof sensor.value).toBe('number');
        });
      }
    });

    test('should return GPU temperature', () => {
      const temp = smc.gpuTemperature();
      expect(typeof temp).toBe('number');
      expect(temp).toBeGreaterThanOrEqual(0);
    });

    test('should return CPU die temperature', () => {
      const temp = smc.cpuTemperatureDie();
      expect(typeof temp).toBe('number');
      expect(temp).toBeGreaterThanOrEqual(0);
    });
  });

  describe('Voltage Sensors', () => {
    test('should return all voltage sensors (IOKit)', () => {
      const sensors = smc.getAllVoltageSensors();
      expect(Array.isArray(sensors)).toBe(true);

      // On Apple Silicon, should have multiple sensors
      const modelName = smc.getModelInfo();
      if (modelName.includes('Apple M')) {
        expect(sensors.length).toBeGreaterThan(0);

        // Check sensor structure
        sensors.forEach((sensor: any) => {
          expect(sensor).toHaveProperty('name');
          expect(sensor).toHaveProperty('value');
          expect(typeof sensor.name).toBe('string');
          expect(typeof sensor.value).toBe('number');
        });
      }
    });

    test('should return CPU voltage', () => {
      const voltage = smc.cpuVoltage();
      expect(typeof voltage).toBe('number');
      expect(voltage).toBeGreaterThanOrEqual(0);
    });

    test('should return GPU voltage', () => {
      const voltage = smc.gpuVoltage();
      expect(typeof voltage).toBe('number');
      expect(voltage).toBeGreaterThanOrEqual(0);
    });

    test('should return memory voltage', () => {
      const voltage = smc.memoryVoltage();
      expect(typeof voltage).toBe('number');
      expect(voltage).toBeGreaterThanOrEqual(0);
    });
  });

  describe('Current Sensors', () => {
    test('should return all current sensors (IOKit)', () => {
      const sensors = smc.getAllCurrentSensors();
      expect(Array.isArray(sensors)).toBe(true);

      // On Apple Silicon, should have multiple sensors
      const modelName = smc.getModelInfo();
      if (modelName.includes('Apple M')) {
        expect(sensors.length).toBeGreaterThan(0);

        // Check sensor structure
        sensors.forEach((sensor: any) => {
          expect(sensor).toHaveProperty('name');
          expect(sensor).toHaveProperty('value');
          expect(typeof sensor.name).toBe('string');
          expect(typeof sensor.value).toBe('number');
        });
      }
    });
  });

  describe('Power Sensors', () => {
    test('should return CPU power', () => {
      const power = smc.cpuPower();
      expect(typeof power).toBe('number');
      expect(power).toBeGreaterThanOrEqual(0);
    });

    test('should return GPU power', () => {
      const power = smc.gpuPower();
      expect(typeof power).toBe('number');
      expect(power).toBeGreaterThanOrEqual(0);
    });

    test('should return system power', () => {
      const power = smc.systemPower();
      expect(typeof power).toBe('number');
      expect(power).toBeGreaterThanOrEqual(0);
    });
  });

  describe('Fan Sensors', () => {
    test('should return fan count', () => {
      const fanCount = smc.fans();
      expect(typeof fanCount).toBe('number');
      expect(fanCount).toBeGreaterThanOrEqual(0);
    });

    test('should return fan RPM', () => {
      const rpm = smc.fanRpm(0);
      expect(typeof rpm).toBe('number');
      expect(rpm).toBeGreaterThanOrEqual(0);
    });

    test('should return fan min speed', () => {
      const min = smc.fanMin(0);
      expect(typeof min).toBe('number');
      expect(min).toBeGreaterThanOrEqual(0);
    });

    test('should return fan max speed', () => {
      const max = smc.fanMax(0);
      expect(typeof max).toBe('number');
      expect(max).toBeGreaterThanOrEqual(0);
    });
  });
});

