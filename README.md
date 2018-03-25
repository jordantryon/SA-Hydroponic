# SA-Hydroponic
**Description:** Arduino code and console application for the SourceAmerica hydroponic vertical farming system.

**Version:** 0.2.0

## Authors / Contributors
- Jordan Tryon
- Daniel Tryon
- Cas Usiatynski

## Troubleshooting
1. Check for updates made to the GitHub repository.
2. Check error message given by the console application, make necessary modifications if possible.
3. Check VFproperties.txt and ensure it contains the correct IDs and is formatted correctly as follows:
```
Machine Serial Number: 000
Current Spreadsheet ID: 00000000000000000000000000000000000000000000
Google Drive Folder ID: 000000000000000000000000000000000
```
4. If problem persists, please send an Email to SAverticalfarm@gmail.com for further assistance.

## Change Log
**0.0.1 The Safety Update**
- Run AC motor to cycle through trays automatically.
- Stop to water trays for specified amount of time.

0.0.2
- Added sensors to check temperature, humidity, etc.
- Print sensor data to serial monitor for operators.

0.0.3
- Added error checking features to prevent system damage.
  - Left and right time offset.
  - Left and right tray counts.
  - Lack of water in reservoir.s
- Added eStop buttons to allow operators to stop the machine easily and safely.

**0.1.0 The Logging Update**
- Implemented console application to log serial data into text files.

0.1.1
- Console application modified to log data into a Google Sheet.

**0.2.0 The User Friendly Update**
- Added support for an LCD to display current sensor readings and status messages.
- Added secondary pump to refill reservoir, machine can now make it through the night.
- Code is commented an reorganized, now with functions.
