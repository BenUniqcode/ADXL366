/**
 * Arduino library for the Analog Devices ADXL366 / ADXL367 accelerometer
 * Adapted by Ben Wheeler <ben@uniqcode.com>
 * from ADXL366_WE by Wolfgang (Wolle) Ewald https://github.com/wollewald/ADXL366_WE
 * Released under the MIT License.
 */

#ifndef ADXL366_WE_H_
#define ADXL366_WE_H_

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>
#include <SPI.h>
#include "xyzFloat.h"

/* Definitions */

constexpr uint8_t INT_PIN_1        {0x01};   
constexpr uint8_t INT_PIN_2        {0x02};
constexpr uint8_t ADXL366_ACT_LOW  {0x01}; 
constexpr uint8_t ADXL366_ACT_HIGH {0x00};
#define ADXL367_ACT_LOW ADXL_345_ACT_LOW
#define ADXL367_ACT_HIGH ADXL366_ACT_HIGH

constexpr float ADXL366_FLOAT_ERROR {999.0}; // Returned by getPitch() and getRoll() to indicate error
constexpr float ADXL367_FLOAT_ERROR {ADXL366_FLOAT_ERROR};

typedef enum ADXL366_PWR_CTL {
    ADXL366_WAKE_UP_0, ADXL366_WAKE_UP_1, ADXL366_SLEEP, 
    ADXL366_MEASURE, ADXL366_AUTO_SLEEP, ADXL366_LINK,
    ADXL367_WAKE_UP_0  = ADXL366_WAKE_UP_0, 
    ADXL367_WAKE_UP_1  = ADXL366_WAKE_UP_1, 
    ADXL367_SLEEP      = ADXL366_SLEEP, 
    ADXL367_MEASURE    = ADXL366_MEASURE,  
    ADXL367_AUTO_SLEEP = ADXL366_AUTO_SLEEP, 
    ADXL367_LINK       = ADXL366_LINK
} adxl345_pwrCtl;

typedef enum ADXL366_DATA_RATE {
    ADXL366_DATA_RATE_ERROR   = -1, // returned on read error, cannot be set
    ADXL366_DATA_RATE_3200    = 0x0F,
    ADXL366_DATA_RATE_1600    = 0x0E,
    ADXL366_DATA_RATE_800     = 0x0D,
    ADXL366_DATA_RATE_400     = 0x0C,
    ADXL366_DATA_RATE_200     = 0x0B,
    ADXL366_DATA_RATE_100     = 0x0A,
    ADXL366_DATA_RATE_50      = 0x09,
    ADXL366_DATA_RATE_25      = 0x08,
    ADXL366_DATA_RATE_12_5    = 0x07,
    ADXL366_DATA_RATE_6_25    = 0x06,
    ADXL366_DATA_RATE_3_13    = 0x05,
    ADXL366_DATA_RATE_1_56    = 0x04,
    ADXL366_DATA_RATE_0_78    = 0x03,
    ADXL366_DATA_RATE_0_39    = 0x02,
    ADXL366_DATA_RATE_0_20    = 0x01,
    ADXL366_DATA_RATE_0_10    = 0x00,
    ADXL367_DATA_RATE_ERROR   = ADXL366_DATA_RATE_ERROR,
    ADXL367_DATA_RATE_3200    = ADXL366_DATA_RATE_3200,
    ADXL367_DATA_RATE_1600    = ADXL366_DATA_RATE_1600,
    ADXL367_DATA_RATE_800     = ADXL366_DATA_RATE_800,
    ADXL367_DATA_RATE_400     = ADXL366_DATA_RATE_400,
    ADXL367_DATA_RATE_200     = ADXL366_DATA_RATE_200,
    ADXL367_DATA_RATE_100     = ADXL366_DATA_RATE_100,
    ADXL367_DATA_RATE_50      = ADXL366_DATA_RATE_50,
    ADXL367_DATA_RATE_25      = ADXL366_DATA_RATE_25,
    ADXL367_DATA_RATE_12_5    = ADXL366_DATA_RATE_12_5,
    ADXL367_DATA_RATE_6_25    = ADXL366_DATA_RATE_6_25,
    ADXL367_DATA_RATE_3_13    = ADXL366_DATA_RATE_3_13,
    ADXL367_DATA_RATE_1_56    = ADXL366_DATA_RATE_1_56,
    ADXL367_DATA_RATE_0_78    = ADXL366_DATA_RATE_0_78,
    ADXL367_DATA_RATE_0_39    = ADXL366_DATA_RATE_0_39,
    ADXL367_DATA_RATE_0_20    = ADXL366_DATA_RATE_0_20,
    ADXL367_DATA_RATE_0_10    = ADXL366_DATA_RATE_0_10
} adxl345_dataRate;

typedef enum ADXL366_RANGE {
    ADXL366_RANGE_ERROR        = -1, // returned on read error, cannot be set
    ADXL366_RANGE_16G          = 0b11,
    ADXL366_RANGE_8G           = 0b10,
    ADXL366_RANGE_4G           = 0b01,
    ADXL366_RANGE_2G           = 0b00,
    ADXL367_RANGE_ERROR        = ADXL366_RANGE_ERROR,
    ADXL367_RANGE_16G          = ADXL366_RANGE_16G,
    ADXL367_RANGE_8G           = ADXL366_RANGE_8G,
    ADXL367_RANGE_4G           = ADXL366_RANGE_4G,
    ADXL367_RANGE_2G           = ADXL366_RANGE_2G
} adxl345_range;

typedef enum ADXL366_ORIENTATION {
  ADXL366_ORIENTATION_ERROR = -1, // returned on read error
  ADXL367_ORIENTATION_ERROR = ADXL366_ORIENTATION_ERROR,
  FLAT = 0, FLAT_1, XY, XY_1, YX, YX_1
} adxl345_orientation;

typedef enum ADXL366_INT {
    ADXL366_OVERRUN, ADXL366_WATERMARK, ADXL366_FREEFALL, ADXL366_INACTIVITY, 
    ADXL366_ACTIVITY, ADXL366_DOUBLE_TAP, ADXL366_SINGLE_TAP, ADXL366_DATA_READY,
    ADXL367_OVERRUN    = ADXL366_OVERRUN,
    ADXL367_WATERMARK  = ADXL366_WATERMARK,
    ADXL367_FREEFALL   = ADXL366_FREEFALL,
    ADXL367_INACTIVITY = ADXL366_INACTIVITY, 
    ADXL367_ACTIVITY   = ADXL366_ACTIVITY,
    ADXL367_DOUBLE_TAP = ADXL366_DOUBLE_TAP,
    ADXL367_SINGLE_TAP = ADXL366_SINGLE_TAP,
    ADXL367_DATA_READY = ADXL366_DATA_READY
} adxl345_int;

typedef enum ADXL366_ACT_TAP_SET {
    ADXL366_000, ADXL366_00Z, ADXL366_0Y0, ADXL366_0YZ,
    ADXL366_X00, ADXL366_X0Z, ADXL366_XY0, ADXL366_XYZ,
    ADXL367_000 = ADXL366_000,
    ADXL367_00Z = ADXL366_00Z,
    ADXL367_0Y0 = ADXL366_0Y0,
    ADXL367_0YZ = ADXL366_0YZ,
    ADXL367_X00 = ADXL366_X00,
    ADXL367_X0Z = ADXL366_X0Z,
    ADXL367_XY0 = ADXL366_XY0,
    ADXL367_XYZ = ADXL366_XYZ
} adxl345_actTapSet;

typedef enum ADXL366_DC_AC {
    ADXL366_DC_MODE = 0,
    ADXL366_AC_MODE = 0x08,
    ADXL367_DC_MODE = ADXL366_DC_MODE,
    ADXL367_AC_MODE = ADXL366_AC_MODE
} adxl345_dcAcMode;

typedef enum ADXL366_WAKE_UP_FREQ{
    ADXL366_WUP_FQ_UNSET = -1,
    ADXL366_WUP_FQ_8 = 0, ADXL366_WUP_FQ_4, ADXL366_WUP_FQ_2, ADXL366_WUP_FQ_1,
    ADXL367_WUP_FQ_UNSET = ADXL366_WUP_FQ_UNSET,
    ADXL367_WUP_FQ_8 = ADXL366_WUP_FQ_8,
    ADXL367_WUP_FQ_4 = ADXL366_WUP_FQ_4,
    ADXL367_WUP_FQ_2 = ADXL366_WUP_FQ_2,
    ADXL367_WUP_FQ_1 = ADXL366_WUP_FQ_1
} adxl345_wUpFreq;

typedef enum ADXL366_ACT_TAP {
    ADXL366_TAP_Z, ADXL366_TAP_Y, ADXL366_TAP_X, ADXL366_ASLEEP, ADXL366_ACT_Z, ADXL366_ACT_Y, ADXL366_ACT_X,
    ADXL367_TAP_Z  = ADXL366_TAP_Z,
    ADXL367_TAP_Y  = ADXL366_TAP_Y,
    ADXL367_TAP_X  = ADXL366_TAP_X,
    ADXL367_ASLEEP = ADXL366_ASLEEP,
    ADXL367_ACT_Z  = ADXL366_ACT_Z,
    ADXL367_ACT_Y  = ADXL366_ACT_Y,
    ADXL367_ACT_X  = ADXL366_ACT_X
} adxl345_actTap;

typedef enum ADXL366_FIFO_MODE {
    ADXL366_BYPASS, ADXL366_FIFO, ADXL366_STREAM, ADXL366_TRIGGER,
    ADXL367_BYPASS  = ADXL366_BYPASS,
    ADXL367_FIFO    = ADXL366_FIFO,
    ADXL367_STREAM  = ADXL366_STREAM,
    ADXL367_TRIGGER = ADXL366_TRIGGER
} adxl345_fifoMode;

typedef enum ADXL366_TRIGGER_INT {
    ADXL366_TRIGGER_INT_1, ADXL366_TRIGGER_INT_2,
    ADXL367_TRIGGER_INT_1 = ADXL366_TRIGGER_INT_1,
    ADXL367_TRIGGER_INT_2 = ADXL366_TRIGGER_INT_2
} adxl345_triggerInt;

class ADXL366_WE
{
    public: 
        
        /* Constructors */
        
        ADXL366_WE(uint8_t addr = 0x53) : _wire{&Wire}, i2cAddress{addr}, useSPI{false} {}
        
        ADXL366_WE(TwoWire *w, uint8_t addr = 0x53) : _wire{w}, i2cAddress{addr}, useSPI{false} {}
        
        ADXL366_WE(int cs, bool spi, int mosi = 999, int miso = 999, int sck = 999, int sid = -1) 
            : _spi{&SPI}, csPin{cs}, useSPI{spi}, mosiPin{mosi}, misoPin{miso}, sckPin{sck}, sensorID{sid} {}
            
        ADXL366_WE(SPIClass *s, int cs, bool spi, int mosi = 999, int miso = 999, int sck = 999, int sid = -1)
            :  _spi{s}, csPin{cs}, useSPI{spi}, mosiPin{mosi}, misoPin{miso}, sckPin{sck}, sensorID{sid} {}
        
        /* registers */
        
        static constexpr uint8_t ADXL366_DEVID            {0x00};
        static constexpr uint8_t ADXL366_THRESH_TAP       {0x1D}; 
        static constexpr uint8_t ADXL366_OFSX             {0x1E};
        static constexpr uint8_t ADXL366_OFSY             {0x1F};
        static constexpr uint8_t ADXL366_OFSZ             {0x20};
        static constexpr uint8_t ADXL366_DUR              {0x21};
        static constexpr uint8_t ADXL366_LATENT           {0x22};
        static constexpr uint8_t ADXL366_WINDOW           {0x23};
        static constexpr uint8_t ADXL366_THRESH_ACT       {0x24};
        static constexpr uint8_t ADXL366_THRESH_INACT     {0x25}; 
        static constexpr uint8_t ADXL366_TIME_INACT       {0x26};
        static constexpr uint8_t ADXL366_ACT_INACT_CTL    {0x27};
        static constexpr uint8_t ADXL366_THRESH_FF        {0x28};
        static constexpr uint8_t ADXL366_TIME_FF          {0x29};
        static constexpr uint8_t ADXL366_TAP_AXES         {0x2A};
        static constexpr uint8_t ADXL366_ACT_TAP_STATUS   {0x2B};
        static constexpr uint8_t ADXL366_BW_RATE          {0x2C};
        static constexpr uint8_t ADXL366_POWER_CTL        {0x2D};
        static constexpr uint8_t ADXL366_INT_ENABLE       {0x2E};
        static constexpr uint8_t ADXL366_INT_MAP          {0x2F};
        static constexpr uint8_t ADXL366_INT_SOURCE       {0x30};
        static constexpr uint8_t ADXL366_DATA_FORMAT      {0x31};
        static constexpr uint8_t ADXL366_DATAX0           {0x32};
        static constexpr uint8_t ADXL366_DATAX1           {0x33};
        static constexpr uint8_t ADXL366_DATAY0           {0x34};
        static constexpr uint8_t ADXL366_DATAY1           {0x35};
        static constexpr uint8_t ADXL366_DATAZ0           {0x36};
        static constexpr uint8_t ADXL366_DATAZ1           {0x37};
        static constexpr uint8_t ADXL366_FIFO_CTL         {0x38};
        static constexpr uint8_t ADXL366_FIFO_STATUS      {0x39};

        /* Register bits */
        
        static constexpr uint8_t ADXL366_FULL_RES         {3};
        static constexpr uint8_t ADXL366_SUPPRESS         {3};
        static constexpr uint8_t ADXL366_LOW_POWER        {4};
        
        /* Device ID register value */
        static constexpr uint8_t ADXL366_DEVID_VALUE      {0xe5}; /* Same for ADXL367 */

        /* Other */
        
        static constexpr float MILLI_G_PER_LSB             {3.9};
        static constexpr float UNITS_PER_G              {256.41};// = 1/0.0039
    
        /* Basic settings */
        
        bool init(bool startMeasuring = true);
        void setSPIClockSpeed(unsigned long clock);
        void setCorrFactors(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
        bool setDataRate(adxl345_dataRate rate);
        adxl345_dataRate getDataRate();
        String getDataRateAsString();
        uint8_t getPowerCtlReg();
        bool setRange(adxl345_range range);
        adxl345_range getRange();
        bool setFullRes(bool full);
        String getRangeAsString();
        uint8_t getDeviceID();
        
        /* x,y,z results */
            
        bool getRawValues(xyzFloat *rawVal);
        bool getCorrectedRawValues(xyzFloat *rawVal);
        bool getGValues(xyzFloat *gVal);
            
        /* Angles and Orientation */ 
        
        bool getAngles(xyzFloat *angleVal);
        bool getCorrAngles(xyzFloat *corrAngleVal);
        bool measureAngleOffsets();
        xyzFloat getAngleOffsets();
        void setAngleOffsets(const xyzFloat aos);
        adxl345_orientation getOrientation();
        String getOrientationAsString();
        float getPitch();
        float getRoll();
        
        /* Power, Sleep, Standby */ 
        
        bool setMeasureMode(bool measure);
        bool setSleep(bool sleep, adxl345_wUpFreq freq = ADXL366_WUP_FQ_UNSET);
        bool setAutoSleep(bool autoSleep, adxl345_wUpFreq freq = ADXL366_WUP_FQ_UNSET);
        bool isAsleep();
        bool setLowPower(bool lowpwr);
        bool isLowPower();
        
        /* Interrupts */
        
        bool setInterrupt(adxl345_int type, uint8_t pin);
        bool setInterruptPolarity(uint8_t pol);
        bool deleteInterrupt(adxl345_int type);
        uint8_t readAndClearInterrupts();
        bool checkInterrupt(uint8_t source, adxl345_int type);
        bool setLinkBit(bool link);
        void setFreeFallThresholds(float ffg, float fft);
        bool setActivityParameters(adxl345_dcAcMode mode, adxl345_actTapSet axes, float threshold);
        bool setInactivityParameters(adxl345_dcAcMode mode, adxl345_actTapSet axes, float threshold, uint8_t inactTime);
        bool setGeneralTapParameters(adxl345_actTapSet axes, float threshold, float duration, float latent);
        bool setAdditionalDoubleTapParameters(bool suppress, float window);
        uint8_t getActTapStatus();
        String getActTapStatusAsString();
        
        /* FIFO */
        
        bool setFifoParameters(adxl345_triggerInt intNumber, uint8_t samples);
        bool setFifoMode(adxl345_fifoMode mode);
        uint8_t getFifoStatus();
        bool resetTrigger();
       
    protected:
        TwoWire *_wire;
        SPIClass *_spi;
        SPISettings mySPISettings = SPISettings();
        unsigned long spiClock = 5000000;
        uint8_t i2cAddress;
        uint8_t regVal;   // intermediate storage of register values
        xyzFloat offsetVal;
        xyzFloat angleOffsetVal;
        xyzFloat corrFact;
        int csPin;
        bool useSPI;    
        int mosiPin;
        int misoPin;
        int sckPin;  
        int sensorID;
        float rangeFactor;
        bool adxl345_lowRes;
        void writeRegister(uint8_t reg, uint8_t val);
        bool readRegister8(uint8_t reg, uint8_t *val);
        bool readMultipleRegisters(uint8_t reg, uint8_t count, uint8_t *buf);
};

#endif


