# Timer Switch

Arduino-based timer switch with potentiometer control and LCD display.

## Hardware

- Arduino board
- 16x2 LCD with I2C interface
- Relay module
- Push button
- LED indicator
- 10kΩ potentiometer

## Connections

- **Potentiometer**: A0 (shared for ON/OFF timing)
- **Relay**: Pin 9
- **Button**: Pin 4 (with pull-up)
- **LED**: Pin 13
- **LCD**: I2C pins

## Features

- **Calibrated Control**: Potentiometer calibration for accurate timing
- **EEPROM Storage**: Calibration values saved permanently
- **Timer Range**: 1-99 seconds adjustable via potentiometer
- **Visual Display**: LCD shows current timers and progress
- **Manual Override**: Button toggles relay state

## Usage

1. **First Time**: Hold button during startup to enter calibration mode
2. **Calibration**: Follow LCD prompts to set potentiometer min/max values
3. **Normal Operation**: Turn potentiometer to adjust timing (1-99 seconds)
4. **Start**: Press button to begin timer cycle
5. **Override**: Press button anytime to toggle relay state

## Safety

- **High Voltage**: Controls mains power through relay
- **Proper Enclosure**: Use electrical enclosure for safety
- **Correct Rating**: Ensure relay matches your voltage/current needs
