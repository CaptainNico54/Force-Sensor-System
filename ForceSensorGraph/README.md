## PlatformIO project for live graphing of load sensor data to an ILI9341 (or other) TFT
Requires load cell board, hx711 load cell, and serial SPI ILI9341 screen.  If your screen can only take 3.3V logic, then you also need at least 5 channels of logic conversion.

- Install and run VSCode
- Install the PlatformIO extension from the VSCode extensions pane
- Clone the Photonsters/Print-Load-Sensor repository from the Git pane
- File->Open Folder, then navigate to this ForceSensorGraph directory
- PlatformIO will parse platformio.ini and should install the required frameworks and libraries for you
- Read the comments at the top of the source and include files.  Edit as necessary for your system/hardware
- Build/Upload

![TFT Load Cell Graphing](test/Breadboard.jpeg)
