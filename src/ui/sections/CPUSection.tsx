import React from 'react';
import { Box, Text } from 'ink';
import type { CPU, CPUUsage } from '../../cpu.js';
import { Sparkline } from '../components/Sparkline.js';

interface CPUHistory {
  timestamp: number;
  usage: number;
}

interface CPUSectionProps {
  cpu: CPU;
  cpuUsage: CPUUsage;
  history: CPUHistory[];
  showHistory?: boolean;
}

const createProgressBar = (percentage: number, width: number = 20): string => {
  const filled = Math.round((percentage / 100) * width);
  const empty = width - filled;
  return '█'.repeat(filled) + '░'.repeat(empty);
};

const CPUSectionComponent: React.FC<CPUSectionProps> = ({ cpu, cpuUsage, history, showHistory = true }) => {
  const totalUsagePercent = Math.round(cpuUsage.totalUsage * 100);
  const userPercent = Math.round(cpuUsage.userLoad * 100);
  const systemPercent = Math.round(cpuUsage.systemLoad * 100);

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>💻 CPU Usage</Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Total:</Text>
          </Box>
          <Text bold>{totalUsagePercent}%</Text>
          <Text dimColor> {createProgressBar(totalUsagePercent, 15)}</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>User/Sys:</Text>
          </Box>
          <Text>{userPercent}%</Text>
          <Text dimColor> / </Text>
          <Text>{systemPercent}%</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Load Avg:</Text>
          </Box>
          <Text>
            {cpuUsage.loadAvg1.toFixed(2)}, {cpuUsage.loadAvg5.toFixed(2)}, {cpuUsage.loadAvg15.toFixed(2)}
          </Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Temp:</Text>
          </Box>
          <Text>{cpu.temperature}°C</Text>
          {cpu.temperatureDie > 0 && cpu.temperatureDie !== cpu.temperature && (
            <Text dimColor> (Die: {cpu.temperatureDie}°C)</Text>
          )}
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Power:</Text>
          </Box>
          <Text>{cpu.power.toFixed(2)}W</Text>
          {cpu.voltage > 0 && <Text dimColor> ({cpu.voltage.toFixed(2)}V)</Text>}
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

export const CPUSection = React.memo(CPUSectionComponent);
