import React from 'react';
import { Box, Text } from 'ink';
import type { DiskInfo } from '../../disk.js';

interface DisksSectionProps {
  disks: DiskInfo[];
  detailedMode?: boolean;
}

const createProgressBar = (percentage: number, width: number = 30): string => {
  const clamped = Math.max(0, Math.min(100, percentage));
  const filled = Math.round((clamped / 100) * width);
  const empty = width - filled;
  return '█'.repeat(filled) + '░'.repeat(empty);
};

const formatBytes = (bytes: number): string => {
  const gb = bytes / 1024 ** 3;
  if (gb >= 1000) {
    return `${(gb / 1024).toFixed(1)} TB`;
  }
  return `${gb.toFixed(0)} GB`;
};

const DisksSectionComponent: React.FC<DisksSectionProps> = ({ disks, detailedMode = false }) => {
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
    <Box flexDirection="column" borderStyle="round" borderColor="gray" paddingX={1} flexGrow={1}>
      <Text bold>Disks</Text>
      <Box marginTop={1} flexDirection="column" paddingLeft={2}>
        {mainDisks.map((disk, index) => {
          const usedSize = formatBytes(disk.usedSize);
          const totalSize = formatBytes(disk.totalSize);

          return (
            <Box key={index} flexDirection="row" marginBottom={index < mainDisks.length - 1 ? 1 : 0}>
              <Box width={18}>
                <Text bold>{disk.name}</Text>
              </Box>
              <Box width={20}>
                <Text>
                  {usedSize} / {totalSize}
                </Text>
              </Box>
              <Box width={7}>
                <Text>({Math.round(disk.usagePercent)}%)</Text>
              </Box>
              <Box width={25}>
                <Text>{createProgressBar(disk.usagePercent, 25)}</Text>
              </Box>
              {disk.bsdName && (
                <Text>
                  {' '}
                  {disk.mountPoint} • {disk.fileSystem} • {disk.bsdName}
                </Text>
              )}
            </Box>
          );
        })}

        {mainDisks.length === 0 && <Text dimColor>No disks found</Text>}
      </Box>
    </Box>
  );
};

export const DisksSection = React.memo(DisksSectionComponent);
