/*
Copyright 2021, Photonsters
Jarrod Smith [MakerMatrix]

Licensed under the Apache License
https://www.apache.org/licenses/LICENSE-2.0.txt

This Graph() function is customized from Kris Kasprzak's here:
https://github.com/KrisKasprzak/GraphingFunction/blob/master/Graph.ino

*/

#include <Adafruit_ILI9341.h>

float ox, oy; // old x, y coords

// Initialize these next five vars in main.cpp, after instantiating the ILI9341 object:
float tftX;   // TFT resolution in X direction
float tftY;   // TFT resoluiton in Y direction
float graphY; // Graph location (lower left)
float graphW; // Graph height
float graphH; // Graph height

// Declare/set/pre-compute some graph parameters:
float titleMargin = 20.;     // Title space height in pixels
boolean yLabelsDraw = true;  // Draw the Y labels or not?
float yLabelMargin = 30.;    // Y label space in pixels
float yLabelLo = -100.;      // Lower Y label (axis units)
float yLabelHi = 100.;       // Upper Y label (axis units)
float yIncr = 25;            // Y label increment (axis units)
boolean xLabelsDraw = true;  // Draw the X labels or not?
float xLabelMargin = 25.;    // X label space in pixels
float xLabelLo = 0.;         // Lower X label (axis units)
float xLabelHi = 5.;         // Upper X label (axis units)
float xIncr = 1.;            // X label increment (axis units)
float rightMargin = 5.;      // Space to right of graph in pixels
float graphX = yLabelMargin; // Graph location (lower left)
String title = "Load Cell A";
String yTitle = "Force";
String xTitle = "Time (m)";

// Forward declaration for Graph(), if you need it:
// void Graph(Adafruit_ILI9341 &d, float x, float y, float gx, float gy, float w, float h,
//         float xlo, float xhi, float xinc, float ylo, float yhi, float yinc, String title,
//         String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor,
//         unsigned int tcolor, unsigned int bcolor, boolean &xLabelsDraw, boolean &yLabelsDraw);

/*

  function to draw a cartesian coordinate system and plot whatever data you want
  just pass x and y and the graph will be drawn

  huge arguement list
  &d name of your display object
  x = x data point
  y = y datapont
  gx = x graph location (lower left)
  gy = y graph location (lower left)
  w = width of graph
  h = height of graph
  xlo = lower bound of x axis
  xhi = upper bound of x asis
  xinc = division of x axis (distance not count)
  ylo = lower bound of y axis
  yhi = upper bound of y asis
  yinc = division of y axis (distance not count)
  title = title of graph
  xlabel = x axis label
  ylabel = y axis label
  gcolor = graph line colors
  acolor = axi ine colors
  pcolor = color of your plotted data
  tcolor = text color
  bcolor = background color
  &redraw = flag to redraw graph on fist call only
*/

void Graph(Adafruit_ILI9341 &d, float x, float y, float gx, float gy, float w, float h,
           float xlo, float xhi, float xinc, float ylo, float yhi, float yinc, String title,
           String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor,
           unsigned int tcolor, unsigned int bcolor, boolean &xLabelsDraw, boolean &yLabelsDraw)
{
    float f;
    float temp;

    if (yLabelsDraw)
    {
        yLabelsDraw = false; // Just draw the axis/labels one time
        oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
        // draw y scale
        for (f = ylo; f <= yhi; f += yinc)
        {
            // compute the transform
            temp = (f - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
            if (f == 0)
            {
                d.drawLine(gx, temp, gx + w, temp, acolor);
            }
            else
            {
                d.drawLine(gx, temp, gx + w, temp, gcolor);
            }
            d.setTextSize(1);
            d.setTextColor(tcolor, bcolor);
            d.setCursor(gx - 30, temp);
            // precision is default Arduino--this could really use some format control
            d.println(int(f));
        }
        // Draw the Title
        d.setTextSize(2);
        d.setTextColor(tcolor, bcolor);
        d.setCursor(gx, 0);
        d.println(title);
        // Draw Y Labels
        d.setTextSize(1);
        d.setTextColor(acolor, bcolor);
        d.setRotation(2);
        d.setCursor(tftY / 2 - 10, yLabelMargin - 10);

        d.println(ylabel);
        d.setRotation(3);
    }

    if (xLabelsDraw)
    {
        ox = (x - xlo) * (w) / (xhi - xlo) + gx;
        xLabelsDraw = false; // Just draw that axis/labels one time
        // draw x scale
        for (f = xlo; f <= xhi; f += xinc)
        {
            // compute the transform
            temp = (f - xlo) * (w) / (xhi - xlo) + gx;
            if (f == 0)
            {
                d.drawLine(temp, gy, temp, gy - h, acolor);
            }
            else
            {
                d.drawLine(temp, gy, temp, gy - h, gcolor);
            }
            d.setTextSize(1);
            d.setTextColor(tcolor, bcolor);
            d.setCursor(temp - 2, gy + 3);
            // precision is default Arduino--this could really use some format control
            d.println(int(f));
        }
        d.setTextSize(1);
        d.setTextColor(acolor, bcolor);
        d.setCursor((tftX / 2) - gx / 2, tftY - 10);
        d.println(xlabel);
    }

    //graph drawn now plot the data
    // the entire plotting code are these few lines...
    x = (x - xlo) * (w) / (xhi - xlo) + gx;
    y = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
    d.drawLine(ox, oy, x, y, pcolor);
    d.drawLine(ox, oy + 1, x, y + 1, pcolor);
    d.drawLine(ox, oy - 1, x, y - 1, pcolor);
    ox = x;
    oy = y;
} // End of graphing function