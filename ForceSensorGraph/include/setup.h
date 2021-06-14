#define DEBUG 1 // Set to 1 to get program status on the Serial monitor


const int dataInterval = 250; // How often (ms) to sample and plot data
const uint8_t fQLen = 90;     // How many points to keep on the FIFO queue?

// LoadCell pins
// LoadCell A:
const uint8_t HX711_DOUT = A1;
const uint8_t HX711_SCK = A0;

// Tare Button (pin 13 for V0.2)
const uint8_t TARE_PIN = 13;

// Drive LCD_SPI_EN low to enable the 5V -> 3.3V logic converters
const uint8_t LCD_SPI_EN = 5;

// const float referenceMass = 1.0;     // Reference mass for calibration routine, in kg
const uint16_t eepromAddress = 1019; // use the last four bytes of the EEPROM for calibration constant

// Prototypes
void tareHandler();
void doTare();
void calibrateHandler();
void doCalibration(TFT_ILI9341 &tft);
void endHandler();