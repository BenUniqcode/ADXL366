# ADXL366_WE
Arduino Library for the ADXL366 accelerometer. 

This is based on ADXL366_WE by Wolfgang Ewald <wolfgang.ewald@wolles-elektronikkiste.de>, and was adapted for the ADXL366 by Ben Wheeler <ben@uniqcode.com>. 

The API remains almost fully compatible with ADXL366_WE - if you have existing code using that library, it should be sufficient to globally replace "345" with "366" 

I have tried to create a library for the ADXL366 which is easy to use for people who don't want to deal with all the registers. Therefore I have added lots example sketches which will enable you to deal even with the more complex features such as the FIFO modes. Howerever I still recommend to have a look into the data sheet to get a deeper understanding. 

You can use both I2C and SPI. If you want to find out how to use SPI, look at the ADXL366_SPI_basic_data.ino. 

<h2>ADXL367 vs. ADXL366</h2>
The ADXL367 has the same registers and settings like the ADXL366. The data sheets of both devices differ only marginally, e.g. the sensitivity of the ADXL366 has defined min./max. values, whereas the ADXL367 has only typical values. All example sketches run on both devices without any changes. However, with version 3.0.2 I have introduced a new ADXL367_WE class and defines like "ADXL367_RANGE_8G" because users might feel better using the correct name.


The ADXL367 modules that I have seen so far have two QWIIC connectors, which are convenient to use if your MCU board also has a QWIIC connector (e.g. Arduino UNO R4).


<h2>Important Note on Release 3.0.0</h2>

In versions < 3.0.0, there were several functions that returned a structure of type "xyzFloat". To be exact not the structure was returned but a pointer to this structure. Since the structures were created in the functions, the memory space where they were stored could have been overwritten. I have now changed this, but unfortunately, former sketches are not compatible anymore. However the change you need to apply is small. Example: 

Version < 3.0.0:
````
xyzFloat g = getGValues();
````

Versions >= 3.0.0:
````
 xyzFloat g;
 myAcc.getGValues(&g);
````

<h2>General Information</h2>

A detailed tutorial is available: 

https://wolles-elektronikkiste.de/ADXL366-teil-1  (German)

https://wolles-elektronikkiste.de/en/ADXL366-the-universal-accelerometer-part-1  (English) 

If you are not so much experienced with the ADXL366, I recommend to work through the examples in the following order:

1) ADXL366_basic_data
2) ADXL366_SPI_basic_data
3) ADXL366_calibration
4) ADXL366_angles_orientation
5) ADXL366_pitch_roll_corrected_angles
6) ADXL366_sleep
7) ADXL366_free_fall_interrupt
8) ADXL366_data_ready_interrupt
9) ADXL366_activity_inactivity_interrupt
10) ADXL366_auto_sleep.ino
11) ADXL366_single_tap
12) ADXL366_double_tap
13) ADXL366_fifo_fifo
14) ADXL366_fifo_stream
15) ADXL366_fifo_trigger
16) ADXL366_SPI_two_devices_one_interface
17) ADXL366_SPI_two_devices_two_interfaces

To develop this library I have worked with a ADXL366 module. It should also work with the bare ADXL366 IC. For the module I have noticed that the power consumption is much higher than mentioned in the data sheet. I think the issue is the voltage converter on the module. You can reduce the power consumption by choosing 3.3 volts instead of 5 volts. At least this worked with my module. 

If you like my library please give it a star. If you don't like it I would be happy to get feedback. And if you find bugs I will try to eliminate them as quickly as possible. 

<h2>If SPI does not work</h2>

My library has implemented SPI 4-Wire. Some modules have SDO connected GND via an 0 ohm resistor. With this resistor only SPI 3-Wire would work. Your options are:

1) Remove the 0 ohm resistor connected to SDO (R4 on my module)
2) Replace the 0 ohm resistor connected to SDO by an internal or external pull-down resistor of 4.7 kohm or 10 kohm
3) Find another library which supports SPI 3-Wire

![ADXL366_hack](https://github.com/wollewald/ADXL366_WE/assets/41305162/2fc39482-70f7-4de1-ac0b-2e27f28ac15e)

Another SPI issue might occur when using ESP8266 boards like the WEMOS D1 mini or NodeMCU. The standard CS Pin (e.g. D8/GPIO15 on a WEMOS D1 mini or NodeMCU) might not work since the CS Pin on most ADXL366 modules has a pull-up resistor. And if D8 is high at reset, the ESP8266 will not boot. In that case choose a different ESP8266 pin as CS!  

