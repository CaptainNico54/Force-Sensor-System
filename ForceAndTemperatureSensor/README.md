## PlatformIO project for measuring temperature and load sensor data for the Athena project
Requires hx711 load cell board, compatible load cell (this one is 20kg), and the RP2040-based force sensor PCB

- Install and run VSCode
- Install the PlatformIO extension from the VSCode extensions pane
- Clone the CaptainNico54/Print-Load-Sensor repository from the Git pane
- File->Open Folder, then navigate to the dForceAndTemperatureSensor directory
- PlatformIO will parse platformio.ini and should install the required frameworks and libraries for you
- Read the comments at the top of the source and include files, in particular setup.h.  Edit as necessary for your system/hardware
- If you want informational messages to the serial monitor, set DEBUG = 2 or 1 at the top of setup.h.  This causes the code to block until a serial monitor is present.
- Build/Upload the project to your board.
