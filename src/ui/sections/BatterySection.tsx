import React from 'react';
import { Box, Text } from 'ink';
import type { Battery } from '../../battery.js';

interface BatterySectionProps {
  battery: Battery;
}

const createProgressBar = (percentage: number, width: number = 20): string => {
  const filled = Math.round((percentage / 100) * width);
  const empty = width - filled;
  return '█'.repeat(filled) + '░'.repeat(empty);
};

const getBatteryIcon = (percentage: number, isCharging: boolean): string => {
  if (isCharging) return '🔌';
  if (percentage >= 90) return '🔋';
  if (percentage >= 60) return '🔋';
  if (percentage >= 30) return '🔋';
  if (percentage >= 10) return '🪫';
  return '🪫';
};

const BatterySectionComponent: React.FC<BatterySectionProps> = ({ battery }) => {
  // Determine status string
  let status = 'On Battery';
  if (battery.fully_charged) {
    status = 'Fully Charged';
  } else if (battery.is_charging) {
    status = 'Charging';
  } else if (battery.external_connected) {
    status = 'Plugged In - Not Charging';
  }

  const isCharging = battery.is_charging || battery.fully_charged;
  const icon = getBatteryIcon(battery.charge_percent, isCharging);

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>Battery</Text>
      <Box flexDirection="column" paddingLeft={2} marginTop={1}>
        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Status:</Text>
          </Box>
          <Text bold>
            {status} ({battery.charge_percent}%)
          </Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11} />
          <Text>{createProgressBar(battery.charge_percent, 30)}</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Health:</Text>
          </Box>
          <Text>
            {battery.health_percent}% ({battery.max_capacity} / {battery.design_capacity} mAh)
          </Text>
        </Box>

        {battery.time_remaining_formatted && battery.time_remaining_formatted !== 'Calculating...' && (
          <Box flexDirection="row">
            <Box width={11}>
              <Text dimColor>Time:</Text>
            </Box>
            <Text>{battery.time_remaining_formatted}</Text>
          </Box>
        )}

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Power:</Text>
          </Box>
          <Text>
            {battery.power.toFixed(1)}W ({(battery.voltage / 1000).toFixed(1)}V, {(battery.amperage / 1000).toFixed(2)}
            A)
          </Text>
        </Box>

        <Box flexDirection="row">
          <Box width={11}>
            <Text dimColor>Cycles:</Text>
          </Box>
          <Text>{battery.cycle_count}</Text>
          <Text dimColor> • </Text>
          <Text dimColor>Temp: </Text>
          <Text>{battery.temperature}°C</Text>
        </Box>
      </Box>
    </Box>
  );
};

export const BatterySection = React.memo(BatterySectionComponent);
