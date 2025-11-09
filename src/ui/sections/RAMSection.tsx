import React from 'react';
import { Box, Text } from 'ink';
import type { RAMUsage } from '../../memory.js';
import { Sparkline } from '../components/Sparkline.js';

interface RAMHistory {
  timestamp: number;
  usedPercentage: number;
}

interface RAMSectionProps {
  ram: RAMUsage;
  history: RAMHistory[];
  showHistory?: boolean;
}

const createProgressBar = (percentage: number, width: number = 20): string => {
  const filled = Math.round((percentage / 100) * width);
  const empty = width - filled;
  return '█'.repeat(filled) + '░'.repeat(empty);
};

const getUsageColor = (percentage: number): string => {
  if (percentage >= 90) return 'red';
  if (percentage >= 75) return 'yellow';
  return 'green';
};

const getPressureColor = (level: string): string => {
  if (level === 'critical') return 'red';
  if (level === 'warning') return 'yellow';
  return 'green';
};

export const RAMSection: React.FC<RAMSectionProps> = ({ ram, history, showHistory = true }) => {
  const usageColor = getUsageColor(ram.usagePercent);
  const pressureColor = getPressureColor(ram.pressureStatus);

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>🧠 RAM Usage</Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Used:</Text>
          </Box>
          <Text color={usageColor} bold>
            {ram.usedGB.toFixed(1)} GB / {ram.totalGB.toFixed(1)} GB ({Math.round(ram.usagePercent)}%)
          </Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11} />
          <Text dimColor>{createProgressBar(ram.usagePercent, 30)}</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Free:</Text>
          </Box>
          <Text color="green">{ram.freeGB.toFixed(1)} GB</Text>
          <Text dimColor> • </Text>
          <Text dimColor>Active: </Text>
          <Text color="blue">{ram.activeGB.toFixed(1)} GB</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Wired:</Text>
          </Box>
          <Text color="cyan">{ram.wiredGB.toFixed(1)} GB</Text>
          <Text dimColor> • </Text>
          <Text dimColor>Comp: </Text>
          <Text color="yellow">{ram.compressedGB.toFixed(1)} GB</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Pressure:</Text>
          </Box>
          <Text color={pressureColor} bold>
            {ram.pressureStatus}
          </Text>
        </Box>

        {showHistory && history.length > 0 && (
          <Box flexDirection="row">
            <Box width={11}>
              <Text dimColor>History:</Text>
            </Box>
            <Sparkline data={history.map(h => h.usedPercentage)} width={30} color={usageColor} />
          </Box>
        )}
      </Box>
    </Box>
  );
};
