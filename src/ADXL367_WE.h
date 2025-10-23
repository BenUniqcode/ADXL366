/******************************************************************************
 *
 * This is a library for the ADXL366 / ADXL367 accelerometer.
 *
 * You'll find several example sketches which should enable you to use the library. 
 *
 * You are free to use it, change it or build on it. In case you like it, it would
 * be cool if you give it a star.
 *
 * If you find bugs, please inform me!
 * 
 * Written by Wolfgang (Wolle) Ewald
 * https://wolles-elektronikkiste.de/ADXL366-teil-1 (German)
 * https://wolles-elektronikkiste.de/en/ADXL366-the-universal-accelerometer-part-1 (English)
 *
 * 
 ******************************************************************************/

#ifndef ADXL367_WE_H_
#define ADXL367_WE_H_

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "ADXL366_WE.h"
#include <Wire.h>
#include <SPI.h>
#include "xyzFloat.h"

class ADXL367_WE : public ADXL366_WE {
    public:
        using ADXL366_WE::ADXL366_WE;               
};



#endif


