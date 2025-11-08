import chalk from 'chalk';

// Check for flags
const args = process.argv.slice(2);
const detailedMode = args.includes('--detailed') || args.includes('-d');
const watchMode = args.includes('--watch') || args.includes('-w');
const helpMode = args.includes('--help') || args.includes('-h');

if (helpMode) {
  console.log(chalk.cyan.bold('macstats - macOS System Monitor'));
  console.log('');
  console.log('Usage:');
  console.log('  macstats              Show system stats (single snapshot)');
  console.log('  macstats --detailed   Show all disks including system volumes');
  console.log('  macstats -d           Show all disks (short)');
  console.log('  macstats --watch      Launch real-time dashboard with live updates');
  console.log('  macstats -w           Launch real-time dashboard (short)');
  console.log('  macstats -w -d        Launch real-time dashboard with all disks');
  console.log('  macstats --help       Show this help message');
  console.log('');
  console.log('Controls (in watch mode):');
  console.log('  q or Ctrl+C           Exit');
  console.log('  r                     Force refresh');
  console.log('');
  process.exit(0);
}

async function main(): Promise<void> {
  try {
    const { render } = await import('ink');
    const { Dashboard } = await import('./ui/Dashboard.js');
    const React = await import('react');

    const { waitUntilExit } = render(
      React.createElement(Dashboard, {
        refreshInterval: watchMode ? 2000 : 0,
        detailedMode: detailedMode,
        exitAfterRender: !watchMode
      }),
      {
        // Disable patchConsole in non-watch mode to prevent alternate screen buffer
        patchConsole: watchMode
      }
    );

    // In non-watch mode, wait for the component to exit
    if (!watchMode) {
      await waitUntilExit();
    }
  } catch (err) {
    const error = err as Error;
    console.error(chalk.red('Error:'), error.message);
    process.exit(1);
  }
}

// Run the main function
main();
