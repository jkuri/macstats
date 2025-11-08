import React from 'react';
import { Text } from 'ink';

/**
 * Props for the Sparkline component
 */
export interface SparklineProps {
  /**
   * Array of numeric values to visualize (0-100 range expected)
   */
  data: number[];

  /**
   * Width of the sparkline in characters
   */
  width?: number;

  /**
   * Color for the sparkline
   */
  color?: string;
}

/**
 * A compact sparkline component for visualizing numeric trends in terminal applications.
 *
 * Uses Unicode block characters to render sparklines, making them perfect for CLI applications
 * and terminal-based dashboards.
 *
 * @example
 * ```tsx
 * // Basic usage
 * <Sparkline data={[10, 30, 20, 50, 40]} width={20} />
 * ```
 *
 * @param props - Component properties
 * @returns React element containing the rendered sparkline, or null for empty data
 */
export function Sparkline(props: SparklineProps): React.ReactElement | null {
  const { data, width = 30, color } = props;

  // Handle empty or invalid data
  if (!data || data.length === 0) {
    return null;
  }

  // Filter out non-finite values
  const validData = data.filter(Number.isFinite);
  if (validData.length === 0) {
    return null;
  }

  // Generate sparkline symbols
  const symbols = valuesToSymbols(validData, width);

  return <Text color={color}>{symbols}</Text>;
}

/**
 * Converts array of values (0-100) to sparkline symbols
 * @param data - Array of numeric values (0-100 range)
 * @param targetWidth - Desired width in characters
 * @returns String of sparkline symbols
 */
function valuesToSymbols(data: number[], targetWidth: number): string {
  // Block characters from empty to full (9 levels)
  const blocks = ['░', '▁', '▂', '▃', '▄', '▅', '▆', '▇', '█'];

  // Take last 'targetWidth' data points and reverse so oldest is on left
  const recentData = data.slice(-targetWidth).reverse();

  // Pad with empty blocks if not enough data (on the right for future data)
  const padding = targetWidth - recentData.length;
  const paddingStr = padding > 0 ? '░'.repeat(padding) : '';

  // Convert each value to a symbol
  const symbols = recentData
    .map(value => {
      // Normalize to 0-1 range (data is expected to be 0-100)
      const normalized = Math.max(0, Math.min(value / 100, 1));

      // Map to block index (0-8)
      const index = Math.min(Math.floor(normalized * (blocks.length - 1)), blocks.length - 1);

      return blocks[index];
    })
    .join('');

  return symbols + paddingStr;
}
