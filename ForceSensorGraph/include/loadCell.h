#define DEBUG 1 // Set to 1 to get program status on the Serial monitor

// LoadCell pins
// LoadCell A:
const int HX711_DOUT = A1;
const int HX711_SCK = A0;

// Tare Button (pin 13 for V0.2)
const int TARE_PIN = 13;

const float referenceMass = 1.0; // Reference mass for calibration routine, in kg
const uint16_t eepromAddress = 1019; // use the last four bytes of the EEPROM for calibration constant

// Prototypes
void tareHandler();
void doTare();
void calibrateHandler();
void doCalibration(TFT_ILI9341 &tft);
void endHandler();