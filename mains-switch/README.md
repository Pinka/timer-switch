# Timer Switch with Dual Potentiometer Control

This Arduino project controls a mains switch (relay) with independently adjustable ON and OFF timing periods using two potentiometers. The button now toggles the current relay state instead of advancing through states.

## Hardware Requirements

- Arduino board (Uno, Nano, etc.)
- LCD display with I2C interface (16x2)
- Relay module for mains switching
- Push button
- LED indicator
- **Two 10kΩ Potentiometers** (ON time and OFF time control)

## Wiring Diagram

### Potentiometer Connections

Connect the potentiometers to the Arduino as follows:

```
ON Time Potentiometer:
- Pin 1 (VCC) → Arduino 5V
- Pin 2 (Wiper) → Arduino A0 (Analog Pin 0)
- Pin 3 (GND) → Arduino GND

OFF Time Potentiometer:
- Pin 1 (VCC) → Arduino 5V
- Pin 2 (Wiper) → Arduino A1 (Analog Pin 1)
- Pin 3 (GND) → Arduino GND
```

### Other Connections

- Relay: Pin 9
- Button: Pin 4 (with internal pull-up)
- LED: Pin 13
- LCD: I2C pins (SDA, SCL)

## Features

### Original Functionality

- Automatic on/off cycling based on initial timing calibration
- Manual override with push button
- LCD display showing current state and timers
- Global timeout protection (120 minutes)

### New Dual Potentiometer Features

- **Independent ON/OFF control**: Two separate potentiometers for ON and OFF timing
- **Real-time adjustment**: Turn either potentiometer to adjust respective timing periods
- **Range**: 20% to 500% of base timing (10 seconds to 5 minutes) for each timer
- **Base timing**: 1 minute ON, 1 minute OFF (adjustable in code)
- **Visual feedback**: LCD shows current adjustment percentages for both timers
- **Smooth operation**: Updates every 100ms for responsive control

### New Button Functionality

- **Toggle operation**: Button now toggles the current relay state (ON/OFF)
- **System start**: First press starts the system and turns relay ON
- **Manual override**: Subsequent presses toggle between ON and OFF states
- **State-aware**: Button behavior adapts to current system state

## How It Works

1. **Initial Setup**: The system starts in "Ready" state showing both ON and OFF adjustment percentages
2. **System Start**: Press button to start the system and begin first ON cycle
3. **Manual Control**: Press button anytime to toggle relay state (ON/OFF)
4. **Automatic Operation**: System uses potentiometer-adjusted timing for automatic cycles
5. **Independent Adjustment**: Turn ON potentiometer to adjust ON time, OFF potentiometer for OFF time
6. **Visual Feedback**: LCD displays current adjustment percentages and countdown timers

## Code Configuration

You can modify these variables in the code to customize the behavior:

```cpp
unsigned long baseOnTime = 60000;      // Base ON time: 1 minute
unsigned long baseOffTime = 60000;     // Base OFF time: 1 minute
unsigned long minTime = 10000;         // Minimum time: 10 seconds
unsigned long maxTime = 300000;        // Maximum time: 5 minutes
```

## Usage

1. Connect all components according to the wiring diagram
2. Upload the code to your Arduino
3. Power on the system
4. Turn the ON potentiometer to adjust ON time (20% to 500%)
5. Turn the OFF potentiometer to adjust OFF time (20% to 500%)
6. Press the button to start the timer cycle
7. Press the button anytime to toggle the relay state
8. The system will automatically cycle ON/OFF based on your potentiometer settings

## Safety Notes

- **High Voltage Warning**: This project controls mains voltage through a relay
- **Proper Isolation**: Ensure proper electrical isolation between Arduino and mains circuits
- **Relay Rating**: Use a relay rated for your mains voltage and current requirements
- **Enclosure**: Mount in a proper electrical enclosure for safety

## Troubleshooting

- **No LCD Display**: Check I2C connections and address
- **Potentiometers Not Working**: Verify analog pin connections (A0 for ON, A1 for OFF)
- **Relay Not Switching**: Check relay connections and power supply
- **Timing Issues**: Verify potentiometers are properly connected to pins A0 and A1
- **Button Not Responding**: Check button wiring and internal pull-up configuration
