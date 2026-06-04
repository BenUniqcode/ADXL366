/**
 * Arduino library for the Analog Devices ADXL366 / ADXL367 accelerometer
 * Adapted by Ben Wheeler <ben@uniqcode.com>
 * from ADXL366_WE by Wolfgang (Wolle) Ewald https://github.com/wollewald/ADXL366_WE
 * Released under the MIT License.
 */

#include "ADXL366_WE.h"

/************ Basic settings ************/
    
bool ADXL366_WE::init(bool startMeasuring){    
    if(useSPI){
        if(mosiPin == 999){
            _spi->begin();
        }
#ifdef ESP32
        else {
            _spi->begin(sckPin, misoPin, mosiPin, csPin);
        }
#endif
#ifdef ARDUINO_ARCH_STM32
        else {
            _spi->setMISO(misoPin);
            _spi->setMOSI(mosiPin);
            _spi->setSCLK(sckPin);
            _spi->begin();
        }
#endif
        setSPIClockSpeed(spiClock);
        pinMode(csPin, OUTPUT);
        digitalWrite(csPin, HIGH);
    } else {
        // Using I2C
        // If I2C is not configured for High Speed, clear the I2C_HS bit in FILTER_CTL
        // As the ADXL366 boots up in High Speed mode, we can't rely on being able to read the
        // existing value. But we are resetting anyway.
        uint32_t i2cClock = _wire->getClock();
        if (i2cClock < 400000L) {
            Serial.printf("I2C clock is %u, disabling High Speed mode on ADXL\n", i2cClock);
            writeRegister(ADXL366_FILTER_CTL, 0x20);
        }
    }

    // Check that the device is present and communicating, by reading the first 4 registers
    // The first 3 are the same for both the 366 and 367 - they differ only by the fourth.
    uint8_t devid[4];
    bool ok = readMultipleRegisters(ADXL366_DEVID_AD, 4, devid);

    if (!ok || devid[0] != 0xad || devid[1] != 0x1d || devid[2] != 0xf7) {
        // If we didn't get a response, try a soft reset
        Serial.printf("Invalid device ID: Found 0x%02x%02x%02x, expected 0xad1df7 - trying soft reset\n", devid[0], devid[1], devid[2]);
        // Trigger a soft reset and wait 20ms
        softReset();
        // Try the read again
        ok = readMultipleRegisters(ADXL366_DEVID_AD, 4, devid);
        if (!ok || devid[0] != 0xad || devid[1] != 0x1d || devid[2] != 0xf7) {
            Serial.printf("Invalid device ID: Found 0x%02x%02x%02x, expected 0xad1df7 - giving up\n", devid[0], devid[1], devid[2]);
            return false;
        }
    }
    // Rev number: 0x03 = ADXL367, 0x05 = ADXL366
    if (devid[3] == 0x03) {
        Serial.println("Found device ADXL367");
    } else if (devid[3] == 0x05) {
        Serial.println("Found device ADXL366");
    } else {
        Serial.printf("Found unknown ADXL device revision number 0x%02x\n", devid[3]);
    }

    // Disable all interrupts
    writeRegister(ADXL366_INTMAP1_LOWER, 0);
    writeRegister(ADXL366_INTMAP2_LOWER, 0);
    writeRegister(ADXL366_INTMAP1_UPPER, 0);
    writeRegister(ADXL366_INTMAP2_UPPER, 0);
    // Clear any extant interrupts
    readAndClearInterrupts();

    // Start measure mode unless caller asked not to
    if (startMeasuring) {
        setMeasureMode(true);
    }
    return true;
}

void ADXL366_WE::setSPIClockSpeed(unsigned long clock = 5000000){
    spiClock = clock;
    mySPISettings = SPISettings(spiClock, MSBFIRST, SPI_MODE3);
}

void ADXL366_WE::setCorrFactors(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax){
    const float UNITS_PER_G = 1000.0 / (rangeFactor * MILLI_G_PER_LSB);
    corrFact.x = UNITS_PER_G / (0.5 * (xMax - xMin));
    corrFact.y = UNITS_PER_G / (0.5 * (yMax - yMin));
    corrFact.z = UNITS_PER_G / (0.5 * (zMax - zMin));
    offsetVal.x = (xMax + xMin) * 0.5;
    offsetVal.y = (yMax + yMin) * 0.5;
    offsetVal.z = (zMax + zMin) * 0.5;
}

bool ADXL366_WE::setDataRate(adxl366_dataRate rate){
    // ODR = bottom 3 bits of FILTER_CTL
    if (!readRegister8(ADXL366_FILTER_CTL, &regVal) || rate == ADXL366_DATA_RATE_ERROR) {
        return false;
    }
    regVal &= ~0x07;
    regVal |= rate;
    writeRegister(ADXL366_FILTER_CTL, regVal);
    return true;
}
    
adxl366_dataRate ADXL366_WE::getDataRate(){
    // ODR = bottom 3 bits of FILTER_CTL
    if (!readRegister8(ADXL366_FILTER_CTL, &regVal)) {
        return ADXL366_DATA_RATE_ERROR;
    }
    return static_cast<adxl366_dataRate>(regVal & 0x07);
}


String ADXL366_WE::getDataRateAsString(){
    adxl366_dataRate dataRate = getDataRate();    
    switch(dataRate) {
        case ADXL366_DATA_RATE_ERROR: return(F("ERROR")); break;
        case ADXL366_DATA_RATE_400:  return(F("400 Hz"));  break;
        case ADXL366_DATA_RATE_200:  return(F("200 Hz"));  break;
        case ADXL366_DATA_RATE_100:  return(F("100 Hz"));  break;
        case ADXL366_DATA_RATE_50:   return(F("50 Hz"));   break;
        case ADXL366_DATA_RATE_25:   return(F("25 Hz"));   break;
        case ADXL366_DATA_RATE_12_5: return(F("12.5 Hz")); break;
        default: return(F("unknown"));
    }
}

bool ADXL366_WE::setRange(adxl366_range range){
    if (!readRegister8(ADXL366_FILTER_CTL, &regVal) || range == ADXL366_RANGE_ERROR) {
        return false;
    }
    // The 366 is always 14-bit, so the range factor varies
    switch(range){
        case ADXL366_RANGE_ERROR: return false; break; // Already handled, but avoids compiler warning
        case ADXL366_RANGE_2G:  rangeFactor = 1.0;  break;
        case ADXL366_RANGE_4G:  rangeFactor = 2.0;  break;
        case ADXL366_RANGE_8G:  rangeFactor = 4.0;  break;
    }
    regVal &= 0x3f;
    regVal |= range << 6;
    writeRegister(ADXL366_FILTER_CTL, regVal);
    return true;
}

adxl366_range ADXL366_WE::getRange(){
    if (!readRegister8(ADXL366_FILTER_CTL, &regVal)) {
        return ADXL366_RANGE_ERROR;
    }
    regVal = regVal >> 6;
    return static_cast<adxl366_range>(regVal);
}

String ADXL366_WE::getRangeAsString(){
    adxl366_range range = getRange();
    switch(range){
        case ADXL366_RANGE_ERROR: return(F("ERROR")); break;
        case ADXL366_RANGE_2G:  return(F("2g"));   break;
        case ADXL366_RANGE_4G:  return(F("4g"));   break;
        case ADXL366_RANGE_8G:  return(F("8g"));   break;
        default: return(F("unknown"));
        
    }
}

// Left for backwards compatibility, but on the 366 it's better to check the subsequent registers;
// see the check in init()
uint8_t ADXL366_WE::getDeviceID(){
    if (readRegister8(ADXL366_DEVID_AD, &regVal)) {
        return regVal;
    } else {
        return 0;
    }
}

/************ x,y,z results ************/

// Get 8-bit raw values for all axes with a single read. If you don't need full precision,
// this is faster than doing a full 14-bit read.
bool ADXL366_WE::getRawValues8(xyzFloat *rawVal) {
    uint8_t rawData[3]; 
    if (!readMultipleRegisters(ADXL366_XDATA, 3, rawData)) {
        return false;
    }
    rawVal->x = (rawData[0] << 6) * 1.0;
    rawVal->y = (rawData[1] << 6) * 1.0;
    rawVal->z = (rawData[2] << 6) * 1.0;
    return true;

}

bool ADXL366_WE::getRawValues(xyzFloat *rawVal){
    uint8_t rawData[6]; 
    if (!readMultipleRegisters(ADXL366_XDATA_H, 6, rawData)) {
        return false;
    }
    // These are 14-bit numbers. We must shift them such that the MSB is at the top bit when
    // casting to int16_t, so that it is used for the sign bit. 
    // Then we need to divide by 4 to get rid of the excess bits.
    rawVal->x = (static_cast<int16_t>((rawData[0] << 8) | rawData[1])) * 0.25;
    rawVal->y = (static_cast<int16_t>((rawData[2] << 8) | rawData[3])) * 0.25;
    rawVal->z = (static_cast<int16_t>((rawData[4] << 8) | rawData[5])) * 0.25;
    return true;
}

bool ADXL366_WE::getCorrectedRawValues(xyzFloat *rawVal){
    if (!getRawValues(rawVal)) {
        return false;
    }
    rawVal->x -= (offsetVal.x / rangeFactor);
    rawVal->y -= (offsetVal.y / rangeFactor);
    rawVal->z -= (offsetVal.z / rangeFactor);
    return true;
}

bool ADXL366_WE::getGValues(xyzFloat *gVal){
    if (!getCorrectedRawValues(gVal)) {
        return false;
    }
    *gVal *= corrFact * MILLI_G_PER_LSB * rangeFactor / 1000.0; 
    return true;
}

/************ Angles and Orientation ************/ 

bool ADXL366_WE::getAngles(xyzFloat *angleVal){
    xyzFloat gVal;
    if (!getGValues(&gVal)) {
        return false;
    }
    if(gVal.x > 1){
        gVal.x = 1;
    }
    else if(gVal.x < -1){
        gVal.x = -1;
    }
    angleVal->x = (asin(gVal.x)) * 57.296;
    
    if(gVal.y > 1){
        gVal.y = 1;
    }
    else if(gVal.y < -1){
        gVal.y = -1;
    }
    angleVal->y = (asin(gVal.y)) * 57.296;
    
    if(gVal.z > 1){
        gVal.z = 1;
    }
    else if(gVal.z < -1){
        gVal.z = -1;
    }
    angleVal->z = (asin(gVal.z)) * 57.296;
    return true;
}

bool ADXL366_WE::getCorrAngles(xyzFloat *corrAngleVal){
    if (!getAngles(corrAngleVal)) {
        return false;
    }
    *corrAngleVal -= angleOffsetVal;
    return true;
}

bool ADXL366_WE::measureAngleOffsets(){
    return getAngles(&angleOffsetVal);
}

xyzFloat ADXL366_WE::getAngleOffsets(){
    return angleOffsetVal;
}

void ADXL366_WE::setAngleOffsets(const xyzFloat aos){
    angleOffsetVal = aos;
}

adxl366_orientation ADXL366_WE::getOrientation(){
    adxl366_orientation orientation = FLAT;
    xyzFloat angleVal;
    if (!getAngles(&angleVal)) {
        return ADXL366_ORIENTATION_ERROR;
    }
    if(abs(angleVal.x) < 45){      // |x| < 45
        if(abs(angleVal.y) < 45){      // |y| < 45
            if(angleVal.z > 0){          //  z  > 0
                orientation = FLAT;
            }
            else{                        //  z  < 0
                orientation = FLAT_1;
            }
        }
        else{                         // |y| > 45 
            if(angleVal.y > 0){         //  y  > 0
                orientation = XY;
            }
            else{                       //  y  < 0
                orientation = XY_1;   
            }
        }
    }
    else{                           // |x| >= 45
        if(angleVal.x > 0){           //  x  >  0
            orientation = YX;       
        }
        else{                       //  x  <  0
            orientation = YX_1;
        }
    }
    return orientation;
}

String ADXL366_WE::getOrientationAsString(){
    adxl366_orientation orientation = getOrientation();
    String orientationAsString = "";
    switch(orientation){
        case ADXL366_ORIENTATION_ERROR: orientationAsString = "ERROR"; break;
        case FLAT:      orientationAsString = "z up";   break;
        case FLAT_1:    orientationAsString = "z down"; break;
        case XY:        orientationAsString = "y up";   break;
        case XY_1:      orientationAsString = "y down"; break;
        case YX:        orientationAsString = "x up";   break;
        case YX_1:      orientationAsString = "x down"; break;
    }
    return orientationAsString;
}

float ADXL366_WE::getPitch(){
    xyzFloat gVal;
    if (!getGValues(&gVal)) {
        return ADXL366_FLOAT_ERROR;
    }
    float pitch = (atan2(-gVal.x, sqrt(abs((gVal.y*gVal.y + gVal.z*gVal.z))))*180.0)/M_PI;
    return pitch;
}
    
float ADXL366_WE::getRoll(){
    xyzFloat gVal;
    if (!getGValues(&gVal)) {
        return ADXL366_FLOAT_ERROR;
    }
    float roll = (atan2(gVal.y, gVal.z)*180.0)/M_PI;
    return roll;
}

/************ Power, Sleep, Standby ************/ 

bool ADXL366_WE::setMeasureMode(bool measure){
    if (!readRegister8(ADXL366_POWER_CTL, &regVal)) {
        return false;
    }
    if(measure){
        regVal |= 0x02;
    }
    else{
        regVal &= ~0x02;
    }
    writeRegister(ADXL366_POWER_CTL, regVal);
    return true;
}

// bool ADXL366_WE::setSleep(bool sleep, adxl366_wUpFreq freq){
//     if (!readRegister8(ADXL366_POWER_CTL, &regVal)) {
//         return false;
//     }
//     if (freq != ADXL366_WUP_FQ_UNSET) {
//         regVal &= 0b11111100;
//         regVal |= freq;
//     }
//     if(sleep){
//         regVal |= (1<<ADXL366_SLEEP);
//     }
//     else{
//         // it is recommended to enter Stand Mode when clearing the Sleep Bit!
//         if (!setMeasureMode(false)) {
//             return false;
//         }
//         regVal &= ~(1<<ADXL366_SLEEP);
//         regVal &= ~(1<<ADXL366_MEASURE);
//     }
//     writeRegister(ADXL366_POWER_CTL, regVal);
//     if(!sleep){
//         setMeasureMode(true); // No return check here as the setting has been changed already
//     }
//     return true;
// }
    
// bool ADXL366_WE::setAutoSleep(bool autoSleep, adxl366_wUpFreq freq){
//     if (!readRegister8(ADXL366_POWER_CTL, &regVal)) {
//         return false;
//     }
//     if(autoSleep){
//         // Both AUTO_SLEEP and LINK bits must be set
//         regVal |= (1<<ADXL366_AUTO_SLEEP) | (1<<ADXL366_LINK);
//     } else {
//         // The AUTO_SLEEP bit is cleared, but leave the LINK bit alone in case set by something else
//         regVal &= ~(1<<ADXL366_AUTO_SLEEP);
//     }
//     if (freq != ADXL366_WUP_FQ_UNSET) {
//         regVal &= 0b11111100;
//         regVal |= freq;
//     }
//     writeRegister(ADXL366_POWER_CTL, regVal);
//     return true;
// }
        
bool ADXL366_WE::isAsleep(){
    if (!readRegister8(ADXL366_STATUS, &regVal)) {
        return false; // Not ideal
    }
    return !(regVal & (1<<ADXL366_INT_AWAKE));
}

// bool ADXL366_WE::setLowPower(bool lowpwr){
//     if (!readRegister8(ADXL366_BW_RATE, &regVal)) {
//         return false;
//     }
//     if(lowpwr){
//         regVal |= (1<<ADXL366_LOW_POWER);
//     }
//     else{
//         regVal &= ~(1<<ADXL366_LOW_POWER);
//     }
//     writeRegister(ADXL366_BW_RATE, regVal);
//     return true;
// }

// bool ADXL366_WE::isLowPower(){
//     if (!readRegister8(ADXL366_BW_RATE, &regVal)) {
//         return false; // Not ideal
//     }
//     return regVal & (1<<ADXL366_LOW_POWER);
// }
            
/************ Interrupts ************/

void ADXL366_WE::disableAllInterrupts()
{
    writeRegister(ADXL366_INTMAP1_LOWER, 0);
    writeRegister(ADXL366_INTMAP2_LOWER, 0);
    writeRegister(ADXL366_INTMAP1_UPPER, 0);
    writeRegister(ADXL366_INTMAP2_UPPER, 0);
}

bool ADXL366_WE::setInterrupt(adxl366_int type, uint8_t pin, bool setOn) {
    adxl366_register reg;
    if (pin == INT_PIN_1) {
        reg = (type > 7) ? ADXL366_INTMAP1_UPPER : ADXL366_INTMAP1_LOWER;
    } else {
        reg = (type > 7) ? ADXL366_INTMAP2_UPPER : ADXL366_INTMAP2_LOWER;
    }
    if (!readRegister8(reg, &regVal)) {
        return false;
    }
    // As we've now selected the correct INTMAP "bank", clear the bank select bit of the type
    // so it's just the bit number within this bank, then select that bit.
    uint8_t bitMask = 1U << (type & ~0x80);
    // Enable or disable the interrupt
    if (setOn) {
        regVal |= bitMask;
    } else {
        regVal &= ~bitMask;
    }
    writeRegister(reg, regVal);
    return true;
}

bool ADXL366_WE::setInterruptPolarity(uint8_t pol, uint8_t pin){
    if(!pin || pin == INT_PIN_1){
        if (!readRegister8(ADXL366_INTMAP1_LOWER, &regVal)) {
            return false;
        }
        regVal &= ~0x80;
        regVal |= (pol << 7);
        writeRegister(ADXL366_INTMAP1_LOWER, regVal);
    }
    if(!pin || pin == INT_PIN_2) {
        if (!readRegister8(ADXL366_INTMAP2_LOWER, &regVal)) {
            return false;
        }
        regVal &= ~0x80;
        regVal |= (pol << 7);
        writeRegister(ADXL366_INTMAP2_LOWER, regVal);
    }
    return true;
}

bool ADXL366_WE::deleteInterrupt(adxl366_int type, uint8_t pin){
    return setInterrupt(type, pin, false);
}

uint32_t ADXL366_WE::readAndClearInterrupts(){
    uint8_t status[3];
    if (!readMultipleRegisters(ADXL366_STATUS_COPY, 3, status)) {
        return 0; // Not ideal
    }
    uint32_t merged = 0;
    memcpy(&merged, status, 3); // Assumes Little Endian (LSB first)?
    return merged;
}

bool ADXL366_WE::checkInterrupt(uint32_t source, adxl366_int type){
    return source & (1<<type);
}

// bool ADXL366_WE::setLinkBit(bool link){
//     if (!readRegister8(ADXL366_POWER_CTL, &regVal)) {
//         return false;
//     }
//     if(link){
//         regVal |= (1<<ADXL366_LINK);
//     }
//     else{
//         regVal &= ~(1<<ADXL366_LINK);
//     }
//     writeRegister(ADXL366_POWER_CTL, regVal);
//     return true;
// }

// void ADXL366_WE::setFreeFallThresholds(float ffg, float fft){
//     regVal = static_cast<uint8_t>(round(ffg / 0.0625));
//     if(regVal<1){
//         regVal = 1;
//     }
//     writeRegister(ADXL366_THRESH_FF, regVal);
//     regVal = static_cast<uint8_t>(round(fft / 5));
//     if(regVal<1){
//         regVal = 1;
//     }
//     writeRegister(ADXL366_TIME_FF, regVal);
// }

bool ADXL366_WE::setActivityParameters(bool useReferenced, float threshold) {
    regVal = static_cast<uint8_t>(round(threshold / 0.0625));
    if(regVal<1){
        regVal = 1;
    }
    writeRegister(ADXL366_THRESH_ACT_H, regVal);

    if (!readRegister8(ADXL366_ACT_INACT_CTL, &regVal)) {
        return false;
    }
    regVal &= ~0x03;
    regVal |= 0x01;
    if (useReferenced) {
        regVal |= 0x02;
    }
    writeRegister(ADXL366_ACT_INACT_CTL, regVal);
    return true;
}

bool ADXL366_WE::setInactivityParameters(bool useReferenced, float threshold, uint16_t inactTime) {
    regVal = static_cast<uint8_t>(round(threshold / 0.0625));
    if(regVal<1){
        regVal = 1;
    }
    writeRegister(ADXL366_THRESH_INACT_H, regVal);
    writeRegister(ADXL366_TIME_INACT_H, inactTime >> 8);
    writeRegister(ADXL366_TIME_INACT_L, inactTime & 0xff);

    if (!readRegister8(ADXL366_ACT_INACT_CTL, &regVal)) {
        return false;
    }
    regVal &= ~0x0c;
    regVal |= 0x04;
    if (useReferenced) {
        regVal |= 0x08;
    }
    writeRegister(ADXL366_ACT_INACT_CTL, regVal);
    return true;
}

bool ADXL366_WE::setAxisMask(adxl366_axisMask axisMask) {
    if (!readRegister8(ADXL366_AXIS_MASK, &regVal)) {
        return false;
    }
    regVal &= ~0x07;
    regVal |= axisMask;
    writeRegister(ADXL366_AXIS_MASK, regVal);
    return true;
}

bool ADXL366_WE::setTapAxis(adxl366_tapAxis tapAxis) {
    if (!readRegister8(ADXL366_AXIS_MASK, &regVal)) {
        return false;
    }
    regVal &= ~0x30;
    regVal |= tapAxis;
    writeRegister(ADXL366_AXIS_MASK, regVal);
    return true;
}

// threshold is in g
// duration and latent are in ms
bool ADXL366_WE::setGeneralTapParameters(float threshold, float duration, float latent){
    // TAP_THRESH scale factor is 31.25mg/LSB
    regVal = static_cast<uint8_t>(round(threshold / 0.03125));
    if(regVal<1){
        regVal = 1;
    }
    writeRegister(ADXL366_TAP_THRESH, regVal);
    
    // TAP_DUR scale factor is 625us/LSB - same as ADXL345
    regVal = static_cast<uint8_t>(round(duration / 0.625));
    if(regVal<1){
        regVal = 1;
    }
    writeRegister(ADXL366_TAP_DUR, regVal);
    
    // TAP_LATENT scale factor is 1.25ms/LSB
    regVal = static_cast<uint8_t>(round(latent / 1.25));
    if(regVal<1){
        regVal = 1;
    }
    writeRegister(ADXL366_TAP_LATENT, regVal);     
    return true; 
}

bool ADXL366_WE::setDoubleTapWindow(float window){
    // TAP_WINDOW scale factor is 1.25ms/LSB
    regVal = static_cast<uint8_t>(round(window / 1.25));
    writeRegister(ADXL366_TAP_WINDOW, regVal);
    return true;
}

/************ FIFO ************/

bool ADXL366_WE::setFifoParameters(adxl366_fifoMode mode, adxl366_fifoAxes axes, adxl366_fifoExtra extra, uint16_t samples) {
    if (samples > 511) {
        return false;
    }
    if (!readRegister8(ADXL366_FIFO_CONTROL, &regVal)) {
        return false;
    }
    // The low byte of samples goes here
    writeRegister(ADXL366_FIFO_SAMPLES, samples & 0xff);
    // The rest goes into FIFO_CONTROL
    // Bit 7 = reserved - keep it, zero the rest
    regVal &= 0x80;
    // Bits 6:3 = channels, which comprises:
    // Bits 6:5 = extra (whether to do temp/ADC conversions as well)
    regVal |= (extra << 6);
    // Bits 4:3 = axes
    regVal |= (axes << 4);
    // Bit 2 = top bit of samples
    regVal |= (samples & 0x100) << 2;
    // Bits 1:0 = mode
    regVal |= mode;
    Serial.printf("setFifoParameters: Setting register FIFO_CONTROL to value 0x%02x\n", regVal);
    writeRegister(ADXL366_FIFO_CONTROL, regVal);
    return true;
}

bool ADXL366_WE::setFifoMode(adxl366_fifoMode mode)
{
    if (!readRegister8(ADXL366_FIFO_CONTROL, &regVal)) {
        return false;
    }
    // Bits 1:0 = mode
    regVal &= ~0x03;
    regVal |= mode;
    Serial.printf("setFifoMode: Setting register FIFO_CONTROL to value 0x%02x\n", regVal);
    writeRegister(ADXL366_FIFO_CONTROL, regVal);
    return true;
}

// uint8_t ADXL366_WE::getFifoStatus(){
//     if (!readRegister8(ADXL366_FIFO_STATUS, &regVal)) {
//         return 0; // Not ideal
//     }
//     return regVal;
// }

bool ADXL366_WE::resetTrigger() 
{
    return setFifoMode(ADXL366_BYPASS) && setFifoMode(ADXL366_TRIGGER);
}

void ADXL366_WE::softReset()
{
    writeRegister(ADXL366_SOFT_RESET, ADXL366_SOFT_RESET_VAL);
    delay(20);
}

bool ADXL366_WE::dumpAllRegisters()
{
    uint8_t buf[ADXL366_PEDOMETER_SENS_L];
    if (!readMultipleRegisters(ADXL366_DEVID_AD, ADXL366_PEDOMETER_SENS_L, buf))
    {
        Serial.printf("getAllRegisters failed");
        return false;
    }
    for (int i = ADXL366_DEVID_AD; i < ADXL366_PEDOMETER_SENS_L; i++)
    {
        Serial.printf("%02x: %02x\n", i, buf[i]);
    }
    return true;
}


/************************************************ 
    private functions
*************************************************/

void ADXL366_WE::writeRegister(adxl366_register reg, uint8_t val){
    if(!useSPI){
        _wire->beginTransmission(i2cAddress);
        _wire->write(reg);
        _wire->write(val);
        _wire->endTransmission();    
    }
    else{
        _spi->beginTransaction(mySPISettings);
        digitalWrite(csPin, LOW);
        delayMicroseconds(5);
        _spi->transfer(reg); 
        _spi->transfer(val);
        digitalWrite(csPin, HIGH);
        _spi->endTransaction();
    }
}
  
bool ADXL366_WE::readRegister8(adxl366_register reg, uint8_t *val){
    if(!useSPI){    
        bool ok = true;
        _wire->beginTransmission(i2cAddress);
        ok &= (_wire->write(reg) == 1);
        ok &= (_wire->endTransmission(false) == 0);
        _wire->requestFrom(i2cAddress, static_cast<uint8_t>(1));
        if(ok && _wire->available()){
            // Success
            *val = _wire->read();
            return true;
        } 
        // No response
        return false;
    }
    else{
        _spi->beginTransaction(mySPISettings);
        digitalWrite(csPin, LOW);
        delayMicroseconds(5);
        _spi->transfer(reg | 0x80); 
        *val = _spi->transfer(0x00);
        digitalWrite(csPin, HIGH);
        _spi->endTransaction();
        return true; // Error checking is not possible with SPI
    }
}

bool ADXL366_WE::readMultipleRegisters(adxl366_register reg, uint8_t count, uint8_t *buf){
    if(!useSPI){
        bool ok = true;
        _wire->beginTransmission(i2cAddress);
        ok &= (_wire->write(reg) == 1);
        ok &= (_wire->endTransmission(false) == 0);
        _wire->requestFrom(i2cAddress, count);
        if (ok && _wire->available() == count) {
            // Success
            for(int i=0; i<count; i++){
                buf[i] = _wire->read();
            }
            return true;
        }
        // No response, or incomplete response
        return false;
    }
    else{
        _spi->beginTransaction(mySPISettings);
        digitalWrite(csPin, LOW);
        delayMicroseconds(5);
        _spi->transfer(reg | 0xc0); 
        for(int i=0; i<count; i++){
            buf[i] = _spi->transfer(0x00);
        }
        digitalWrite(csPin, HIGH);
        _spi->endTransaction();
        return true; // Error checking is not possible with SPI
    }
}


