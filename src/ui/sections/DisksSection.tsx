import React from 'react';
import { Box, Text } from 'ink';
import type { DiskInfo } from '../../disk.js';

interface DisksSectionProps {
  disks: DiskInfo[];
  detailedMode?: boolean;
}

const createProgressBar = (percentage: number, width: number = 30): string => {
  const filled = Math.round((percentage / 100) * width);
  const empty = width - filled;
  return '█'.repeat(filled) + '░'.repeat(empty);
};

const getUsageColor = (percentage: number): string => {
  if (percentage >= 90) return 'red';
  if (percentage >= 75) return 'yellow';
  return 'green';
};

const formatBytes = (bytes: number): string => {
  const gb = bytes / 1024 ** 3;
  if (gb >= 1000) {
    return `${(gb / 1024).toFixed(1)} TB`;
  }
  return `${gb.toFixed(0)} GB`;
};

export const DisksSection: React.FC<DisksSectionProps> = ({ disks, detailedMode = false }) => {
  // In detailed mode, show all disks. Otherwise, filter to main disks only
  const displayDisks = detailedMode
    ? disks
    : disks.filter(disk => {
        // Show root volume
        if (disk.mountPoint === '/') return true;

        // Skip system volumes
        if (disk.mountPoint.includes('/System/Volumes')) return false;
        if (disk.mountPoint.includes('/private/var/vm')) return false;
        if (disk.mountPoint.includes('/Library/Developer/CoreSimulator')) return false;

        // Show removable disks that are not simulators
        if (disk.isRemovable && !disk.mountPoint.includes('Simulator')) return true;

        return false;
      });

  const mainDisks = displayDisks;

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="blue" paddingX={1}>
      <Text bold color="blue">
        💾 Disks
      </Text>
      <Box marginTop={1} flexDirection="column" paddingLeft={2}>
        {mainDisks.map((disk, index) => {
          const usageColor = getUsageColor(disk.usagePercent);
          const usedSize = formatBytes(disk.usedSize);
          const totalSize = formatBytes(disk.totalSize);

          return (
            <Box key={index} flexDirection="column" marginBottom={index < mainDisks.length - 1 ? 1 : 0}>
              <Box flexDirection="row">
                <Box width={20}>
                  <Text bold>{disk.name}</Text>
                </Box>
                <Text color={usageColor}>
                  {usedSize} / {totalSize} ({Math.round(disk.usagePercent)}%)
                </Text>
              </Box>
              <Box paddingLeft={2} flexDirection="row">
                <Text color={usageColor}>{createProgressBar(disk.usagePercent, 50)}</Text>
              </Box>
              {disk.bsdName && (
                <Box paddingLeft={2} flexDirection="row">
                  <Text dimColor>
                    {disk.mountPoint} • {disk.fileSystem} • {disk.bsdName}
                  </Text>
                </Box>
              )}
            </Box>
          );
        })}

        {mainDisks.length === 0 && <Text dimColor>No disks found</Text>}
      </Box>
    </Box>
  );
};
