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
#define DEBUG 0

// Specify the screen orientation - Landscape is 1 or 3 on ILI9341
#define CHART_ORIENTATION 1

// Comment the next line in/out to invert the sign of the load cell values
// #define INVERT_Y

#define DATA_INTERVAL 150       // How often (ms) to sample and plot data
#define QUEUE_LENGTH 200        // How many points to keep on the FIFO queue?
#define XRANGE 35               // How many seconds does the X axis represent?
#define XTICKTIME 5             // How many seconds between X tick marks?
#define REFERENCE_MASS 1000     // Reference mass for calibration routine, in g

// use the last sector of the flash for calibration constant
#define FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define OVERRIDE_CALIBRATION 10 // Override the FLASH calibration value with this one

// Function prototypes - DO NOT CHANGE
void tareHandler();
void doTare();
void calibrateHandler();
void doCalibration(TFT_eSPI &tft);
void endHandler();
void initChart();
ChartXY::point getMinMax();
boolean scaleY(float yMin, float yMax, String reason);
boolean autoScale(ChartXY::point mm, ChartXY::point p);
void initChart();