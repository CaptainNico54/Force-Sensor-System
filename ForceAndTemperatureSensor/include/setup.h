// Which version of the PCB do you have?
#define PCBV3

#ifdef PCBV3
#define HX711_DOUT 19
#define HX711_SCK 18
#define TARE_PIN 1
#endif

// If DEBUG is anything but zero, the program will block until a serial monitor is attached/open
// Set to 1 to get force sensor values on the serial monitor
// Set to 2 to get more detailed program status
#define DEBUG 1

// Configurable parameters
#define DATA_INTERVAL 250       // How often (ms) to sample data
#define REFERENCE_MASS 1000     // Reference mass for calibration routine, in g
#define VBE 0.706               // Internal diode voltage to be subtracted from temperature ADC reading
#define DEG_PER_V 0.001721      // Change of diode voltage per degree C

// Force calibration settings
// #define OVERRIDE_CALIBRATION 10 // Uncommenting this line sets the calibration value
#define DEFAULT_CALIBRATION 100 // If no calibration is stored in flash, use this
// Use the last sector of the flash for calibration constant
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)


// Function prototypes - DO NOT CHANGE
int getCalibrationPage();
int getCalibration();
void writeCalibration(int cal);
void tareHandler();
void doTare(uint8_t dt);
void calibrateHandler();
void doCalibration();
void endHandler();
void temperature_init();
float temperature_read();