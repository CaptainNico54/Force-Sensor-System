#include <TFT_Charts.h>
#include <TFT_ILI9341.h>
#include <cppQueue.h>
#include "setup.h"

extern cppQueue fQ;
extern ChartXY xyChart;
extern TFT_ILI9341 tft;

void initChart()
{
  // Initialize the screen
  tft.begin();
  xyChart.begin(tft);

// Invert the screen if requested in setup.h
#ifdef FLIP_TFT
  tft.setRotation(1);
#endif

  tft.fillScreen(xyChart.tftBGColor);
  xyChart.setAxisLimitsX(0, XRANGE, XTICKTIME);
  xyChart.setAxisLimitsY(-100, 100, 25);
  xyChart.drawTitleChart(tft, "Z-Axis Force");
  xyChart.drawAxisX(tft, 10);
  xyChart.drawAxisY(tft, 10);
  xyChart.drawLabelsX(tft);
  xyChart.drawLabelsY(tft);
  xyChart.drawY0(tft);
  delay(400); // Let noise settle

  if (DEBUG == 2)
  {
    xyChart.tftInfo();
  }

  // Flush the queue if there is already data there
  if (fQ.nbRecs())
  {
    if (DEBUG == 2)
    {
      Serial.println("Flushing the queue...");
    }
    fQ.flush();
  }

  // Seed the queue with the origin coords
  ChartXY::point p;
  p.x = 0.;
  p.y = 0.;
  fQ.push(&p);
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

  for (i = 1; i <= QUEUE_LENGTH; i++)
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

  if (DEBUG == 2)
  {
    Serial.println(reason);
    Serial.println("Current Y limits: " + String(xyChart.yMin) + ", " + String(xyChart.yMax));
    Serial.println("Scaling Y to range " + String(yMin) + ", " + String(yMax));
  }
  tft.fillScreen(xyChart.tftBGColor);
  xyChart.setAxisLimitsY(yMin, yMax, (yMax - yMin) / 8);
  xyChart.drawAxisY(tft, 10);
  xyChart.drawLabelsY(tft);
  xyChart.drawAxisX(tft, 10);
  xyChart.drawLabelsX(tft);
  xyChart.drawY0(tft);
  xyChart.drawTitleChart(tft, "Load Cell A");
  return (true);
}

// Scale the Y axis if needed
boolean autoScale(ChartXY::point mm, ChartXY::point p)
{
  float fMin = mm.x;
  float fMax = mm.y;
  float expandFactor = 7;  // Determines the default Y range in relation to the current min/max values when expanding limits
  float limitFactor = 0.1; // Determines the additional Y range added to the current min/max values when expanding limits
  boolean scaled = false;

  if (DEBUG == 2)
  {
    Serial.println("\nCheck limits: Current Y value is " + String(p.y));
    Serial.println("fMin = " + String(fMin) + ", fMax = " + String(fMax));
  }

  if (p.y < xyChart.yMin)
  {
    scaled = scaleY(p.y - (limitFactor * fabs(p.y)), xyChart.yMax, "New Y value less than yMin");
    xyChart.drawTitleChart(tft, "Z-Axis Force");
  }
  else if (p.y > xyChart.yMax)
  {
    scaled = scaleY(xyChart.yMin, p.y + (limitFactor * fabs(p.y)), "New Y value more than yMax");
    xyChart.drawTitleChart(tft, "Z-Axis Force");
  }
  // Expansion heuristic (make the data fill at least 1/expandFract of the y range )
  else if ((xyChart.yMax - xyChart.yMin) > (expandFactor * (fMax - fMin)))
  {
    float yRange = fMax - fMin;
    float yMid = fMin + (yRange / 2);
    float yLimit = yRange * (expandFactor - 2) / 2;
    scaled = scaleY(yMid - yLimit, yMid + yLimit, "Y limits too large relative to Y range");
    xyChart.drawTitleChart(tft, "Z-Axis Force");
  }
  // Centering heuristic (keep y range centered to within 40% of the y limits)
  else if (fabs((xyChart.yMax - fMax) - (fMin - xyChart.yMin)) > fabs(0.4 * (fMax - fMin)))
  {
    float yRange = fMax - fMin;
    float yMid = fMin + (yRange / 2);
    float yLimit = (xyChart.yMax - xyChart.yMin);
    scaled = scaleY(yMid - (yLimit / 2), yMid + (yLimit / 2), "Y range not centered");
    xyChart.drawTitleChart(tft, "Z-Axis Force");
  }
  return (scaled);
}