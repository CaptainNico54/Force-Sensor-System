#define HX711_DOUT 19
#define HX711_SCK 18
#define ONE_WIRE_BUS 26 // Where the DS18B20 DQ pin is wired
#define T_IN_C          // Set this to T_IN_C or T_IN_F, to report temperatures in C or F

// If DEBUG is anything but zero, the program will block until a serial monitor is attached/open
// Set to 1 to get force sensor values on the serial monitor
// Set to 2 to get more detailed program status
#define DEBUG 1

// Configurable parameters
#define DATA_INTERVAL 250       // How often (ms) to sample data
#define REFERENCE_MASS 1000     // Reference mass for calibration routine, in g
#define INVERT_FORCES 1         // set to something other than zero if you need inverted forces

// Force calibration settings
// #define OVERRIDE_CALIBRATION 10 // Uncommenting this line sets the calibration value
#define DEFAULT_CALIBRATION 100 // If no calibration is stored in flash, use this
// Use the last sector of the flash for calibration constant
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)