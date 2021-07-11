// NOTE: the constructor for the TFT object takes hardwired values for the SPI Pins.
// These must be set in the User_Setup.h file that comes with the library.
// For the latest board, TFT_DC = 8; TFT_RST=9; TFT_CS = 10.
// Also, comment out all the fonts except LOAD_GLCD, and comment out the FAST_LINE feature.
// Look for this file in the folder .pio/libdeps/<platform>/TFT_ILI9341


// If DEBUG is anything but zero, the program will block until a serial monitor is attached/open
// Set to 1 to get force sensor values on the serial monitor
// Set to 2 to get more detailed program status
#define DEBUG 0

// Which version of the PCB do you have?
// #define PCBV1
#define PCBV2

const int dataInterval = 250; // How often (ms) to sample and plot data
const uint8_t fQLen = 90;     // How many points to keep on the FIFO queue?

#ifdef PCBV1
// LoadCell pins
const uint8_t HX711_DOUT = A1;
const uint8_t HX711_SCK = A0;
#endif

#ifdef PCBV2
const uint8_t HX711_DOUT = 1;
const uint8_t HX711_SCK = 0;

// Drive LCD_SPI_EN low to enable the 5V -> 3.3V logic converters
const uint8_t LCD_SPI_EN = 5;
#endif

// Tare Button (pin 13 for V0.2)
const uint8_t TARE_PIN = 13;

const float referenceMass = 1000;    // Reference mass for calibration routine, in kg
const uint16_t eepromAddress = 1019; // use the last four bytes of the EEPROM for calibration constant

// Function prototypes
void tareHandler();
void doTare();
void calibrateHandler();
void doCalibration(TFT_ILI9341 &tft);
void endHandler();