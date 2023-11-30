#include <Arduino.h>
#include "setup.h"

extern "C" {
    #include <hardware/flash.h>
    #include <hardware/sync.h>
}


// Find which page of flash holds the latest calibration value (RP2040-specific)
int getCalibrationPage()
{
    unsigned short page;
    int *p, addr;

    // Loop over all pages in the last sector of the RP2040 flash
    for( page = 0; page < FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE; page++){
        addr = page * FLASH_PAGE_SIZE + FLASH_TARGET_OFFSET + XIP_BASE;
        // The calibration value will be in int stored at the first four bytes
        p = (int *)addr; 
        if( DEBUG == 2){
            Serial.print("Calibration value at page " + String(page, DEC) );
            Serial.print(" (address 0x" + (String(int(p), HEX)) + ") = ");
            Serial.println(*p);
        }
        // The first four bytes will be -1 if the values are 0xFFFFFFFF...
        if( *p == -1){
            if( DEBUG == 2){
                Serial.println( "Latest calibration found at page #" + String(page-1, DEC));
            }
            return( page-1);
        }
    }
    // If we get here, the calibration page must have been at the very last page in the sector
    return( page);
}

// Read the latest calibration value (RP2040-specific)
int getCalibration()
{
    int page, *p, addr;

    page = getCalibrationPage();
    // Check that there *was* a calibration value found
    if( page < 0){
        if( DEBUG == 2){
            Serial.println( "No calibration values found, using the default of " + String(DEFAULT_CALIBRATION, DEC));
        }
        return(DEFAULT_CALIBRATION);
    }else{
        addr = page * FLASH_PAGE_SIZE + FLASH_TARGET_OFFSET + XIP_BASE;
        p = (int *)addr;
        if( DEBUG == 2){
            Serial.println( "Read calibration from page " + String(page, DEC) + ": " + String(*p, DEC));
        }
        return( *p);
    }
}

// Store new calibration value (RP2040-specific)
void writeCalibration(int cal)
{
    int emptypage;
    int buf[FLASH_PAGE_SIZE/sizeof(int)]; // One page worth of ints

    emptypage = getCalibrationPage() + 1; // Write to the next empty page of the sector
    
    // If the sector is full, we must erase it and start over
    if( emptypage == 16){    
        if(DEBUG == 2){
            Serial.println("Full sector, erasing...");
        }
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
        restore_interrupts (ints);
        emptypage = 0; // Now let us write to the first page
    }

    if(DEBUG == 2){
        Serial.println("Writing to page #" + String(emptypage, DEC));
    }

    *buf = cal;  // Put the calibration value at the first four bytes of buf
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(FLASH_TARGET_OFFSET + (emptypage*FLASH_PAGE_SIZE), (uint8_t *)buf, FLASH_PAGE_SIZE);
    restore_interrupts (ints);
}