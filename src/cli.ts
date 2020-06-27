import { getBatteryData } from './battery';
import { getFanData } from './fan';
import { getCpuData } from './cpu';
import chalk from 'chalk';

Promise.resolve()
  .then(() => getCpuData())
  .then(data => {
    const cpu = [
      chalk.yellow('----- CPU -----'),
      `Temperature: ${chalk.green(String(data.temperature) + String.fromCharCode(0x00b0) + 'C')}`,
      ''
    ];
    console.log(cpu.join('\n'));
  })
  .then(() => getFanData())
  .then(data => {
    const fan = Object.keys(data).map(index => {
      return `Fan #${index}: ${chalk.green(String(data[index]))}`;
    });
    fan.unshift(chalk.yellow(`----- Fans -----`));
    fan.push('');
    console.log(fan.join('\n'));
  })
  .then(() => getBatteryData())
  .then(data => {
    let battery = [chalk.yellow(`----- Battery -----`)];

    if (data.battery_installed) {
      battery = battery.concat([
        `Battery Installed:  ` + chalk.green('✔'),
        `Is Charged:         ` + (data.is_charging ? chalk.red('✗') : chalk.green('✔')),
        `Capacity:           ` + chalk.green(String(data.percentage) + '%'),
        `Cycle Count:        ` + chalk.green(String(data.cycle_count) + ' (' + data.cycle_percentage + '%)'),
        `Design Cycle Count: ` + chalk.green(String(data.design_cycle_count)),
        `Current Charge:     ` + chalk.green(String(data.current_capacity) + ' mAh'),
        `Maximum Charge:     ` + chalk.green(String(data.max_capacity) + ' mAh'),
        `Design Capacity:    ` + chalk.green(String(data.design_capacity) + ' mAh'),
        `Time Remaining:     ` + chalk.green(String(data.time_remaining_formatted)),
        `Temperature:        ` + chalk.green(String(data.temperature) + String.fromCharCode(0x00b0) + 'C')
      ]);
    } else {
      battery = battery.concat([`Battery Installed:  ` + chalk.red('✗')]);
    }
    console.log(battery.join('\n'));
  });
