import React from 'react';
import { Box, Text } from 'ink';
import type { Power } from '../../power.js';

interface PowerSectionProps {
  power: Power;
}

const PowerSectionComponent: React.FC<PowerSectionProps> = ({ power }) => {
  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>Power Consumption</Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>CPU:</Text>
          </Box>
          <Text>{power.cpu.toFixed(2)}W</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>GPU:</Text>
          </Box>
          <Text>{power.gpu.toFixed(2)}W</Text>
        </Box>

        {power.ane > 0 && (
          <Box flexDirection="row">
            <Box width={11}>
              <Text dimColor>ANE:</Text>
            </Box>
            <Text>{power.ane.toFixed(2)}W</Text>
          </Box>
        )}

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>RAM:</Text>
          </Box>
          <Text>{power.ram.toFixed(2)}W</Text>
        </Box>

        {power.gpu_ram > 0 && (
          <Box flexDirection="row">
            <Box width={11}>
              <Text dimColor>GPU RAM:</Text>
            </Box>
            <Text>{power.gpu_ram.toFixed(2)}W</Text>
          </Box>
        )}

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Combined:</Text>
          </Box>
          <Text bold>{power.all.toFixed(2)}W</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>System:</Text>
          </Box>
          <Text bold>{power.system.toFixed(2)}W</Text>
        </Box>
      </Box>
    </Box>
  );
};

export const PowerSection = React.memo(PowerSectionComponent);
