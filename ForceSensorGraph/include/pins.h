// For Nico's force sensor board and the ILI9341 TFT:
// SCL will be CS
// SDA will be DC
// TFT_RST will piggyback on the MCU reset signal (-1)
// If using TFT_ILI9341, those values must be set in User_Setup.h
// If using Adafruit_ILI9341 (slow), those values can be set here, just uncomment:
// #define TFT_DC SDA
// #define TFT_CS SCL
// #define TFT_RST -1

// LoadCell pins
// LoadCell A:
const int HX711_A_DOUT = A1;
const int HX711_A_SCK = A0;
// LoadCell B:
const int HX711_B_DOUT = A2; // Is this right?
const int HX711_B_SCK = A3;  // Is this right?

// Tare Button (pin 13 for V0.2)
const int TARE_PIN = 13;