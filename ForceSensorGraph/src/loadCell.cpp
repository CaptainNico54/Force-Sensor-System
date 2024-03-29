#include <Arduino.h>
#include <HX711.h>
#include <EEPROM.h>
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
extern float fMean, allTimeSum, allTimeSamples, t_offset;

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
    delay(1000);  // Let things settle for 1s before reading.
    hx711.tare(); // Take the tare reading (10 samples is default)
    if (DEBUG)
    {
        Serial.println("...Done.");
    }
    fMean = allTimeSum = allTimeSamples = 0; // Reset the legend stats
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
    tft.setCursor(0, 120);
    tft.setTextColor(GREEN);
    tft.println(" Then, double-click the\n tare button to calibrate");

    while (!done) // Wait for the user to setup the reference mass
    {
        tareButton.tick(); // We are looking for a double-click
        delay(10);
    }

    // Clear the double-click message and inform the user what's happening
    tft.fillRect(0, 120, tft.width(), tft.height(), xyChart.tftBGColor);
    tft.setCursor(0, 120);
    tft.setTextColor(RED);
    tft.println(" CALIBRATING, DO NOT TOUCH");
    tft.print("  .");

    kgForce = calSum = 0;
    calFactor = 1;

    // Calibration scale minimizer/de-noiser
    while (converged < 10)
    {
        if (hx711.is_ready())
        {
            hx711.set_scale(calFactor);
            kgForce = hx711.get_units(10);           // Get a sample -> we are trying to make this value be 1000g
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
            // Normalize the calibration factor by the get_units() return value and reference mass
            calFactor = calFactor * kgForce / REFERENCE_MASS;
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
    tft.fillScreen(xyChart.tftBGColor);
    tft.drawRect(55, 3, 206, 33, YELLOW);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.setCursor(60, 10);
    tft.println("CALIBRATION");
    tft.setTextSize(2);

    tft.setCursor(0, 60);
    tft.print("\n Calibration Converged!\n Constant = ");
    tft.println(round(calAvg));

    EEPROM.write(EEPROM_ADDR, round(calAvg));
    if (DEBUG == 2)
    {
        Serial.print("Wrote calibration value = " + String(round(calAvg)) + " to EEPROM address " + String(EEPROM_ADDR));
    }
    
    delay(5000); // Let the user read the last onscreen msg

    calibrating = false; // We are done, reset the calibration flag
    initChart();         // Reinitialize the chart
}