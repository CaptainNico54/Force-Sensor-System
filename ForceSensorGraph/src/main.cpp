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
#include <SPI.h>
#include <TFT_Charts.h>
#include <HX711.h>
#include <OneButton.h>
#include <cppQueue.h>
#include "pins.h" // Customize TFT and hx711 pins in here
// #include <EEPROM.h>

#define DEBUG 0 // Set to 1 to get program status on the Serial monitor

// Initialize some global variables
const int dataInterval = 250;      // How often (ms) to sample and plot data
const int fQLen = 100;             // How many points to keep on the FIFO queue?
const int calVal_eepromAdress = 0; // What EEPROM address should store the calibration
float t_offset = 0;                // Offset for t=0 on X.
float xPrev = 0;                   // Previous value for X
float yPrev = 0;                   // Previous value for Y
float fMean, allTimeSum, allTimeSamples;

// Instantiate a cppQueue to store fQLen number of points
cppQueue fQ(sizeof(ChartXY::point), fQLen, FIFO);

// ILI9341 constructor: Use hardware SPI for the TFT.
// On Uno/Leonardo, pins #13, #12, #11 and the #defines in TFT_ILI9341/User_Setup.h
TFT_ILI9341 tft = TFT_ILI9341(TFT_CS, TFT_DC);

OneButton tareButton(TARE_PIN, false); // OneButton constructor | false -> pullup
HX711 hx711;                           // HX711 constructor
ChartXY xyChart;                       // ChartXY constructor

void tareHandler()
{
  if (DEBUG)
  {
    Serial.print("Taring...");
  }
  delay(500);   // Let things settle for 500ms before reading.
  hx711.tare(); // Take the tare reading (10 samples is default)
  if (DEBUG)
  {
    Serial.println("...Done.");
  }
  fMean = allTimeSum = allTimeSamples = 0;
}

void calibrateHandler()
{
  if (DEBUG)
  {
    Serial.print("Calibration routine to go here...");
  }
  hx711.set_scale(2);
  hx711.tare();
}

ChartXY::point getMinMax()
{
  int i;
  float min, max;
  ChartXY::point p;

  // Initialize fmin, fMax with the first y-value on the queue
  fQ.peekIdx(&p, 0);
  min = p.y;
  max = p.y;

  for (i = 1; i <= fQLen; i++)
  {
    fQ.peekIdx(&p, i);
    if (p.y < min)
    {
      min = p.y;
    }
    if (p.y > max)
    {
      max = p.y;
    }
  }
  p.x = min; // Return min as the x-coord of this "point"
  p.y = max; // Return max as the y-coord of this "point"
  return (p);
}

boolean scaleY(float yMin, float yMax, String reason)
{

  if (DEBUG)
  {
    Serial.println(reason);
    Serial.println("Current Y limits: " + String(xyChart.yMin) + ", " + String(xyChart.yMax));
    Serial.println("Scaling Y to range " + String(yMin) + ", " + String(yMax));
  }
  tft.fillScreen(xyChart.tftBGColor);
  xyChart.drawTitleChart(tft, "Load Cell A");
  xyChart.setAxisLimitsY(yMin, yMax, (yMax - yMin) / 8);
  //xyChart.eraseChartRegion(tft);
  xyChart.drawAxisY(tft, 10);
  xyChart.drawLabelsY(tft);
  xyChart.drawAxisX(tft, 10);
  xyChart.drawLabelsX(tft);
  xyChart.drawY0(tft);
  return (true);
}

// Scale the Y axis if needed
boolean autoScale(ChartXY::point mm, ChartXY::point p)
{
  float fMin = mm.x;
  float fMax = mm.y;
  float shrinkFactor = 7; // Determines the default Y range in relation to the current min/max values when shrinking limits
  float growFactor = 0.1; // Determines the additional Y range added to the current min/max values when expanding limits
  boolean scaled = false;

  if (DEBUG)
  {
    Serial.println("\nCheck limits: Current Y value is " + String(p.y));
    Serial.println("fMin = " + String(fMin) + ", fMax = " + String(fMax));
  }

  if (p.y < xyChart.yMin)
  {
    scaled = scaleY(p.y - (growFactor * fabs(p.y)), xyChart.yMax, "New Y value less than yMin");
  }
  else if (p.y > xyChart.yMax)
  {
    scaled = scaleY(xyChart.yMin, p.y + (growFactor * fabs(p.y)), "New Y value more than yMax");
  }
  else if ((xyChart.yMax - xyChart.yMin) > (shrinkFactor * (fMax - fMin)))
  {
    float yRange = fMax - fMin;
    float yMid = fMin + (yRange / 2);
    float yLimit = yRange * (shrinkFactor - 2) / 2;
    scaled = scaleY(yMid - yLimit, yMid + yLimit, "Y limits too large relative to Y range.");
  }
  return (scaled);
}

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
    Serial.print(fQLen);
    Serial.println(" points.");
  }

  fMean = allTimeSum = allTimeSamples = 0; // Initialize fMean

  // Initialize the force sensor
  hx711.begin(HX711_A_DOUT, HX711_A_SCK);
  hx711.tare(20);      // Tare with 20 readings (default is 10)
  hx711.set_scale(30); // This should eventually come out of EEPROM

  if (DEBUG)
  {
    Serial.println("Finished initializing load cell.");
  }

  // Tare button setup:
  pinMode(TARE_PIN, INPUT_PULLUP);
  tareButton.attachClick(tareHandler);
  tareButton.attachLongPressStart(calibrateHandler);

  // Seed the queue with the first point of the first line (origin)
  ChartXY::point p;
  p.x = 0.;
  p.y = 0.;
  fQ.push(&p);

  // Initialize the screen
  tft.begin();
  xyChart.begin(tft);
  tft.fillScreen(BLACK);
  if (DEBUG)
  {
    xyChart.tftInfo();
  }
  xyChart.setAxisLimitsX(0, 30, 6);
  xyChart.setAxisLimitsY(-100, 100, 25);
  xyChart.drawTitleChart(tft, "Load Cell A");
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
  int i;                    // Loop indices
  ChartXY::point p, p0, p1; // Temporary points to hold queue values
  float dx;                 // Delta-X, for interleaved line scrolling on X axis
  float fMin = 0, fMax = 0;
  boolean scrolling = false, noise = false;
  String legend;

  tareButton.tick(); // Check the tare button

  // Get smoothed value from the dataset:
  if (hx711.is_ready())
  {
    // All of the time-based logic will blow up when millis() overflows (~49 days).
    if (millis() > (((xPrev + t_offset) * 1000) + dataInterval))
    {
      p.y = hx711.get_units();                   // Read the load cell value as a float (4 bytes on 8-bit AVRs)
      p.x = (float(millis()) / 1000 - t_offset); // Elapsed time in seconds.  (Why must I cast millis() here?)

      allTimeSamples += 1;
      allTimeSum += p.y;
      fMean = allTimeSum / allTimeSamples;
      p.y -= fMean; // Normalize the reading by subtracting the average force reading

      // Report the current [time, force] value
      if (DEBUG)
      {
        Serial.print("\nCurrent time, force point: ");
        Serial.print(p.x);
        Serial.print(", ");
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
      legend = " Min:" + String(fMin, 1) + "    ";
      xyChart.drawLegend(tft, legend, 230, 20, 1, GREEN);
      legend = "Norm:" + String(fMean, 1) + "    ";
      xyChart.drawLegend(tft, legend, 230, 30, 1, DKORANGE);

      // Check if the queue is full
      if (fQ.isFull())
      {
        scrolling = true;
        fQ.pop(&p0);                                        // Pop the oldest point
        fQ.peek(&p1);                                       // Peek at the new oldest point -> New xMin
        dx = p1.x - p0.x;                                   // We will move the axis right by dx, remember it
        xyChart.setAxisLimitsX(p1.x, xyChart.xMax + dx, 6); // New X axis limits run from oldest point to xMax+dx
      }
      else
      {
        fQ.peek(&p0);       // or peek the oldest point -> "New" xMin
        fQ.peekIdx(&p1, 1); // and peek at the second oldest point
        dx = 0;
      }

      fQ.push(&p); // Update the queue with latest value
      xPrev = p.x;
      yPrev = p.y;

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
      for (i = 1; i <= fQLen; i++)
      {
        fQ.peekIdx(&p0, i - 1);                                   // Peek at tail of line to move
        fQ.peekIdx(&p1, i);                                       // Peek at head of line to move
        xyChart.eraseLine(tft, p0.x + dx, p0.y, p1.x + dx, p1.y); // erase it at the old x-axis limits
        xyChart.drawLine(tft, p0.x, p0.y, p1.x, p1.y);            // draw it at the current x-axis limits
      }
      if (noise) // Wait here for noise to decay (after the graph is drawn) - it looks prettier.
      {
        delay(300);
        noise = false;
      }
    }
  }
}