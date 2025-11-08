import React from 'react';
import { Box, Text } from 'ink';
import type { SensorData } from '../../sensors.js';
import type { Fan } from '../../fan.js';

interface SensorsSectionProps {
  sensors: SensorData;
  fans: Fan;
}

const createTempBar = (temp: number, maxTemp: number = 100): string => {
  const width = 10;
  const filled = Math.round((temp / maxTemp) * width);
  const empty = width - filled;
  return '█'.repeat(filled) + '░'.repeat(empty);
};

const getTempColor = (temp: number): string => {
  if (temp >= 80) return 'red';
  if (temp >= 60) return 'yellow';
  return 'green';
};

export const SensorsSection: React.FC<SensorsSectionProps> = ({ sensors, fans }) => {
  // Get battery temperature
  const batteryTemp = sensors.temperatures.find(
    (t: { name: string; value: number }) =>
      t.name.toLowerCase().includes('battery') || t.name.toLowerCase().includes('tb0')
  );

  // Get top 3 hottest sensors for display
  const topTemps = [...sensors.temperatures].sort((a, b) => b.value - a.value).slice(0, 3);

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="yellow" paddingX={1}>
      <Text bold color="yellow">
        🌡️ Sensors & Temperatures
      </Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        {/* Top 3 hottest sensors - more compact */}
        {topTemps.slice(0, 3).map((temp, idx) => (
          <Box key={idx} flexDirection="row">
            <Box width={11}>
              <Text dimColor>{idx === 0 ? 'Hottest:' : ''}</Text>
            </Box>
            <Text color={getTempColor(temp.value)} bold={idx === 0}>
              {temp.value.toFixed(1)}°C
            </Text>
            <Text dimColor> {createTempBar(temp.value)}</Text>
          </Box>
        ))}

        {batteryTemp && (
          <Box flexDirection="row">
            <Box width={11}>
              <Text dimColor>Battery:</Text>
            </Box>
            <Text color={getTempColor(batteryTemp.value)}>{batteryTemp.value.toFixed(1)}°C</Text>
            <Text dimColor> {createTempBar(batteryTemp.value)}</Text>
          </Box>
        )}

        {/* Fans */}
        {Object.keys(fans).length > 0 && (
          <>
            {Object.entries(fans).map(([index, fan]) => (
              <Box key={index} flexDirection="row">
                <Box width={11}>
                  <Text dimColor>Fan #{index}:</Text>
                </Box>
                <Text color="cyan">{fan.rpm} RPM</Text>
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

        {/* Show total sensor count */}
        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Sensors:</Text>
          </Box>
          <Text color="white">
            {sensors.temperatures.length} temp, {sensors.voltages.length} volt, {sensors.currents.length} curr
          </Text>
        </Box>
      </Box>
    </Box>
  );
};
