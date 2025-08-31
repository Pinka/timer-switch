# Timer Switch

Arduino-based dual-timer switch for controlling mains power with independent ON/OFF cycles.

## Hardware

- Arduino board
- 16x2 LCD with I2C interface
- Relay module
- Push button
- LED indicator
- 2x 10kΩ potentiometers

## Connections

| Component     | Pin | Description          |
| ------------- | --- | -------------------- |
| ON Timer Pot  | A0  | ON duration (1-99s)  |
| OFF Timer Pot | A1  | OFF duration (1-99s) |
| Relay         | 9   | Mains power control  |
| Button        | 4   | Control input        |
| LED           | 13  | Status indicator     |
| LCD           | I2C | Display              |

## Features

- **Dual Timers**: Independent ON/OFF cycles (1-99 seconds each)
- **Calibration**: Potentiometer calibration for accurate timing
- **EEPROM Storage**: Settings saved permanently
- **LCD Display**: Real-time timer and countdown display
- **Manual Override**: Button toggles relay state

## Usage

### Setup

1. Hold button during startup to enter calibration
2. Follow LCD prompts to calibrate both potentiometers
3. System saves calibration automatically

### Operation

1. Turn potentiometers to set ON/OFF times
2. Press button to start timer cycle
3. Press button anytime to toggle relay
4. LCD shows current settings and countdown

## Safety

⚠️ **High Voltage Warning**: Controls mains power through relay.

- Use proper electrical enclosure
- Ensure relay matches voltage/current requirements
- Test with low voltage first
- Follow local electrical codes

## Applications

- Aquarium lighting cycles
- Garden irrigation
- HVAC fan control
- Equipment timing
- Home automation
