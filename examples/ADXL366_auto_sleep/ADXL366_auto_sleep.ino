/***************************************************************************
* Example sketch for the ADXL366_WE / ADXL367_WE library
*
* This sketch shows how to use the auto sleep function.
*   
* Further information can be found on: 
* https://wolles-elektronikkiste.de/adxl345-teil-1
* https://wolles-elektronikkiste.de/en/adxl345-the-universal-accelerometer-part-1 (English)
* 
***************************************************************************/
#include<Wire.h>
#include<ADXL366_WE.h>
#define ADXL366_I2CADDR 0x53 // 0x1D if SDO = HIGH
const int int2Pin = 2;
volatile bool in_activity = false;

/* There are several ways to create your ADXL366 object:
 * ADXL366_WE myAcc = ADXL366_WE()                -> uses Wire / I2C Address = 0x53
 * ADXL366_WE myAcc = ADXL366_WE(ADXL366_I2CADDR) -> uses Wire / ADXL366_I2CADDR
 * ADXL366_WE myAcc = ADXL366_WE(&wire2)          -> uses the TwoWire object wire2 / ADXL366_I2CADDR
 * ADXL366_WE myAcc = ADXL366_WE(&wire2, ADXL366_I2CADDR) -> all together
 */
ADXL366_WE myAcc = ADXL366_WE(ADXL366_I2CADDR);

void setup() {
  Wire.begin();
  Serial.begin(115200);
  pinMode(int2Pin, INPUT);
  Serial.println("ADXL366_Sketch - Auto Sleep");
  Serial.println();
  if (!myAcc.init()) {
    Serial.println("ADXL366 not connected!");
  }
/* Insert your data from ADXL366_calibration.ino and uncomment for more precise results */
  // myAcc.setCorrFactors(-266.0, 285.0, -268.0, 278.0, -291.0, 214.0);
  
/* Choose the data rate         Hz
    ADXL366_DATA_RATE_3200    3200
    ADXL366_DATA_RATE_1600    1600
    ADXL366_DATA_RATE_800      800
    ADXL366_DATA_RATE_400      400
    ADXL366_DATA_RATE_200      200
    ADXL366_DATA_RATE_100      100
    ADXL366_DATA_RATE_50        50
    ADXL366_DATA_RATE_25        25
    ADXL366_DATA_RATE_12_5      12.5  
    ADXL366_DATA_RATE_6_25       6.25
    ADXL366_DATA_RATE_3_13       3.13
    ADXL366_DATA_RATE_1_56       1.56
    ADXL366_DATA_RATE_0_78       0.78
    ADXL366_DATA_RATE_0_39       0.39
    ADXL366_DATA_RATE_0_20       0.20
    ADXL366_DATA_RATE_0_10       0.10
*/
  myAcc.setDataRate(ADXL366_DATA_RATE_25);
  Serial.print("Data rate: ");
  Serial.print(myAcc.getDataRateAsString());

/* Choose the measurement range
    ADXL366_RANGE_16G    16g     
    ADXL366_RANGE_8G      8g     
    ADXL366_RANGE_4G      4g   
    ADXL366_RANGE_2G      2g
*/ 
  myAcc.setRange(ADXL366_RANGE_4G);
  Serial.print("  /  g-Range: ");
  Serial.println(myAcc.getRangeAsString());
  Serial.println();
 
  attachInterrupt(digitalPinToInterrupt(int2Pin), in_activityISR, RISING);

/* The following settings are similar to the settings in ADXL366_activity_inactivity_interrupt.ino */

/* Three parameters have to be set for activity:
    1. DC / AC Mode:
        ADXL366_DC_MODE - Threshold is the defined one (parameter 3)
        ADXL366_AC_MODE - Threshold = starting acceleration + defined threshold 
    2. Axes, that are considered:
        ADXL366_000  -  no axis (which makes no sense)
        ADXL366_00Z  -  z 
        ADXL366_0Y0  -  y
        ADXL366_0YZ  -  y,z
        ADXL366_X00  -  x
        ADXL366_X0Z  -  x,z
        ADXL366_XY0  -  x,y
        ADXL366_XYZ  -  all axes
    3. Threshold in g
*/  
  myAcc.setActivityParameters(ADXL366_DC_MODE, ADXL366_XY0, 0.8);

/* Four parameters have to be set for in activity:
    1. DC / AC Mode:
        see activity parameters
    2. Axes, that are considered:
        see activity parameters
    3. Threshold in g
    4. Inactivity period threshold in seconds (max 255)
*/    
  myAcc.setInactivityParameters(ADXL366_DC_MODE, ADXL366_XY0, 0.8, 10);
  
/* You can choose the following interrupts:
     Variable name:             Triggered, if:
    ADXL366_OVERRUN      -   new data replaces unread data
    ADXL366_WATERMARK    -   the number of samples in FIFO equals the number defined in FIFO_CTL
    ADXL366_FREEFALL     -   acceleration values of all axes are below the threshold defined in THRESH_FF 
    ADXL366_INACTIVITY   -   acc. value of all included axes are < THRESH_INACT for period > TIME_INACT
    ADXL366_ACTIVITY     -   acc. value of included axes are > THRESH_ACT
    ADXL366_DOUBLE_TAP   -   double tap detected on one incl. axis and various defined conditions are met
    ADXL366_SINGLE_TAP   -   single tap detected on one incl. axis and various defined conditions are met
    ADXL366_DATA_READY   -   new data available

    Assign the interrupts to INT1 (INT_PIN_1) or INT2 (INT_PIN_2). Data ready, watermark and overrun are 
    always enabled. You can only change the assignment of these which is INT1 by default.

    You can delete interrupts with deleteInterrupt(type);
*/ 
  myAcc.setInterrupt(ADXL366_ACTIVITY, INT_PIN_2);
  myAcc.setInterrupt(ADXL366_INACTIVITY, INT_PIN_2);
 
/* Auto sleep is connected with activity and inactivity. The device goes in sleep when inactivity is 
    detected. The link bit must be set, if you want to use auto sleep. The library sets the link bit 
    automatically. When the ADXL366 goes into sleep mode it wakes up periodically (default is 8 Hz).
    
    Choose the wake up frequency:
    ADXL366_WUP_FQ_1  =  1 Hz
    ADXL366_WUP_FQ_2  =  2 Hz
    ADXL366_WUP_FQ_4  =  4 Hz 
    ADXL366_WUP_FQ_8  =  8 Hz
    
*/
  myAcc.setAutoSleep(true, ADXL366_WUP_FQ_1);
  // alternative: myAcc.setAutoSleep(true/false) without changing the wake up frequency.
  myAcc.readAndClearInterrupts();

}

void loop() {
  if ((millis() % 300) == 1) {
    xyzFloat g;
    myAcc.getGValues(&g);
    Serial.print("g-x   = ");
    Serial.print(g.x);
    Serial.print("  |  g-y   = ");
    Serial.print(g.y);
    Serial.print("  |  g-z   = ");
    Serial.println(g.z);
    delay(4);
  }

  if(in_activity == true) {
    byte intSource = myAcc.readAndClearInterrupts();
    if(myAcc.checkInterrupt(intSource, ADXL366_ACTIVITY)){
      Serial.println("Activity!");
      if(!myAcc.isAsleep()){        //check the asleep bit
        Serial.println("I am awake!");
      }
    }
     
    if(myAcc.checkInterrupt(intSource, ADXL366_INACTIVITY)){
      Serial.println("Inactivity!");
      if(myAcc.isAsleep()){
        Serial.println("I am sleeping...");
      }
    }
      
    myAcc.readAndClearInterrupts();
    in_activity = false;
  }
}

void in_activityISR() {
  in_activity = true;
}
