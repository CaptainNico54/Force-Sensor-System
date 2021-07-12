/*
Copyright 2021, Photonsters
Jarrod Smith [MakerMatrix]

Licensed under the Apache License
https://www.apache.org/licenses/LICENSE-2.0.txt

This program graphs live data from the HX711 Force Sensor onto a Serial SPI ILI9341 TFT.
It's set up to use hardware SPI.  Look in pins.h to define the TFT pins for CS, DC, and RST.

The Graph() function in graph.h is customized from Kris Kasprzak's here:
https://github.com/KrisKasprzak/GraphingFunction/blob/master/Graph.ino
*/

#include <Arduino.h>
#include <TFT_Charts.h>
#include <HX711.h>
#include <OneButton.h>
#include <cppQueue.h>
// #include <EEPROM.h>
#include "setup.h"

// Initialize some global variables
float fMean, allTimeSum, allTimeSamples;
float lastT = 0, t_offset = 0; // Offset for t=0 on X.

// Global external variables
extern boolean taring;       // Taring button activated?
extern boolean calibrating;  // Calibration button activated?
extern boolean done;         // Done calibrating?
extern const int calValAddr; // What EEPROM address should store the calibration

// Button and ADC objects
extern OneButton tareButton; // OneButton constructor
extern HX711 hx711;          // HX711 constructor

// Prototype declarations for functions in chart.cpp
ChartXY::point getMinMax();
boolean scaleY(float yMin, float yMax, String reason);
boolean autoScale(ChartXY::point mm, ChartXY::point p);

// Instantiate a cppQueue to store QUEUE_LENGTH number of points
cppQueue fQ(sizeof(ChartXY::point), QUEUE_LENGTH, FIFO);

// ILI9341 constructor: This library takes width/height for the arguments.
// Hardware SPI pins are required, and are read from TFT_ILI9341/User_Setup.h
TFT_ILI9341 tft = TFT_ILI9341(320, 240);

ChartXY xyChart; // ChartXY constructor

void setup()
{
  if (DEBUG)
  {
    Serial.begin(9600);
    // Wait until a serial monitor comes online
    while (!Serial)
      ;
    Serial.println();
    Serial.println("Starting...");
    Serial.print("Queue size is ");
    Serial.print(QUEUE_LENGTH);
    Serial.println(" points.");
  }

#ifdef PCBV2
  // Drive LCD_SPI_EN to 0V, to enable the onboard 5V -> 3.3V logic converters
  pinMode(LCD_SPI_EN, OUTPUT);
  digitalWrite(LCD_SPI_EN, LOW);
#endif

  fMean = allTimeSum = allTimeSamples = 0; // Initialize fMean

  // Initialize the force sensor
  hx711.begin(HX711_DOUT, HX711_SCK);
  hx711.tare(20);      // Tare with 20 readings (default is 10)
  hx711.set_scale(18); // This should eventually come out of EEPROM

  if (DEBUG == 2)
  {
    Serial.println("Finished initializing load cell.");
  }

  // Tare button setup:
  pinMode(TARE_PIN, INPUT_PULLUP);
  tareButton.attachClick(tareHandler);
  tareButton.attachLongPressStart(calibrateHandler);
  tareButton.attachDoubleClick(endHandler);

  // Seed the queue with the first point of the first line (origin)
  ChartXY::point p;
  p.x = 0.;
  p.y = 0.;
  fQ.push(&p);

  // Initialize the screen
  tft.begin();
  xyChart.begin(tft);

// Invert the screen if requested in setup.h
#ifdef FLIP_TFT
  tft.setRotation(1);
#endif

  tft.fillScreen(BLACK);
  if (DEBUG == 2)
  {
    xyChart.tftInfo();
  }
  xyChart.setAxisLimitsX(0, XRANGE, XTICKTIME);
  xyChart.setAxisLimitsY(-100, 100, 25);
  xyChart.drawTitleChart(tft, "Z-Axis Force");
  xyChart.drawAxisX(tft, 10);
  xyChart.drawAxisY(tft, 10);
  xyChart.drawLabelsX(tft);
  xyChart.drawLabelsY(tft);
  xyChart.drawY0(tft);
  delay(400); // Let noise settle

  // We are now at t=0. Set x-axis offset to now (in seconds)
  t_offset = millis() / 1000;
}

// Read data and throw it at the screen forever
void loop(void)
{
  uint8_t i;                // Loop indices
  ChartXY::point p, p0, p1; // Temporary points to hold queue values
  float dx;                 // Delta-X, for interleaved line scrolling on X axis
  float fMin = 0, fMax = 0;
  boolean scrolling = false, noise = false;
  String legend;

  tareButton.tick(); // Check the tare button

  // Single click
  if (taring)
  {
    taring = false;
    doTare();
  }

  // Long press
  if (calibrating)
  {
    done = false;
    doCalibration(tft);
  }

  // Get smoothed value from the dataset:
  if (hx711.is_ready())
  {
    // All of the time-based logic will blow up when millis() overflows (~49 days).
    if (millis() > (((lastT + t_offset) * 1000) + DATA_INTERVAL))
    {
      p.y = hx711.get_units();                   // Read the load cell value as a float (4 bytes on 8-bit AVRs)
      p.x = (float(millis()) / 1000 - t_offset); // Elapsed time in seconds.  (Why must I cast millis() here?)
      #ifdef INVERT_Y
        p.y = -p.y; // Invert the hx711 reading - this is dependent on the orientation.
      #endif

      // Check for an outlier (presumed glitch/noise)
      if ((fMax > 0 && (p.y > 1000 * fMax)) || (fMin < 0 && (p.y < 1000 * fMin)))
      {
        if (DEBUG == 2)
        {
          Serial.print("DETECTED OUTLIER " + String(p.y) + " - IGNORING THIS VALUE.");
        }
        p.y = fMean;
      }
      
      allTimeSamples += 1;
      allTimeSum += p.y;
      fMean = allTimeSum / allTimeSamples;

      // Report the current [time, force] value
      if (DEBUG == 2)
      {
        Serial.print("\nCurrent time, force point: ");
        Serial.print(p.x);
        Serial.print(", ");
      }

      if (DEBUG)
      {
        Serial.println(p.y);
      }

      if (fQ.getCount() > 20)
      {
        p0 = getMinMax();
        fMin = p0.x;
        fMax = p0.y;              // Work out the min/max values we have in the queue
        noise = autoScale(p0, p); // Autoscale the data in Y
      }
      // Draw a live legend with current/min/max/mean values
      legend = "Curr:" + String(p.y, 1) + "    ";
      xyChart.drawLegend(tft, legend, 230, 0, 1, YELLOW);
      legend = " Max:" + String(fMax, 1) + "    ";
      xyChart.drawLegend(tft, legend, 230, 10, 1, RED);
      legend = "Mean:" + String(fMean, 1) + "    ";
      xyChart.drawLegend(tft, legend, 230, 20, 1, DKORANGE);
      legend = " Min:" + String(fMin, 1) + "    ";
      xyChart.drawLegend(tft, legend, 230, 30, 1, GREEN);
    
      legend = "";

      // Check if the queue is full
      if (fQ.isFull())
      {
        scrolling = true;
        fQ.pop(&p0);                                                // Pop the oldest point
        fQ.peek(&p1);                                               // Peek at the new oldest point -> New xMin
        dx = p1.x - p0.x;                                           // We will move the axis right by dx, remember it
        xyChart.setAxisLimitsX(p1.x, xyChart.xMax + dx, XTICKTIME); // New X axis limits run from oldest point to xMax+dx
      }
      else
      {
        fQ.peek(&p0);       // or peek the oldest point -> "New" xMin
        fQ.peekIdx(&p1, 1); // and peek at the second oldest point
        dx = 0;
      }

      lastT = p.x;
      fQ.push(&p); // Update the queue with latest value

      if (scrolling)
      { // Erase the line between between two oldest points.
        xyChart.eraseLine(tft, p0.x + dx, p0.y, p1.x + dx, p1.y);
        // Redraw the axes/labels
        xyChart.drawAxisX(tft, 10);
        xyChart.drawLabelsX(tft);
        xyChart.drawAxisY(tft, 10);
        xyChart.drawY0(tft);
      }
      // Now move lines one by one (looks SO much better than clearing/drawing, but bookeeping...)
      for (i = 1; i <= QUEUE_LENGTH; i++)
      {
        fQ.peekIdx(&p0, i - 1);                                   // Peek at tail of line to move
        fQ.peekIdx(&p1, i);                                       // Peek at head of line to move
        xyChart.eraseLine(tft, p0.x + dx, p0.y, p1.x + dx, p1.y); // erase it at the old x-axis limits
        xyChart.drawLine(tft, p0.x, p0.y, p1.x, p1.y);            // draw it at the current x-axis limits
      }
      // Wait here for noise to decay (after the graph is drawn) - it looks prettier.
      if (noise)
      {
        delay(300);
        noise = false;
      }
    }
  }
}