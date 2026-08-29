# FitRide
A prototype of a bicycle-mounted system for real-time speed, distance, and calorie tracking, meant for health-conscious people.

FitRide is a bicycle-mounted system made to track speed, distance, and estimated calories burned while riding.

The project uses a Hall-effect sensor and magnets attached to the bicycle wheel to detect its rotation. An Arduino processes this data to calculate the speed and distance travelled.

The calculated values are displayed on a 16×2 LCD and are also sent to a mobile phone through Bluetooth.

## Features

- Real-time speed tracking
- Distance calculation
- Estimated calorie calculation
- LCD display
- Bluetooth data transmission

## Components Used

- Arduino
- Hall-effect sensor
- Magnets
- 16×2 I2C LCD
- Bluetooth module

## How It Works

Three magnets are attached to the bicycle wheel, and the Hall-effect sensor detects them as the wheel rotates. The Arduino uses the time between the sensor readings to calculate the speed and distance. Calories are estimated based on the speed and rider's weight.

## Code

The main Arduino code for the project is available in:

`FitRide.ino`