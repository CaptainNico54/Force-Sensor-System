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
#include <HX711.h>
#include <OneButton.h>
#include <cppQueue.h>
// #include <EEPROM.h>
#include "colors.h" // Custom colors are here
#include "graph.h"  // Customize graph parameters here
#include "pins.h"   // Customize TFT and hx711 pins in here

// Initialize some global variables
const int dataInterval = 250; // How often (ms) to sample and plot data
const int calVal_eepromAdress = 0;
short iForce, qForce;          // This will be force multiplied by 100 (to save 2x on RAM)
float fForce, t, t_offset = 0; // old and current times for x-axis

// Declare a Queue to store our data after counting the number of points we need to cache
int forceQueueLen = round((xLabelHi - xLabelLo) * (1000.0 / dataInterval));
cppQueue forceQueueA(sizeof(short), forceQueueLen, FIFO);

// ILI9341 constructor: Use hardware SPI for the TFT.
// On Uno/Leonardo, pins #13, #12, #11 and the #defines in pins.h for CS/DC/RST
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

OneButton tareButton(TARE_PIN, false); // OneButton constructor
HX711 hx711A;                          // HX711 constructor

void tareHandler()
{
  Serial.print("Taring...");
  delay(500);      // Let things settle for 500ms before reading.
  hx711A.tare(20); // Take the tare reading with 20 sample (10 is default)
  Serial.println("...Done.");
}

void calibrateHandler()
{
  Serial.print("Calibration routine to go here...");
  hx711A.set_scale(2);
  hx711A.tare();
}

void setup()
{
  Serial.begin(9600);
  // Wait until a serial monitor comes online
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Starting...");
  Serial.println("Data queue length is ");
  Serial.println(forceQueueLen);

  // Initialize the force sensor
  hx711A.begin(HX711_A_DOUT, HX711_A_SCK);
  hx711A.tare(20); // Tare with 20 readings (default is 10)
  hx711A.set_scale(3);

  Serial.println("Finished initializing load cell.");

  // Tare button setup: second parameter is false because we have a pullup on this button
  pinMode(TARE_PIN, INPUT_PULLUP);
  tareButton.attachClick(tareHandler);
  tareButton.attachLongPressStart(calibrateHandler);

  // Initialize the screen
  tft.begin();
  tftX = tft.height();                        // The X direction is the hardware's height
  tftY = tft.width();                         // The Y direction is the hardware's width
  graphX = yLabelMargin;                      // Graph location (lower left)
  graphY = tftY - xLabelMargin;               // Graph location (lower left)
  graphW = tftX - yLabelMargin - rightMargin; // Graph width (X)
  graphH = tftY - xLabelMargin - titleMargin; // Graph height (Y)
  char msg[50];
  tft.setRotation(3);
  sprintf(msg, "Initialized %dx%d TFT screen.", tft.width(), tft.height());
  Serial.println(msg);
  tft.fillScreen(BLACK); // This makes NOISE and screws up the next HX711 reading
  delay(350); // Wait for the noise to decay away.

}

// Read data and throw it at the screen forever
void loop(void)
{
  // Check the tare button
  tareButton.tick();

  // Start over from X = 0 when time falls off the right side of the graph:
  if (t > xLabelHi)
  {
    t_offset += t; // Remember how many seconds have elapsed so far
    t = 0;         // Reset the timer
    Serial.println("Starting graph over!");
    tft.fillScreen(BLACK); // Clear the screen and redraw axes/labels - generates NOISE that hampers the next HX11 reading
    delay(300); // Let the noise decay away.
    xLabelsDraw = true;
    yLabelsDraw = true;
  }

  // Get smoothed value from the dataset:
  if (hx711A.is_ready())
  {
    // All of the time-based logic will blow up when millis() overflows (~49 days).
    // TODO: Fix that.
    if (millis() > (((t + t_offset) * 1000) + dataInterval))
    {
      // Pack the float into a 16-bit int with 1 place after the decimal ( * 10)
      iForce = round(hx711A.get_units());
      fForce = iForce / 10;                    // Make the float version (1 place after the decimal)
      t = ((float)millis()) / 1000 - t_offset; // Repeating elapsed time in seconds
      // Store this value in the queue, making room first if needed
      if (forceQueueA.isFull())
      {
        forceQueueA.drop();
      }
      forceQueueA.push(&iForce);
      Serial.print("Load cell output: ");
      Serial.print(t);
      Serial.print(", ");
      forceQueueA.peekPrevious(&qForce); // Peek at the record we just put on the Queue
      Serial.print((float)qForce / 10);
      Serial.print(", ");
      forceQueueA.peek(&qForce); // Peek at the the record we can peek at (depends on LIFO/FIFO?)
      Serial.println((float)qForce / 10);
      

      // Draw a line on the TFT between the last (t, force) point and the current one.
      Graph(tft, t, fForce, graphX, graphY, graphW, graphH, xLabelLo, xLabelHi, xIncr,
            yLabelLo, yLabelHi, yIncr, title, xTitle, yTitle,
            DKBLUE, RED, YELLOW, WHITE, BLACK, xLabelsDraw, yLabelsDraw);
    }
  }
}
