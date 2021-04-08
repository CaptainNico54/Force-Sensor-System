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
#include <TFT_ILI9341.h>
#include <TFT_Charts.h>
#include <HX711.h>
#include <OneButton.h>
#include <cppQueue.h>
#include "pins.h" // Customize TFT and hx711 pins in here
// #include <EEPROM.h>

// Initialize some global variables
const int dataInterval = 250;      // How often (ms) to sample and plot data
const int fQLen = 100;             // How many points to keep on the FIFO queue?
const int calVal_eepromAdress = 0; // What EEPROM address should store the calibration
float t_offset = 0;                // Offset for t=0 on X.
float xPrev = 0;                   // Previous value for X
float yPrev = 0;                   // Previous value for Y
struct point                       // Points struct to put in the queue
{
  float x, y;
};

// Instantiate a cppQueue to store fQLen number of points
cppQueue fQ(sizeof(point), fQLen, FIFO);

// ILI9341 constructor: Use hardware SPI for the TFT.
// On Uno/Leonardo, pins #13, #12, #11 and the #defines in TFT_ILI9341/User_Setup.h
TFT_ILI9341 tft = TFT_ILI9341(TFT_CS, TFT_DC);

OneButton tareButton(TARE_PIN, false); // OneButton constructor | false -> pullup
HX711 hx711;                           // HX711 constructor
ChartXY xyChart;                       // ChartXY constructor

void tareHandler()
{
  Serial.print("Taring...");
  delay(500);   // Let things settle for 500ms before reading.
  hx711.tare(); // Take the tare reading (10 samples is default)
  Serial.println("...Done.");
}

void calibrateHandler()
{
  Serial.print("Calibration routine to go here...");
  hx711.set_scale(2);
  hx711.tare();
}

void setup()
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

  // Initialize the force sensor
  hx711.begin(HX711_A_DOUT, HX711_A_SCK);
  hx711.tare(20);      // Tare with 20 readings (default is 10)
  hx711.set_scale(30); // This should eventually come out of EEPROM

  Serial.println("Finished initializing load cell.");

  // Tare button setup:
  pinMode(TARE_PIN, INPUT_PULLUP);
  tareButton.attachClick(tareHandler);
  tareButton.attachLongPressStart(calibrateHandler);

  // Seed the queue with the first point of the first line (origin)
  point p;
  p.x = 0.;
  p.y = 0.;
  fQ.push(&p);

  // Initialize the screen
  tft.begin();
  xyChart.begin(tft);
  tft.fillScreen(BLACK);
  xyChart.tftInfo();
  xyChart.setAxisLimitsX(0, 30, 6);
  xyChart.setAxisLimitsY(-100, 100, 25);
  xyChart.drawTitle(tft, "Load Cell A");
  xyChart.drawAxisX(tft, 10);
  xyChart.drawAxisY(tft, 10);
  xyChart.drawLabelsX(tft);
  xyChart.drawLabelsY(tft);
  delay(300); // Let noise settle

  // We are now at t=0. Set x-axis offset to now (in seconds)
  t_offset = millis() / 1000;
}

// Read data and throw it at the screen forever
void loop(void)
{
  int i;        // Loop indices
  point p, p0, p1; // Temporary points to hold queue values
  float dx;        // Delta-X, for interleaved line scrolling on X axis

  tareButton.tick(); // Check the tare button

  // Get smoothed value from the dataset:
  if (hx711.is_ready())
  {
    // All of the time-based logic will blow up when millis() overflows (~49 days).
    if (millis() > (((p.x + t_offset) * 1000) + dataInterval))
    {
      p.y = hx711.get_units();                   // Read the load cell value as a float (4 bytes on 8-bit AVRs)
      p.x = (float(millis()) / 1000 - t_offset); // Elapsed time in seconds.  (Why must I cast millis() here?)

      // Report the current [time, force] value
      Serial.print(p.x);
      Serial.print(", ");
      Serial.println(p.y);

      // Check if the queue is full
      if (fQ.isFull())
      {
        xyChart.isScrolling = true;               // Start scrolling the data on X
        fQ.pop(&p0);                              // Pop the oldest point
        fQ.push(&p);                              // Update the queue with latest value
        fQ.peek(&p1);                             // Peek at the new oldest point -> New xMin
        dx = p1.x - p0.x;                         // We will move the axis right by dx, remember it
        xyChart.setAxisLimitsX(p1.x, xyChart.xMax + dx, 6); // New X axis limits run from oldest point to 30+dx
        Serial.print("New xMin is ");
        Serial.println(p1.x);
        Serial.print("New xMax is ");
        Serial.println(xyChart.xMax + dx);
        // Erase the line between between the popped point and the new oldest point.
        xyChart.eraseLine(tft, p0.x + dx, p0.y, p1.x + dx, p1.y);
          // Redraw the axes/labels
        xyChart.drawAxisX(tft, 10);
        xyChart.drawLabelsX(tft);
        xyChart.drawAxisY(tft, 10);
        // Now move lines one by one (looks SO much better than clearing/drawing, but bookeeping...)
        for (i = 1; i <= fQLen; i++)
        {
          fQ.peekIdx(&p0, i - 1);                        // Peek at tail of line to move
          fQ.peekIdx(&p1, i);                            // Peek at head of line to move
          xyChart.drawLine(tft, p0.x, p0.y, p1.x, p1.y); // draw it at the current x-axis limits
          xyChart.eraseLine(tft, p0.x + dx, p0.y, p1.x + dx, p1.y); // erase it at the old x-axis limits
          xyChart.eraseLine(tft, p0.x + dx, p0.y, p1.x + dx, p1.y); // erase it at the old x-axis limits

        }
      }
      else
      {
        fQ.push(&p); // Push the latest value onto the queue
        xyChart.drawLine(tft, xPrev, yPrev, p.x, p.y);
        xPrev = p.x;
        yPrev = p.y;
      }
    }
  }
}
