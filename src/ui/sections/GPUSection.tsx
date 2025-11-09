import React from 'react';
import { Box, Text } from 'ink';
import type { GPU } from '../../gpu.js';

interface GPUSectionProps {
  gpu: GPU;
}

const getTempColor = (temp: number): string => {
  if (temp >= 80) return 'red';
  if (temp >= 60) return 'yellow';
  return 'green';
};

const getPowerColor = (power: number): string => {
  if (power >= 30) return 'red';
  if (power >= 15) return 'yellow';
  return 'green';
};

export const GPUSection: React.FC<GPUSectionProps> = ({ gpu }) => {
  const tempColor = getTempColor(gpu.temperature);
  const powerColor = getPowerColor(gpu.power);

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>🎮 GPU</Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Temp:</Text>
          </Box>
          <Text color={tempColor}>{gpu.temperature}°C</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Power:</Text>
          </Box>
          <Text color={powerColor}>{gpu.power.toFixed(2)}W</Text>
          {gpu.voltage > 0 && <Text dimColor> ({gpu.voltage.toFixed(2)}V)</Text>}
        </Box>
      </Box>
    </Box>
  );
};
