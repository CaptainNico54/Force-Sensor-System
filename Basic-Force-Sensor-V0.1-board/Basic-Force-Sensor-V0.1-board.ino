#include "HX711.h"
#include "hwb.hpp"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = A1;
const int LOADCELL_SCK_PIN = A0;
const int DATA_RATE_PIN = 11;
const int DATA_RATE = LOW;  // LOW for 80hz, High for 10hz)
const int SCALE_OFFSET = 54; // Calibration for weight


//Button
bool isTarePressed() {
  return readHwb();
}

HX711 scale;

void setup() {
  Serial.begin(38400);
  while (!Serial) { }
  pinMode(DATA_RATE_PIN, OUTPUT);    // sets the digital pin 11 as output
  digitalWrite(DATA_RATE_PIN, DATA_RATE);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(SCALE_OFFSET); // 
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0\
  setupHwbInput( true );
  
}

void loop() {

  if(Serial.available()){ // zeros scale when z is pressed in the Java Script
    char temp = Serial.read();
    if(temp == '1')
      scale.tare();
}
 else if(isTarePressed()){
    scale.tare();
}

  Serial.println(scale.get_units(2));
  delay(1);        // delay for stability
  

}
