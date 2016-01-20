/* BMP085 Extended Example Code
  by: Jim Lindblom
  SparkFun Electronics
  date: 1/18/11
  updated: 2/26/13
  license: CC BY-SA v3.0 - http://creativecommons.org/licenses/by-sa/3.0/
  
  Get pressure and temperature from the BMP085 and calculate 
  altitude. Serial.print it out at 115200 baud to serial monitor.

*/

#include <Arduino.h>

#define BMP085_ADDRESS 0x77  // I2C address of BMP085

class BMP085 {
	
public:
  BMP085();

  //Public Functions
  void begin();
  int16_t Calibration();
  int16_t GetTemperature();
  int32_t GetPressure();

  //Public Variables

  // Use these for altitude conversions
  const float p0 = 101325;     // Pressure at sea level (Pa)

private:
  //Private Functions
  int16_t ReadInt(uint8_t address);
  int8_t Read(uint8_t address);
  uint16_t ReadUT();
  uint32_t ReadUP();
  
  
  //Private Variables

  // Calibration values
int16_t ac1;
int16_t ac2; 
int16_t ac3; 
uint16_t ac4;
uint16_t ac5;
uint16_t ac6;
int16_t b1; 
int16_t b2;
int16_t mb;
int16_t mc;
int16_t md;

// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
int32_t b5; 	
	
};
