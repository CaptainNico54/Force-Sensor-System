#include <Arduino.h>
#include "setup.h"

extern "C" {
    #include <hardware/adc.h>
}

void temperature_init()
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
}

// See https://learnembeddedsystems.co.uk/using-the-rp2040-on-board-temperature-sensor
float temperature_read()
{
    uint16_t raw = adc_read();
    const float conversion_factor = 3.3/(1<<12);
    float result = raw * conversion_factor;
    float temperature = 27 - (result - VBE) / DEG_PER_V;
    return (temperature);
}