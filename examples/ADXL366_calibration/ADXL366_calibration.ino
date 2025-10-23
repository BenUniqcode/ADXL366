/***************************************************************************
* Example sketch for the ADXL366_WE / ADXL367_WE library
*
* This sketch shows how to calibrate the ADXL366 
* It does not use the internal offset registers.
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
  Serial.println(F("ADXL366_Sketch - Calibration"));
  Serial.println();
  if(!myAcc.init()){
    Serial.println(F("ADXL366 not connected!"));
  }
  Serial.println(F("Calibration procedure:"));
  Serial.println(F(" - stay in full resolution"));
  Serial.println(F(" - supply voltage has influence (at least for the modules)"));
  Serial.println(F("        -> choose the same voltage you will use in your project!")); 
  Serial.println(F(" - turn your ADXL slowly (!) to find the min and max raw x,y and z values"));
  Serial.println(F(" - deviations of one or two units don't matter much"));
  Serial.println(F(" - the calibration changes the slope of g vs raw and assumes zero is (min+max)/2 "));
  Serial.println(F(" - write down the six values "));
  Serial.println(F(" - you can try the calibration values in ADXL366_angles_tilt_orientation.ino example sketch"));
  Serial.println(F(" - ready to go? Then type in any key and send. "));
  while(!Serial.available());
  Serial.read();
  Serial.println(); Serial.println(); Serial.println();  
}

void loop() {
  xyzFloat raw;
  myAcc.getRawValues(&raw);
  Serial.print(F("Raw-x = "));
  Serial.print(raw.x);
  Serial.print(F("  |  Raw-y = "));
  Serial.print(raw.y);
  Serial.print(F("  |  Raw-z = "));
  Serial.println(raw.z);
  
  delay(1000);
  
}
