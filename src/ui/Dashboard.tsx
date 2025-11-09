import React, { useState, useEffect, useRef } from 'react';
import { Box, Text, useInput, useApp } from 'ink';
import { SystemSection } from './sections/SystemSection.js';
import { CPUSection } from './sections/CPUSection.js';
import { RAMSection } from './sections/RAMSection.js';
import { GPUSection } from './sections/GPUSection.js';
import { BatterySection } from './sections/BatterySection.js';
import { SensorsSection } from './sections/SensorsSection.js';
import { DisksSection } from './sections/DisksSection.js';
import { getSystemData } from '../system.js';
import { getCpuData, getCPUUsage } from '../cpu.js';
import { getGpuData } from '../gpu.js';
import { getRAMUsage } from '../memory.js';
import { getBatteryData } from '../battery.js';
import { getFanData } from '../fan.js';
import { getDiskInfo } from '../disk.js';
import { getSensorData } from '../sensors.js';

interface DashboardData {
  system: Awaited<ReturnType<typeof getSystemData>>;
  cpu: Awaited<ReturnType<typeof getCpuData>>;
  cpuUsage: Awaited<ReturnType<typeof getCPUUsage>>;
  gpu: Awaited<ReturnType<typeof getGpuData>>;
  ram: Awaited<ReturnType<typeof getRAMUsage>>;
  battery: Awaited<ReturnType<typeof getBatteryData>> | null;
  sensors: Awaited<ReturnType<typeof getSensorData>>;
  fans: Awaited<ReturnType<typeof getFanData>>;
  disks: Awaited<ReturnType<typeof getDiskInfo>>;
}

interface CPUHistory {
  timestamp: number;
  usage: number;
}

interface RAMHistory {
  timestamp: number;
  usedPercentage: number;
}

interface DashboardProps {
  refreshInterval?: number;
  detailedMode?: boolean;
  exitAfterRender?: boolean;
}

export const Dashboard: React.FC<DashboardProps> = ({
  refreshInterval = 2000,
  detailedMode = false,
  exitAfterRender = false
}) => {
  const { exit } = useApp();
  const [data, setData] = useState<DashboardData | null>(null);
  const [cpuHistory, setCpuHistory] = useState<CPUHistory[]>([]);
  const [ramHistory, setRamHistory] = useState<RAMHistory[]>([]);
  const [lastUpdate, setLastUpdate] = useState<Date>(new Date());
  const hasExitedRef = useRef(false);

  // Handle keyboard input
  useInput((input, key) => {
    if (input === 'q' || (key.ctrl && input === 'c')) {
      exit();
    }
    if (input === 'r') {
      // Force refresh
      fetchData();
    }
  });

  const fetchData = async () => {
    try {
      const [system, cpu, cpuUsage, gpu, ram, battery, sensors, fans, disks] = await Promise.all([
        getSystemData(),
        getCpuData(),
        getCPUUsage(),
        getGpuData(),
        getRAMUsage(),
        getBatteryData().catch(() => null),
        getSensorData(),
        getFanData(),
        getDiskInfo()
      ]);

      setData({
        system,
        cpu,
        cpuUsage,
        gpu,
        ram,
        battery,
        sensors,
        fans,
        disks
      });

      // Update CPU history (keep last 60 data points)
      setCpuHistory(prev => {
        const newHistory = [
          ...prev,
          {
            timestamp: Date.now(),
            usage: cpuUsage.totalUsagePercent
          }
        ];
        return newHistory.slice(-60);
      });

      // Update RAM history (keep last 60 data points)
      setRamHistory(prev => {
        const newHistory = [
          ...prev,
          {
            timestamp: Date.now(),
            usedPercentage: ram.usagePercent
          }
        ];
        return newHistory.slice(-60);
      });

      setLastUpdate(new Date());
    } catch (error) {
      console.error('Error fetching data:', error);
    }
  };

  // Initial fetch and periodic updates
  useEffect(() => {
    fetchData();
    if (refreshInterval > 0) {
      const interval = setInterval(fetchData, refreshInterval);
      return () => clearInterval(interval);
    }
  }, [refreshInterval]);

  // Exit after first render if requested (for non-watch mode)
  useEffect(() => {
    if (exitAfterRender && data && !hasExitedRef.current) {
      hasExitedRef.current = true;
      setTimeout(() => exit(), 10);
    }
  }, [data, exitAfterRender, exit]);

  if (!data) {
    return (
      <Box flexDirection="column" padding={1}>
        <Text>Loading...</Text>
      </Box>
    );
  }

  return (
    <Box flexDirection="column">
      {/* System Information - Full width */}
      <SystemSection data={data.system} />

      <Box marginTop={1} />

      {/* CPU, GPU, and RAM - Three columns */}
      <Box flexDirection="row">
        <Box width="33%" paddingRight={1} flexDirection="column">
          <CPUSection cpu={data.cpu} cpuUsage={data.cpuUsage} history={cpuHistory} showHistory={refreshInterval > 0} />
        </Box>
        <Box width="33%" paddingRight={1} paddingLeft={1} flexDirection="column">
          <GPUSection gpu={data.gpu} />
        </Box>
        <Box width="34%" paddingLeft={1} flexDirection="column">
          <RAMSection ram={data.ram} history={ramHistory} showHistory={refreshInterval > 0} />
        </Box>
      </Box>

      <Box marginTop={1} />

      {/* Battery and Sensors - Side by side */}
      <Box flexDirection="row">
        <Box width="50%" paddingRight={1} flexDirection="column">
          {data.battery && <BatterySection battery={data.battery} />}
        </Box>
        <Box width="50%" paddingLeft={1} flexDirection="column">
          <SensorsSection sensors={data.sensors} fans={data.fans} />
        </Box>
      </Box>

      <Box marginTop={1} />

      {/* Disks - Full width */}
      <DisksSection disks={data.disks} detailedMode={detailedMode} />

      {/* Footer - only in watch mode */}
      {refreshInterval > 0 && (
        <Box marginTop={1} borderStyle="round" borderColor="gray" paddingX={1}>
          <Text dimColor>
            Press{' '}
            <Text bold color="yellow">
              q
            </Text>{' '}
            to quit |{' '}
            <Text bold color="yellow">
              r
            </Text>{' '}
            to refresh | Last update: {lastUpdate.toLocaleTimeString()}
          </Text>
        </Box>
      )}
    </Box>
  );
};
