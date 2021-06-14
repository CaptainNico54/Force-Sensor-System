#include <Arduino.h>
#include <HX711.h>
#include <OneButton.h>
#include <TFT_ILI9341.h>
#include <TFT_Charts.h>
#include "setup.h"

// Global variables to accumulate all-time (between tares) mean force, and handle button state
boolean taring = false;      // Taring button activated?
boolean calibrating = false; // Calibration button activated?
boolean done;                // Done calibrating?

// External globals for the chart object
extern TFT_ILI9341 tft;
extern ChartXY xyChart;
extern float fMean, allTimeSum, allTimeSamples;

OneButton tareButton(TARE_PIN, INPUT); // OneButton constructor | the button is pulled down by default
HX711 hx711;                           // HX711 constructor

// This is what happens when you short-press the tare button
void tareHandler()
{
    taring = true;
}

// This is what happens when you long-press the tare button
void calibrateHandler()
{
    calibrating = true;
}

// This is what happens when you double-click the tare button
void endHandler()
{
    done = true;
}

// This should happen when taring == true
void doTare()
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

// This should happen when calibrating == true
void doCalibration(TFT_ILI9341 &tft)
{

    // Initialize the double-click variable to false
    done = false;

    float calFactor, calSum, calAvg, kgForce;
    int converged = 0;

    if (DEBUG)
    {
        Serial.println("Calibrating...");
    }

    tft.fillScreen(xyChart.tftBGColor);
    tft.drawRect(55, 3, 206, 33, YELLOW);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.setCursor(60, 10);
    tft.println("CALIBRATION");
    tft.setTextSize(2);

    tft.setCursor(0, 60);
    tft.println(" Carefully hang a 1.0kg\n mass from the build plate");
    tft.setCursor(0, 100);
    tft.setTextColor(DKGREEN);
    tft.println(" Then, double-click the\n tare button to calibrate");

    while (!done) // Wait for the user to setup the reference mass
    {
        tareButton.tick(); // We are looking for a double-click
        delay(10);
    }

    // Clear the double-click message and inform the user what's happening
    tft.fillRect(0, 100, tft.width(), tft.height(), xyChart.tftBGColor);
    tft.setCursor(0, 100);
    tft.setTextColor(RED);
    tft.println("\n CALIBRATING, DO NOT TOUCH");
    tft.print("  .");

    kgForce = calSum = 0;
    calFactor = 1;

    // Calibration scale minimizer
    while (converged < 10)
    {
        if (hx711.is_ready())
        {
            hx711.set_scale(calFactor);
            kgForce = hx711.get_units(10);              // Get a sample -> we are trying to make this value be 1.00
            calFactor = calFactor * kgForce / 1000;            // Multiply the calibration factor by the get_units return value
            if ((kgForce < 1005) && (kgForce > 995)) // Did this calibration factor produce a 1kg +/- 0.5% output?
            {
                converged++;         // We are one count closer to convergence
                calSum += calFactor; // Build a sum so we can average the successful values later
                if (DEBUG)
                {
                    Serial.print("Success #");
                    Serial.println(converged);
                }
            }

            tft.print(".");

            if (DEBUG)
            {
                Serial.print("get_units returned ");
                Serial.println(kgForce);
                Serial.print("Trying new calFactor = ");
                Serial.println(calFactor);
            }
        }
    }

    calAvg = calSum / converged; // The average of all the converged values will be our actual scale
    hx711.set_scale(calAvg);
    tft.fillRect(0, 100, tft.width(), tft.height(), xyChart.tftBGColor);
    tft.setTextColor(YELLOW);
    tft.setCursor(0, 100);
    tft.print("\n Calibration Converged!\n Constant = ");
    tft.println(round(calAvg));
    delay(2000);
    hx711.tare(20);
    delay(5000);

    calibrating = false;
}