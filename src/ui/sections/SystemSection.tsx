import React from 'react';
import { Box, Text } from 'ink';
import type { SystemInfo } from '../../system.js';

interface SystemSectionProps {
  data: SystemInfo;
}

const SystemSectionComponent: React.FC<SystemSectionProps> = ({ data }) => {
  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>🖥️ System Information</Text>
      <Box marginTop={1} flexDirection="row" paddingLeft={2} flexWrap="wrap" gap={3}>
        <Box>
          <Text>
            <Text dimColor>Model: </Text>
            <Text bold>{data.modelName}</Text>
          </Text>
        </Box>

        {data.hardwareModel && (
          <Box>
            <Text>
              <Text dimColor>Hardware: </Text>
              <Text>
                {data.hardwareModel}
                {data.screenSize && data.releaseYear && ` (${data.screenSize}, ${data.releaseYear})`}
                {data.screenSize && !data.releaseYear && ` (${data.screenSize})`}
                {!data.screenSize && data.releaseYear && ` (${data.releaseYear})`}
              </Text>
            </Text>
          </Box>
        )}

        <Box>
          <Text>
            <Text dimColor>OS: </Text>
            <Text>
              {data.osVersion} <Text>({data.osCodename})</Text>
            </Text>
          </Text>
        </Box>

        {data.serialNumber && (
          <Box>
            <Text>
              <Text dimColor>Serial: </Text>
              <Text>{data.serialNumber}</Text>
            </Text>
          </Box>
        )}

        <Box>
          <Text>
            <Text dimColor>RAM: </Text>
            <Text>{data.totalMemoryGB} GB</Text>
          </Text>
        </Box>

        <Box>
          <Text>
            <Text dimColor>Uptime: </Text>
            <Text>{data.uptimeFormatted}</Text>
          </Text>
        </Box>
      </Box>
    </Box>
  );
};

export const SystemSection = React.memo(SystemSectionComponent);
