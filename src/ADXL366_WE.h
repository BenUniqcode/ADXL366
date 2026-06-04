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
constexpr uint8_t SOFT_RESET_VAL   {0x52}; // Value to write to the SOFT_RESET register to trigger a reset
#define ADXL367_ACT_LOW ADXL366_ACT_LOW
#define ADXL367_ACT_HIGH ADXL366_ACT_HIGH

constexpr float ADXL366_FLOAT_ERROR {999.0}; // Returned by getPitch() and getRoll() to indicate error
constexpr float ADXL367_FLOAT_ERROR {ADXL366_FLOAT_ERROR};

// Registers
typedef enum ADXL366_REGISTER : uint8_t {
        ADXL366_DEVID_AD         = 0x00,
        ADXL366_DEVID_MST        = 0x01,
        ADXL366_PART_ID          = 0x02,
        ADXL366_REV_ID           = 0x03,
        // 0x04 RESERVED
        ADXL366_SERIAL_NUMBER_2  = 0x05,
        ADXL366_SERIAL_NUMBER_1  = 0x06,
        ADXL366_SERIAL_NUMBER_0  = 0x07,
        ADXL366_XDATA            = 0x08, /* for 8-bit reads */
        ADXL366_YDATA            = 0x09, /* for 8-bit reads */
        ADXL366_ZDATA            = 0x0A, /* for 8-bit reads */
        ADXL366_STATUS           = 0x0B,
        ADXL366_FIFO_ENTRIES_L   = 0x0C,
        ADXL366_FIFO_ENTRIES_H   = 0x0D,
        ADXL366_XDATA_H          = 0x0E,
        ADXL366_XDATA_L          = 0x0F,
        ADXL366_YDATA_H          = 0x10,
        ADXL366_YDATA_L          = 0x11,
        ADXL366_ZDATA_H          = 0x12,
        ADXL366_ZDATA_L          = 0x13,
        ADXL366_TEMP_H           = 0x14,
        ADXL366_TEMP_L           = 0x15,
        ADXL366_EX_ADC_H         = 0x16,
        ADXL366_EX_ADC_L         = 0x17,
        ADXL366_I2C_FIFO_DATA    = 0x18, /* For reading bytes from the FIFO with a single multibyte I2C read command */
        // 0x19~0x1E unused
        ADXL366_SOFT_RESET       = 0x1F,
        ADXL366_THRESH_ACT_H     = 0x20,
        ADXL366_THRESH_ACT_L     = 0x21,
        ADXL366_TIME_ACT         = 0x22,
        ADXL366_THRESH_INACT_H   = 0x23,
        ADXL366_THRESH_INACT_L   = 0x24,
        ADXL366_TIME_INACT_H     = 0x25,
        ADXL366_TIME_INACT_L     = 0x26,
        ADXL366_ACT_INACT_CTL    = 0x27,
        ADXL366_FIFO_CONTROL     = 0x28,
        ADXL366_FIFO_SAMPLES     = 0x29,
        ADXL366_INTMAP1_LOWER    = 0x2A,
        ADXL366_INTMAP2_LOWER    = 0x2B,
        ADXL366_FILTER_CTL       = 0x2C,
        ADXL366_POWER_CTL        = 0x2D,
        ADXL366_SELF_TEST        = 0x2E,
        ADXL366_TAP_THRESH       = 0x2F,
        ADXL366_TAP_DUR          = 0x30,
        ADXL366_TAP_LATENT       = 0x31,
        ADXL366_TAP_WINDOW       = 0x32,
        ADXL366_X_OFFSET         = 0x33,
        ADXL366_Y_OFFSET         = 0x34,
        ADXL366_Z_OFFSET         = 0x35,
        ADXL366_X_SENS           = 0x36,
        ADXL366_Y_SENS           = 0x37,
        ADXL366_Z_SENS           = 0x38,
        ADXL366_TIMER_CTL        = 0x39,
        ADXL366_INTMAP1_UPPER    = 0x3A,
        ADXL366_INTMAP2_UPPER    = 0x3B,
        ADXL366_ADC_CTL          = 0x3C,
        ADXL366_TEMP_CTL         = 0x3D,
        ADXL366_TEMP_ADC_OVER_THRSH_H  = 0x3E,
        ADXL366_TEMP_ADC_OVER_THRSH_L  = 0x3F,
        ADXL366_TEMP_ADC_UNDER_THRSH_H = 0x40,
        ADXL366_TEMP_ADC_UNDER_THRSH_L = 0x41,
        ADXL366_TEMP_ADC_TIMER   = 0x42,
        ADXL366_AXIS_MASK        = 0x43,
        ADXL366_STATUS_COPY      = 0x44,
        ADXL366_STATUS_2         = 0x45,
        ADXL366_STATUS_3         = 0x46,
        ADXL366_PEDOMETER_STEP_CNT_H   = 0x47,
        ADXL366_PEDOMETER_STEP_CNT_L   = 0x48,
        ADXL366_PEDOMETER_CTL    = 0x49,
        ADXL366_PEDOMETER_THRES_H = 0x4A,
        ADXL366_PEDOMETER_THRES_L = 0x4B,
        ADXL366_PEDOMETER_SENS_H = 0x4C,
        ADXL366_PEDOMETER_SENS_L = 0x4D,
} adxl366_register;

// Bits in the POWER_CTL register
enum ADXL366_POWER_CTL {
    ADXL366_MEASURE_0, 
    ADXL366_MEASURE_1, 
    ADXL366_AUTOSLEEP, 
    ADXL366_WAKEUP, 
    ADXL366_NOISE_0, 
    ADXL366_NOISE_1, 
    ADXL366_EXT_CLK,
    ADXL367_MEASURE_0 = ADXL366_MEASURE_0, 
    ADXL367_MEASURE_1 = ADXL366_MEASURE_1, 
    ADXL367_AUTOSLEEP = ADXL366_AUTOSLEEP, 
    ADXL367_WAKEUP = ADXL366_WAKEUP, 
    ADXL367_NOISE_0 = ADXL366_NOISE_0, 
    ADXL367_NOISE_1 = ADXL366_NOISE_1, 
    ADXL367_EXT_CLK = ADXL366_EXT_CLK
};

typedef enum ADXL366_DATA_RATE {
    ADXL366_DATA_RATE_ERROR = -1, // returned on read error, cannot be set
	ADXL366_DATA_RATE_12_5  = 0,
	ADXL366_DATA_RATE_25,
	ADXL366_DATA_RATE_50,
	ADXL366_DATA_RATE_100,
	ADXL366_DATA_RATE_200,
	ADXL366_DATA_RATE_400,
    ADXL367_DATA_RATE_ERROR = ADXL366_DATA_RATE_ERROR,
	ADXL367_DATA_RATE_12_5  = ADXL366_DATA_RATE_12_5,
	ADXL367_DATA_RATE_25    = ADXL366_DATA_RATE_25,
	ADXL367_DATA_RATE_50    = ADXL366_DATA_RATE_50,
	ADXL367_DATA_RATE_100   = ADXL366_DATA_RATE_100,
	ADXL367_DATA_RATE_200   = ADXL366_DATA_RATE_200,
	ADXL367_DATA_RATE_400   = ADXL366_DATA_RATE_400,
} adxl366_dataRate;

typedef enum ADXL366_RANGE {
    ADXL366_RANGE_ERROR        = -1, // returned on read error, cannot be set
    ADXL366_RANGE_8G           = 0b10,
    ADXL366_RANGE_4G           = 0b01,
    ADXL366_RANGE_2G           = 0b00,
    ADXL367_RANGE_ERROR        = ADXL366_RANGE_ERROR,
    ADXL367_RANGE_8G           = ADXL366_RANGE_8G,
    ADXL367_RANGE_4G           = ADXL366_RANGE_4G,
    ADXL367_RANGE_2G           = ADXL366_RANGE_2G
} adxl366_range;

typedef enum ADXL366_ORIENTATION {
  ADXL366_ORIENTATION_ERROR = -1, // returned on read error
  ADXL367_ORIENTATION_ERROR = ADXL366_ORIENTATION_ERROR,
  FLAT = 0, FLAT_1, XY, XY_1, YX, YX_1
} adxl366_orientation;

/** 
 * Interrupt enables Low reg (one for each pin)
 * DATA_READY,
 * FIFO_READY,
 * FIFO_WATERMARK,
 * FIFO_OVERRUN,
 * ACT
 * INACT
 * AWAKE
 * ACTIVE_LOW
 * 
 * STATUS reg
 * DATA_READY
 * FIFO_READY
 * FIFO_WATERMARK
 * FIFO_OVERRUN
 * ACT
 * INACT
 * AWAKE
 * ERR_USER_REGS (SEU error, or no register write has been done since powerup)
 * 
 * STATUS2 reg
 * TAP_ONE
 * TAP_TWO
 * TEMP_ADC_LOW
 * TEMP_ADC_HIGH
 * TIMER
 * RESERVED
 * FUSE_REFRESH (Software reset recommended)
 * ERR_FUSE_REGS (Unrecoverable?)
 * 
 * STATUS3 reg (366 only)
 * PEDOMETER_OVERFLOW
 * RESERVED...

 */


// Bits in INTMAP[12]_LOWER and _UPPER and STATUS/2/3 (and STATUS_COPY) registers
typedef enum ADXL366_INT {
    ADXL366_INT_DATA_READY,
    ADXL366_INT_FIFO_READY,
    ADXL366_INT_FIFO_WATERMARK,
    ADXL366_INT_FIFO_OVERRUN,
    ADXL366_INT_ACT,
    ADXL366_INT_INACT,
    ADXL366_INT_AWAKE,
    ADXL366_INT_ACTIVE_LOW,
    ADXL366_INT_TAP_ONE,
    ADXL366_INT_TAP_TWO,
    ADXL366_INT_TEMP_ADC_LOW,
    ADXL366_INT_TEMP_ADC_HI,
    ADXL366_INT_KPALV_TIMER,
    ADXL366_INT_RESERVED,
    ADXL366_INT_ERR_USER_REGS,
    ADXL366_INT_ERR_FUSE,

    ADXL367_INT_DATA_READY = ADXL366_INT_DATA_READY,
    ADXL367_INT_FIFO_READY = ADXL366_INT_FIFO_READY,
    ADXL367_INT_FIFO_WATERMARK = ADXL366_INT_FIFO_WATERMARK,
    ADXL367_INT_FIFO_OVERRUN = ADXL366_INT_FIFO_OVERRUN,
    ADXL367_INT_ACT = ADXL366_INT_ACT,
    ADXL367_INT_INACT = ADXL366_INT_INACT,
    ADXL367_INT_AWAKE = ADXL366_INT_AWAKE,
    ADXL367_INT_ACTIVE_LOW = ADXL366_INT_ACTIVE_LOW,
    ADXL367_INT_TAP_ONE = ADXL366_INT_TAP_ONE,
    ADXL367_INT_TAP_TWO = ADXL366_INT_TAP_TWO,
    ADXL367_INT_TEMP_ADC_LOW = ADXL366_INT_TEMP_ADC_LOW,
    ADXL367_INT_TEMP_ADC_HI = ADXL366_INT_TEMP_ADC_HI,
    ADXL367_INT_KPALV_TIMER = ADXL366_INT_KPALV_TIMER,
    ADXL367_INT_RESERVED = ADXL366_INT_RESERVED,
    ADXL367_INT_ERR_USER_REGS = ADXL366_INT_ERR_USER_REGS,
    ADXL367_INT_ERR_FUSE = ADXL366_INT_ERR_FUSE,
} adxl366_int;

// Bits 0..2 of Axis Mask - set bits mean "ignore this axis"; our names mean the opposite:
// if X Y or Z is present it means the axis is included, if it's 0 then it's ignored.
typedef enum ADXL366_AXIS_MASK_XYZ {
    ADXL366_XYZ,
    ADXL366_XY0,
    ADXL366_X0Z,  
    ADXL366_X00,
    ADXL366_0YZ,
    ADXL366_0Y0, 
    ADXL366_00Z, 
    ADXL366_000,
    ADXL367_XYZ = ADXL366_XYZ,
    ADXL367_XY0 = ADXL366_XY0,
    ADXL367_X0Z = ADXL366_X0Z,
    ADXL367_X00 = ADXL366_X00,
    ADXL367_0YZ = ADXL366_0YZ,
    ADXL367_0Y0 = ADXL366_0Y0,
    ADXL367_00Z = ADXL366_00Z,
    ADXL367_000 = ADXL366_000,
} adxl366_axisMask;

typedef enum ADXL366_TAP_AXIS {
    ADXL366_TAP_AXIS_X,
    ADXL366_TAP_AXIS_Y,
    ADXL366_TAP_AXIS_Z,
    ADXL367_TAP_AXIS_X = ADXL366_TAP_AXIS_X,
    ADXL367_TAP_AXIS_Y = ADXL366_TAP_AXIS_Y,
    ADXL367_TAP_AXIS_Z = ADXL366_TAP_AXIS_Z,
} adxl366_tapAxis;

typedef enum ADXL366_WAKE_UP_FREQ{
    ADXL366_WUP_FQ_UNSET = -1,
    ADXL366_WUP_FQ_12 = 0, 
    ADXL366_WUP_FQ_6, 
    ADXL366_WUP_FQ_3, 
    ADXL366_WUP_FQ_1_5,
    ADXL367_WUP_FQ_UNSET = ADXL366_WUP_FQ_UNSET,
    ADXL367_WUP_FQ_12 = ADXL366_WUP_FQ_12,
    ADXL367_WUP_FQ_6 = ADXL366_WUP_FQ_6,
    ADXL367_WUP_FQ_3 = ADXL366_WUP_FQ_3,
    ADXL367_WUP_FQ_1_5 = ADXL366_WUP_FQ_1_5
} adxl366_wUpFreq;

typedef enum ADXL366_FIFO_MODE {
    ADXL366_BYPASS, 
    ADXL366_FIFO, // aka "oldest saved mode" in the datasheet
    ADXL366_STREAM, 
    ADXL366_TRIGGER,
    ADXL367_BYPASS  = ADXL366_BYPASS,
    ADXL367_FIFO    = ADXL366_FIFO,
    ADXL367_STREAM  = ADXL366_STREAM,
    ADXL367_TRIGGER = ADXL366_TRIGGER
} adxl366_fifoMode;

typedef enum ADXL366_FIFO_AXES {
    ADXL366_FIFO_XYZ,
    ADXL366_FIFO_X00,
    ADXL366_FIFO_0Y0,
    ADXL366_FIFO_00Z
} adxl366_fifoAxes;

typedef enum ADXL366_FIFO_EXTRA {
    ADXL366_FIFO_NO_EXTRA,
    ADXL366_FIFO_WITH_TEMPERATURE,
    ADXL366_FIFO_WITH_ADC
} adxl366_fifoExtra;

typedef enum ADXL366_TRIGGER_INT {
    ADXL366_TRIGGER_INT_1, ADXL366_TRIGGER_INT_2,
    ADXL367_TRIGGER_INT_1 = ADXL366_TRIGGER_INT_1,
    ADXL367_TRIGGER_INT_2 = ADXL366_TRIGGER_INT_2
} adxl366_triggerInt;

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
                
        /* Other */
        
        static constexpr float MILLI_G_PER_LSB {0.25}; // (In the 2G range; multiply by rangeFactor for other ranges)
        
        /* Basic settings */
        
        bool init(bool startMeasuring = true);
        void setSPIClockSpeed(unsigned long clock);
        void setCorrFactors(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
        bool setDataRate(adxl366_dataRate rate);
        adxl366_dataRate getDataRate();
        String getDataRateAsString();
        bool setRange(adxl366_range range);
        adxl366_range getRange();
        String getRangeAsString();
        uint8_t getDeviceID();
        
        /* x,y,z results */
            
        bool getRawValues(xyzFloat *rawVal);
        bool getRawValues8(xyzFloat *rawVal); // 8-bit faster read
        bool getCorrectedRawValues(xyzFloat *rawVal);
        bool getGValues(xyzFloat *gVal);
            
        /* Angles and Orientation */ 
        
        bool getAngles(xyzFloat *angleVal);
        bool getCorrAngles(xyzFloat *corrAngleVal);
        bool measureAngleOffsets();
        xyzFloat getAngleOffsets();
        void setAngleOffsets(const xyzFloat aos);
        adxl366_orientation getOrientation();
        String getOrientationAsString();
        float getPitch();
        float getRoll();
        
        /* Power, Sleep, Standby */ 
        
        bool setMeasureMode(bool measure);
        bool setSleep(bool sleep, adxl366_wUpFreq freq = ADXL366_WUP_FQ_UNSET);
        bool setAutoSleep(bool autoSleep, adxl366_wUpFreq freq = ADXL366_WUP_FQ_UNSET);
        bool isAsleep();
        bool setLowPower(bool lowpwr);
        bool isLowPower();
        
        /* Interrupts */
        
        void disableAllInterrupts();
        bool setInterrupt(adxl366_int type, uint8_t pin, bool setOn = true);
        bool setInterruptPolarity(uint8_t pol, uint8_t pin = 0); // 0 = both, for backwards compatibility
        bool deleteInterrupt(adxl366_int type, uint8_t pin = 0);
        uint32_t readAndClearInterrupts(); // Changed return type because the 366 has more interrupts
        bool checkInterrupt(uint32_t source, adxl366_int type); // Changed type of first param to match above
        bool setLinkBit(bool link);
        void setFreeFallThresholds(float ffg, float fft);

        // These have completely changed on the 366, there is no longer any reporting of
        // which axes triggered - instead you just get one bit set in the STATUS regs for each
        // uint8_t getActTapStatus();
        // String getActTapStatusAsString();

        
        // These have had to change from ADXL345_WE because the axis mask is now shared
        // between them instead of being separate for each. What ADXL345_WE called "AC"/"DC" 
        // mode - but was always called "referenced"/"absolute", has also changed to
        // a simple bool, because how that's enabled is also different.
        bool setActivityParameters(bool useReferenced, float threshold);
        bool setInactivityParameters(bool useReferenced, float threshold, uint16_t inactTime); // inactTime type changed
        bool setGeneralTapParameters(float threshold, float duration, float latent);
        bool setAxisMask(adxl366_axisMask axisMask);
        bool setTapAxis(adxl366_tapAxis tapAxis);
        // This has also changed from setAdditionalDoubleTapParameters because there's only one parameter now,
        // there doesn't seem to be any equivalent of the 345's "suppress" bit.
        bool setDoubleTapWindow(float window);

        /* FIFO */
        
        // FIFO parameters have changed a lot. It usually makes sense to set all the params at once
        // rather than requring setFifoMode() to be separate, however it's still useful to be able
        // to change the FIFO mode separately so I've kept the separate function too.
        bool setFifoParameters(adxl366_fifoMode mode, adxl366_fifoAxes axes, adxl366_fifoExtra extra, uint16_t samples);
        bool setFifoMode(adxl366_fifoMode mode);
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
        bool adxl366_lowRes;
        void writeRegister(adxl366_register reg, uint8_t val);
        bool readRegister8(adxl366_register reg, uint8_t *val);
        bool readMultipleRegisters(adxl366_register reg, uint8_t count, uint8_t *buf);
};

#endif


