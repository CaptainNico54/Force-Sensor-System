// If DEBUG is anything but zero, the program will block until a serial monitor is attached/open
// Set to 1 to get force sensor values on the serial monitor
// Set to 2 to get more detailed program status
#define DEBUG 0

const int dataInterval = 250; // How often (ms) to sample and plot data
const uint8_t fQLen = 90;     // How many points to keep on the FIFO queue?

// LoadCell pins
const uint8_t HX711_DOUT = A1;
const uint8_t HX711_SCK = A0;

// Tare Button (pin 13 for V0.2)
const uint8_t TARE_PIN = 13;

// Drive LCD_SPI_EN low to enable the 5V -> 3.3V logic converters
const uint8_t LCD_SPI_EN = 5;

const float referenceMass = 1000;    // Reference mass for calibration routine, in kg
const uint16_t eepromAddress = 1019; // use the last four bytes of the EEPROM for calibration constant

// Function prototypes
void tareHandler();
void doTare();
void calibrateHandler();
void doCalibration(TFT_ILI9341 &tft);
void endHandler();