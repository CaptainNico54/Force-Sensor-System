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
#include <Adafruit_ILI9341.h>
#include <HX711_ADC.h>
// #include <EEPROM.h>
#include "colors.h" // Custom colors are here
#include "graph.h"  // Customize graph parameters here
#include "pins.h"   // Customize TFT and hx711 pins in here

// Initialize some global variables
const int calVal_eepromAdress = 0;
float force;
float t, t_offset = 0; // old and current times for x-axis

// HX711 constructor:
HX711_ADC hx711a(HX711_A_DOUT, HX711_A_SCK);
//HX711_ADC hx711b(HX711_B_DOUT, HX711_B_SCK);

// ILI9341 constructor:
// Use hardware SPI (on Uno/Leonardo, pins #13, #12, #11) and the #defines in pins.h for CS/DC/RST
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println("Starting...");

  // Initialize the force sensor
  hx711a.begin();
  // Precision right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = false; //set this to true if you want tare to be performed in the next step
  unsigned long stabilizingtime = 2000;
  hx711a.start(stabilizingtime, _tare);
  if (hx711a.getTareTimeoutFlag() || hx711a.getSignalTimeoutFlag())
  {
    Serial.println("Timeout on load cell A. Check MCU>HX711 wiring and pin designations.");
    while (1)
      ;
  }
  else
  {
    hx711a.setCalFactor(1.0); // user set calibration value (float)
    Serial.println("Startup for load cell A is complete");
  }
  //while (!hx711a.update());
  //calibrate(); //start calibration procedure

  // Initialize the screen
  tft.begin();
  tftX = tft.height();                        // The X direction is the hardware's height
  tftY = tft.width();                         // The Y direction is the hardware's width
  graphY = tftY - xLabelMargin;               // Graph location (lower left)
  graphW = tftX - yLabelMargin - rightMargin; // Graph width (X)
  graphH = tftY - xLabelMargin - titleMargin; // Graph height (Y)
  char msg[50];
  tft.setRotation(3);
  sprintf(msg, "Initialized %dx%d TFT screen.", tft.width(), tft.height());
  Serial.println(msg);
  tft.fillScreen(BLACK);
}

void loop(void)
{
  // Read data and throw it at the screen forever

  static boolean newDataReady = 0;
  const int dataInterval = 250; // Change value (ms) to sensor/graph update frequency

  // Check for new data/start next conversion:
  if (hx711a.update())
    newDataReady = true;

  // Start over from X = 0 when time falls off the right side of the graph:
  if (t > xLabelHi)
  {
    t_offset += t; // Remember how many minutes have elapsed so far
    t = 0;         // Reset the minute timer
    Serial.println("Starting graph over!");
    tft.fillScreen(BLACK); // Clear the screen and redraw axes/labels
    xLabelsDraw = true;
    yLabelsDraw = true;
  }

  // Get smoothed value from the dataset:
  if (newDataReady)
  {
    // All of the time-based logic will blow up when millis() overflows (~49 days).
    // TODO: Fix that.
    if (millis() > (((t + t_offset) * 60000) + dataInterval))
    {
      force = (hx711a.getData() - 8528800) / 50; // Kludgy tare + normalization for now
      t = ((float)millis()) / 60000 - t_offset;  // Repeating elapsed time in minutes
      Serial.print("Load cell output: ");
      Serial.print(t);
      Serial.print(", ");
      Serial.println(force);
      newDataReady = 0;

      // Draw a line on the TFT between the last (t, force) point and the current one.
      Graph(tft, t, force, graphX, graphY, graphW, graphH, xLabelLo, xLabelHi, xIncr,
            yLabelLo, yLabelHi, yIncr, title, xTitle, yTitle,
            DKBLUE, RED, YELLOW, WHITE, BLACK, xLabelsDraw, yLabelsDraw);
    }
  }
}
