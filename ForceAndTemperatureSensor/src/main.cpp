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
#include <HX711.h>
#include <OneButton.h>
#include "setup.h"

// Initialize some global variables
float fMean, allTimeSum, allTimeSamples, force;
float t, lastT, t_offset = 0; // Keep track of time
float temperature;

// Global external variables
extern boolean taring;      // Taring button activated?
extern boolean calibrating; // Calibration button activated?
extern boolean done;        // Done calibrating?

// Button and ADC objects
extern OneButton tareButton; // OneButton constructor
extern HX711 hx711;          // HX711 constructor

void setup()
{
  uint8_t hx711Cal;

  // Attempt to reduce ADC noise (https://youtu.be/F0o1C7nEgw0?si=KocGF8g-lJ3rrYsT&t=449)
  // digitalWrite(23, HIGH);
    
  if (DEBUG)
  {
    Serial.begin(9600);
    // Wait until a serial monitor comes online
    while (!Serial)
      ;
    if( DEBUG == 2){
      Serial.println();
      Serial.println("Starting...");
    }    
  }

  fMean = allTimeSum = allTimeSamples = 0; // Initialize fMean

  // Initialize the force sensor
  hx711.begin(HX711_DOUT, HX711_SCK);
  hx711.tare(20); // Tare with 20 readings (default is 10)

  // Initialize the RP2040 internal temperature sensor
  temperature_init();

#ifdef OVERRIDE_CALIBRATION
  hx711Cal = OVERRIDE_CALIBRATION;
  if (DEBUG == 2)
  {
    Serial.println("Calibration overridden with value " + String(hx711Cal));
  }
#else
  hx711Cal = getCalibration();
#endif

  hx711.set_scale(hx711Cal);

  if (DEBUG == 2)
  {
    Serial.println("Finished initializing load cell.");
  }

  // Tare button setup:
  pinMode(TARE_PIN, INPUT_PULLUP);
  tareButton.attachClick(tareHandler);
  tareButton.attachLongPressStart(calibrateHandler);
  tareButton.attachDoubleClick(endHandler);
}

// Read data and throw it out the serial port forever
void loop(void)
{
  //Check the serial port for input, to trigger a tare event
  if( Serial.available())
  {
    char serdata = Serial.read();  
    if( serdata == '1')
    {
      serdata = (char) 0;
      doTare(0);
    }
  }

  tareButton.tick(); // Check the tare button

  // Single click
  if (taring)
  {
    taring = false;
    doTare(1);
  }

  // Long press
  if (calibrating)
  {
    done = false;
    doCalibration();

    // Reset stats
    lastT = allTimeSamples = allTimeSum = fMean = 0;
  }

  // Get smoothed value from the dataset:
  if (hx711.is_ready())
  {
    // All of the time-based logic will blow up when millis() overflows (~49 days).
    //Serial.print(millis());
    //Serial.print(", ");
    //Serial.println(lastT);
    if (millis() > ((lastT * 1000) + DATA_INTERVAL))
    {
      force = hx711.get_units();                   // Read the load cell value as a float (4 bytes on 8-bit AVRs)
      t = (float(millis()) / 1000 - t_offset); // Elapsed time in seconds.  (Why must I cast millis() here?)

      // Check for an outlier (presumed glitch/noise)
      if (fabs(force) > 15000)
      {
        if (DEBUG == 2)
        {
          Serial.print("DETECTED OUTLIER " + String(force) + " - IGNORING THIS VALUE.");
        }
        force = fMean;
      }

      allTimeSamples += 1;
      allTimeSum += force;
      fMean = allTimeSum / allTimeSamples;
            
      // Read the temperature from the RP2040
      temperature = temperature_read();

      // Report the current [time, force] value
      if (DEBUG == 2)
      {
        Serial.print("\nCurrent time, force point, temperature: ");
        Serial.print(t);
        Serial.print(", ");
        Serial.print(force);
        Serial.print(", ");
        Serial.println(temperature);
      }

      if (DEBUG == 1)
      {
        // char buf[10];
        // sprintf(buf, "%8.1f", force);
        Serial.print(force);
        Serial.print(",");
        Serial.print(temperature);
        Serial.println();
      }
      lastT = t;
    }
  }
}