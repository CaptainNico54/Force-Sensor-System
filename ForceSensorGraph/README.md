## PlatformIO project for live graphing of load sensor data to an ILI9341 TFT
Requires hx711 load cell board, compatible load cell (this one is 20kg), and serial SPI ILI9341 screen.  If your screen can only take 3.3V logic, then you need to drive digital pin 5 low to enable the onboard 5V -> 3.3V logic conversion (this is enabled by default).

- Install and run VSCode
- Install the PlatformIO extension from the VSCode extensions pane
- Clone the Photonsters/Print-Load-Sensor repository from the Git pane
- File->Open Folder, then navigate to this ForceSensorGraph directory
- PlatformIO will parse platformio.ini and should install the required frameworks and libraries for you
- Read the comments at the top of the source and include files, in particular setup.h.  Edit as necessary for your system/hardware
- Once PlatformIO has installed the libraries called out in platformio.ini, navigate to the .pio/libdeps/<board>/TFT_ILI9341 directory in the project.
- Modify User_Setup.h in that directory, to remove all but one font (the project won't fit in flash if you don't do this):
```
   #define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
// #define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
// #define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
// #define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
// #define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
// #define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
```
Then define these pins:
```
#define TFT_CS   10  // Chip select control pin
#define TFT_DC   8   // Data Command control pin
#define TFT_RST  9   // Reset pin (could use the AVR's RESET pin by setting this to -1)
```
Finally and optionally, comment out this line as we find it causes stray pixels to be left onscreen with some displays (but not all):
```
// #define FAST_LINE
```

- If you want informational messages to the serial monitor, set DEBUG = 2 at the top of setup.h.  This causes the code to block until a serial monitor is present.
- Build/Upload the project to your board.
