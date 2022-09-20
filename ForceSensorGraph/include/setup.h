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

// Specify the screen orientation - Landscape is 1 or 3 on ILI9341
#define CHART_ORIENTATION 1

// Comment the next line in/out to invert the sign of the load cell values
// #define INVERT_Y

#define DATA_INTERVAL 150   // How often (ms) to sample and plot data
#define QUEUE_LENGTH 210    // How many points to keep on the FIFO queue?
#define XRANGE 35           // How many seconds does the X axis represent?
#define XTICKTIME 5         // How many seconds between X tick marks?
#define REFERENCE_MASS 1000 // Reference mass for calibration routine, in g
#define MIN_Y_RANGE 200     // Minimum value for the Y-axis range (+/- this value)

// use the last sector of the flash for calibration constant
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define DEFAULT_CALIBRATION 100 // If no calibration is stored in flash, use this
// #define OVERRIDE_CALIBRATION 10 // Uncommenting overrides any calibration value with this one

// Function prototypes - DO NOT CHANGE
int getCalibrationPage();
int getCalibration();
void writeCalibration(int cal);
void tareHandler();
void doTare(uint8_t dt);
void calibrateHandler();
void doCalibration(TFT_eSPI &tft);
void endHandler();
void initChart();
ChartXY::point getMinMax();
boolean scaleY(float yMin, float yMax, String reason);
boolean autoScale(ChartXY::point mm, ChartXY::point p);
void initChart();