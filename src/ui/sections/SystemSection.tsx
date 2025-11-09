import React from 'react';
import { Box, Text } from 'ink';
import type { SystemInfo } from '../../system.js';

interface SystemSectionProps {
  data: SystemInfo;
}

export const SystemSection: React.FC<SystemSectionProps> = ({ data }) => {
  return (
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>🖥️ System Information</Text>
      <Box marginTop={1} flexDirection="column" paddingLeft={2}>
        <Box flexDirection="row">
          <Box width={12}>
            <Text dimColor>Model:</Text>
          </Box>
          <Text bold color="green">
            {data.modelName}
          </Text>
        </Box>

        {data.hardwareModel && (
          <Box flexDirection="row">
            <Box width={12}>
              <Text dimColor>Hardware:</Text>
            </Box>
            <Text color="green">
              {data.hardwareModel}
              {data.screenSize && data.releaseYear && ` (${data.screenSize}, ${data.releaseYear})`}
              {data.screenSize && !data.releaseYear && ` (${data.screenSize})`}
              {!data.screenSize && data.releaseYear && ` (${data.releaseYear})`}
            </Text>
          </Box>
        )}

        <Box flexDirection="row">
          <Box width={12}>
            <Text dimColor>OS:</Text>
          </Box>
          <Text color="green">
            {data.osVersion} <Text dimColor>({data.osCodename})</Text>
          </Text>
        </Box>

        {data.serialNumber && (
          <Box flexDirection="row">
            <Box width={12}>
              <Text dimColor>Serial:</Text>
            </Box>
            <Text color="green">{data.serialNumber}</Text>
          </Box>
        )}

        <Box flexDirection="row">
          <Box width={12}>
            <Text dimColor>RAM:</Text>
          </Box>
          <Text color="green">{data.totalMemoryGB} GB</Text>
        </Box>

        <Box flexDirection="row">
          <Box width={12}>
            <Text dimColor>Uptime:</Text>
          </Box>
          <Text color="green">{data.uptimeFormatted}</Text>
        </Box>
      </Box>
    </Box>
  );
};
