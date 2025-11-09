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

export const RAMSection: React.FC<RAMSectionProps> = ({ ram, history, showHistory = true }) => {
  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>🧠 RAM Usage</Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Used:</Text>
          </Box>
          <Text bold>
            {ram.usedGB.toFixed(1)} GB / {ram.totalGB.toFixed(1)} GB ({Math.round(ram.usagePercent)}%)
          </Text>
          <Text dimColor> {createProgressBar(ram.usagePercent, 15)}</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Free:</Text>
          </Box>
          <Text>{ram.freeGB.toFixed(1)} GB</Text>
          <Text dimColor> • </Text>
          <Text dimColor>Active: </Text>
          <Text>{ram.activeGB.toFixed(1)} GB</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Wired:</Text>
          </Box>
          <Text>{ram.wiredGB.toFixed(1)} GB</Text>
          <Text dimColor> • </Text>
          <Text dimColor>Comp: </Text>
          <Text>{ram.compressedGB.toFixed(1)} GB</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Pressure:</Text>
          </Box>
          <Text bold>{ram.pressureStatus}</Text>
        </Box>

        {showHistory && history.length > 0 && (
          <Box flexDirection="row">
            <Box width={11}>
              <Text dimColor>History:</Text>
            </Box>
            <Sparkline data={history.map(h => h.usedPercentage)} width={30} />
          </Box>
        )}
      </Box>
    </Box>
  );
};
