import React from 'react';
import { Box, Text } from 'ink';
import type { SensorData } from '../../sensors.js';
import type { Fan } from '../../fan.js';

interface SensorsSectionProps {
  sensors: SensorData;
  fans: Fan;
}

interface SensorGroup {
  [category: string]: Array<{ name: string; value: number }>;
}

// Group sensors by category (same logic as cli.ts)
function groupSensors(sensors: Array<{ name: string; value: number }>): SensorGroup {
  const groups: SensorGroup = {};

  sensors.forEach(sensor => {
    let category = 'Other';

    if (sensor.name.includes('PMU tdie')) {
      category = 'CPU Die';
    } else if (sensor.name.includes('PMU tdev')) {
      category = 'Device';
    } else if (sensor.name.includes('gas gauge battery')) {
      category = 'Battery';
    } else if (sensor.name.includes('NAND')) {
      category = 'Storage';
    }

    if (!groups[category]) {
      groups[category] = [];
    }
    groups[category].push(sensor);
  });

  return groups;
}

// Calculate statistics for a group of sensors
function calculateStats(values: number[]): { min: number; max: number; avg: number } {
  const validValues = values.filter(v => v > 0);
  if (validValues.length === 0) {
    return { min: 0, max: 0, avg: 0 };
  }
  return {
    min: Math.min(...validValues),
    max: Math.max(...validValues),
    avg: validValues.reduce((a, b) => a + b, 0) / validValues.length
  };
}

const SensorsSectionComponent: React.FC<SensorsSectionProps> = ({ sensors, fans }) => {
  // Group temperature sensors by category
  const tempGroups = groupSensors(sensors.temperatures);

  // Sort categories in a logical order
  const categoryOrder = ['CPU Die', 'Device', 'Battery', 'Storage', 'Other'];
  const sortedCategories = Object.keys(tempGroups)
    .filter(cat => tempGroups[cat].filter(s => s.value > 0).length > 0) // Only show non-empty groups
    .sort((a, b) => {
      const indexA = categoryOrder.indexOf(a);
      const indexB = categoryOrder.indexOf(b);
      if (indexA === -1 && indexB === -1) return a.localeCompare(b);
      if (indexA === -1) return 1;
      if (indexB === -1) return -1;
      return indexA - indexB;
    });

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>Sensors & Temperatures</Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        {/* Temperature sensor groups with min/max/avg */}
        {sortedCategories.map((category, idx) => {
          const validSensors = tempGroups[category].filter(s => s.value > 0);
          const stats = calculateStats(validSensors.map(s => s.value));

          return (
            <Box key={idx} flexDirection="row">
              <Box width={11}>
                <Text dimColor>{category}:</Text>
              </Box>
              <Text bold>{stats.avg.toFixed(1)}°C</Text>
              <Text dimColor>
                {' '}
                ({stats.min.toFixed(1)}-{stats.max.toFixed(1)}°C)
              </Text>
            </Box>
          );
        })}

        {/* Fans - only show if RPM > 0 */}
        {Object.keys(fans).length > 0 && (
          <>
            {Object.entries(fans)
              .filter(([_, fan]) => fan.rpm > 0)
              .map(([index, fan]) => (
                <Box key={index} flexDirection="row">
                  <Box width={11}>
                    <Text dimColor>Fan #{index}:</Text>
                  </Box>
                  <Text>{fan.rpm} RPM</Text>
                  {fan.min > 0 && fan.max > 0 && (
                    <Text dimColor>
                      {' '}
                      ({fan.min}-{fan.max})
                    </Text>
                  )}
                </Box>
              ))}
          </>
        )}
      </Box>
    </Box>
  );
};

export const SensorsSection = React.memo(SensorsSectionComponent);
