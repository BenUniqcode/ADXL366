/***************************************************************************
* Example sketch for the ADXL366_WE / ADXL367_WE library
*
* This sketch shows how to use sleep mode of the ADXL366 and its effect
*   
* Further information can be found on: 
* https://wolles-elektronikkiste.de/adxl345-teil-1 (German)
* https://wolles-elektronikkiste.de/en/adxl345-the-universal-accelerometer-part-1 (English)
* 
***************************************************************************/

#include<Wire.h>
#include<ADXL366_WE.h>
#define ADXL366_I2CADDR 0x53  // 0x1D if SDO = HIGH

/* There are several ways to create your ADXL366 object:
 * ADXL366_WE myAcc = ADXL366_WE()                -> uses Wire / I2C Address = 0x53
 * ADXL366_WE myAcc = ADXL366_WE(ADXL366_I2CADDR) -> uses Wire / ADXL366_I2CADDR
 * ADXL366_WE myAcc = ADXL366_WE(&wire2)          -> uses the TwoWire object wire2 / ADXL366_I2CADDR
 * ADXL366_WE myAcc = ADXL366_WE(&wire2, ADXL366_I2CADDR) -> all together
 */
ADXL366_WE myAcc = ADXL366_WE(ADXL366_I2CADDR);

void setup(){
  Wire.begin();
  Serial.begin(115200);
  Serial.println("ADXL366_Sketch - Sleep");
  Serial.println();
  if(!myAcc.init()){
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
  myAcc.setDataRate(ADXL366_DATA_RATE_50);
  Serial.print("Data rate: ");
  Serial.print(myAcc.getDataRateAsString());

/* Choose the measurement range
    ADXL366_RANGE_16G    16g     
    ADXL366_RANGE_8G      8g     
    ADXL366_RANGE_4G      4g   
    ADXL366_RANGE_2G      2g
*/
  myAcc.setRange(ADXL366_RANGE_2G);
  Serial.print("  /  g-Range: ");
  Serial.println(myAcc.getRangeAsString());
  Serial.println();
}

void loop(){
/* Switch on (true) or switch off (false) sleep mode. Choose the wake up 
    frequency:
    ADXL366_WUP_FQ_1  =  1 Hz
    ADXL366_WUP_FQ_2  =  2 Hz
    ADXL366_WUP_FQ_4  =  4 Hz 
    ADXL366_WUP_FQ_8  =  8 Hz

    In this specific example you will see (provided you move the ADXL366) that
    in sleep mode the values are updated every third to fourth request. In awake
    mode the values are updated steadily. Sleep Mode saves significant power.  
*/
  myAcc.setSleep(true, ADXL366_WUP_FQ_1);
  Serial.println("Measure in Sleep Mode:");
  doMeasurements();
  
  myAcc.setSleep(false);
  Serial.println("Measure in Normal Mode:");
  doMeasurements();
}
  

void doMeasurements(){
  for(int i=0; i<10; i++){
    xyzFloat g;
    myAcc.getGValues(&g);
    
    Serial.print("g-x   = ");
    Serial.print(g.x);
    Serial.print("  |  g-y   = ");
    Serial.print(g.y);
    Serial.print("  |  g-z   = ");
    Serial.println(g.z);
    
    delay(300);
  }
}
