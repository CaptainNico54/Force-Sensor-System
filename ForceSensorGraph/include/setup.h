// Which version of the PCB do you have?
//#define PCBV1
#define PCBV2

/*   #############   IMPORTANT - ACTION REQUIRED!!!!   ##############
The constructor for the TFT_ILI9341 object uses hardwired values for the SPI pins.
So, currently these must be set in the User_Setup.h file that comes with the library.
For the latest board (PCBV2), TFT_DC == 8; TFT_RST == 9; TFT_CS == 10.
For the older board (PCBV1), TFT_DC == SDA; TFT_RST == -1; TFT_CS == SCL.
ALSO: You must comment out FAST_LINE, and all the fonts except for LOAD_GLCD
Look for this file in the folder .pio/libdeps/<platform>/TFT_ILI9341
*/

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

// If DEBUG is anything but zero, the program will block until a serial monitor is attached/open
// Set to 1 to get force sensor values on the serial monitor
// Set to 2 to get more detailed program status
#define DEBUG 0

// Screen orientation - uncomment the next line to invert the LCD
// #define FLIP_TFT

// Comment the next line in/out to invert the sign of the load cell values
// #define INVERT_Y

#define DATA_INTERVAL 333   // How often (ms) to sample and plot data
#define QUEUE_LENGTH 100     // How many points to keep on the FIFO queue?
#define XRANGE 35           // How many seconds does the X axis represent?
#define XTICKTIME 5         // How many seconds between X tick marks?
#define REFERENCE_MASS 1000 // Reference mass for calibration routine, in kg
#define EEPROM_ADDR 1019    // use the last four bytes of the EEPROM for calibration constant

// Function prototypes
void tareHandler();
void doTare();
void calibrateHandler();
void doCalibration(TFT_ILI9341 &tft);
void endHandler();
void initChart();
ChartXY::point getMinMax();
boolean scaleY(float yMin, float yMax, String reason);
boolean autoScale(ChartXY::point mm, ChartXY::point p);
void initChart();