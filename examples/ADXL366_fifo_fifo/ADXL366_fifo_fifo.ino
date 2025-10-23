/***************************************************************************
* Example sketch for the ADXL366_WE / ADXL367_WE library
*
* This sketch shows how to use the FIFO in fifo mode.
*   
* Further information can be found on: 
* https://wolles-elektronikkiste.de/adxl345-teil-1 (German)
* https://wolles-elektronikkiste.de/en/adxl345-the-universal-accelerometer-part-1 (English)
* 
***************************************************************************/

#include<Wire.h>
#include<ADXL366_WE.h>
#define ADXL366_I2CADDR 0x53 // 0x1D if SDO = HIGH
const int int2Pin = 2;
volatile bool event = false;

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
  Serial.println("ADXL366_Sketch - FIFO - Fifo Mode");
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
  myAcc.setDataRate(ADXL366_DATA_RATE_3_13);
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

/* Stop the measure mode - no new data will be obtained */
  myAcc.setMeasureMode(false);
  attachInterrupt(digitalPinToInterrupt(int2Pin), eventISR, RISING);

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
  myAcc.setInterrupt(ADXL366_WATERMARK, INT_PIN_2); // Interrupt when FIFO is full
  
/* The following two FIFO parameters need to be set:
    1. Trigger Bit: not relevant for this FIFO mode.
    2. FIFO samples (max 32). Defines the size of the FIFO. One sample is an x,y,z triple.
*/
  myAcc.setFifoParameters(ADXL366_TRIGGER_INT_1, 32);

/* You can choose the following FIFO modes:
    ADXL366_FIFO     -  you choose the start, ends when FIFO is full (at defined limit)
    ADXL366_STREAM   -  FIFO always filled with new data, old data replaced if FIFO is full; you choose the stop
    ADXL366_TRIGGER  -  FIFO always filled up to 32 samples; when the trigger event occurs only defined number of samples
                        is kept in the FIFO and further samples are taken after the event until FIFO is full again. 
    ADXL366_BYPASS   -  no FIFO
*/   
  myAcc.setFifoMode(ADXL366_FIFO); 
  myAcc.readAndClearInterrupts();
}

void loop() {
  event = false; 
  
  countDown(); // as an alternative you could define any other event to start to fill the FIFO
  myAcc.readAndClearInterrupts();
  myAcc.setMeasureMode(true); // this is the actual start
  while(!event){}  // event = true means WATERMARK interrupt triggered -> means FIFO is full
  myAcc.setMeasureMode(false);
  Serial.println("FiFo full");
  
  for(int i=0; i<34; i++){ // this is > 32 samples, but I want to show that the values do not change when FIFO is full
    xyzFloat g;
    myAcc.getGValues(&g);
    
    Serial.print("g-x   = ");
    Serial.print(g.x);
    Serial.print("  |  g-y   = ");
    Serial.print(g.y);
    Serial.print("  |  g-z   = ");
    Serial.println(g.z);
  }
  Serial.println("For another series of measurements, enter any key and send");
  
  while(!(Serial.available())){}
  Serial.read();
  Serial.println();
}

void eventISR() {
  event = true;
}

void countDown(){
  Serial.println("Move your ADXL366 to obtain interesting data");
  Serial.println();
  delay(1000);
  Serial.print("Fifo collection begins in 3, "); 
  delay(1000);
  Serial.print("2, "); 
  delay(1000);
  Serial.print("1, "); 
  delay(1000);
  Serial.println("Now!");
}
