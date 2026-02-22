import React from 'react';
import { Box, Text } from 'ink';
import type { GPU } from '../../gpu.js';
import { Sparkline } from '../components/Sparkline.js';

interface GPUHistory {
  timestamp: number;
  usage: number;
  temperature: number;
}

interface GPUSectionProps {
  gpu: GPU;
  history?: GPUHistory[];
  showHistory?: boolean;
}

const createProgressBar = (percentage: number, width: number = 15): string => {
  const clamped = Math.max(0, Math.min(100, percentage));
  const filled = Math.round((clamped / 100) * width);
  const empty = width - filled;
  return '█'.repeat(filled) + '░'.repeat(empty);
};

const GPUSectionComponent: React.FC<GPUSectionProps> = ({ gpu, history = [], showHistory = true }) => {
  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>GPU</Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Usage:</Text>
          </Box>
          <Text bold>{gpu.usage}%</Text>
          <Text dimColor> {createProgressBar(gpu.usage, 15)}</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Temp:</Text>
          </Box>
          <Text>{gpu.temperature}°C</Text>
        </Box>

        {showHistory && history.length > 0 && (
          <Box flexDirection="row">
            <Box width={11}>
              <Text dimColor>History:</Text>
            </Box>
            <Sparkline data={history.map(h => h.usage)} width={30} />
          </Box>
        )}
      </Box>
    </Box>
  );
};

export const GPUSection = React.memo(GPUSectionComponent);
