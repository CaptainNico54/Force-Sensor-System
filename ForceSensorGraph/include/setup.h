/*
NOTE: the constructor for the TFT object uses hardwired values for the SPI pins.
So, currently these must be set in the User_Setup.h file that comes with the library.
For the latest board (PCBV2), TFT_DC == 8; TFT_RST == 9; TFT_CS == 10.
For the older board (PCBV1), TFT_DC == SDA; TFT_RST == -1; TFT_CS == SCL.
ALSO: You must comment out FAST_LINE, and all the fonts except for LOAD_GLCD
Look for this file in the folder .pio/libdeps/<platform>/TFT_ILI9341
*/

// If DEBUG is anything but zero, the program will block until a serial monitor is attached/open
// Set to 1 to get force sensor values on the serial monitor
// Set to 2 to get more detailed program status
#define DEBUG 2

// Which version of the PCB do you have?
//#define PCBV1
#define PCBV2

#ifdef PCBV1
    #define HX711_DOUT A1
    #define HX711_SCK A0
    #define TARE_PIN 13
#endif

#ifdef PCBV2
    #define HX711_DOUT 1
    #define HX711_SCK 0
    #define TARE_PIN 13
    #define LCD_SPI_EN 5 // Drive LCD_SPI_EN low to enable the 5V -> 3.3V logic converters
#endif

// Screen orientation - set to true to invert the LCD
const boolean flipScreen = false;

const int dataInterval = 250;        // How often (ms) to sample and plot data
const uint8_t fQLen = 90;            // How many points to keep on the FIFO queue?
const float referenceMass = 1000;    // Reference mass for calibration routine, in kg
const uint16_t eepromAddress = 1019; // use the last four bytes of the EEPROM for calibration constant

// Function prototypes
void tareHandler();
void doTare();
void calibrateHandler();
void doCalibration(TFT_ILI9341 &tft);
void endHandler();