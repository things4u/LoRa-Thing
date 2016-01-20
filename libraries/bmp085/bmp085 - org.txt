/* BMP085 Code, ported to C++ callback fo Arduino 
  by M. Westenberg (mw12554@hotmail.com)
  
  Original by: Jim Lindblom, SparkFun Electronics (date: 1/18/11 updated: 2/26/13)
  license: CC BY-SA v3.0 - http://creativecommons.org/licenses/by-sa/3.0/
  
  Get pressure and temperature from the BMP085 and calculate 
  altitude. Serial.print it out at 115200 baud to serial monitor.
  (Version of Adafruit is very similar)
*/

#include <Wire.h>
#include <bmp085.h>

#define BMP085_ADDRESS 0x77	// I2C address of BMP085

const unsigned char OSS = 0;  // Oversampling Setting

BMP085::BMP085()
{
  //Set initial values for private vars
}

void BMP085::begin(void)
{
  Wire.begin();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int BMP085::ReadInt(unsigned char address)
{
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 2);
  int counter = 0;
  while(Wire.available() < 2)
  {
		counter++;
		delay(1);
		if(counter > 100) return 998; //Error out
  }
	
  msb = Wire.read();
  lsb = Wire.read();
  
  return (int) msb<<8 | lsb;
}


// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
int BMP085::Calibration()
{
  ac1 = ReadInt(0xAA);
  if (ac1 == 998) return(998);	// Error, Timeout, no sensor present
  ac2 = ReadInt(0xAC);
  ac3 = ReadInt(0xAE);
  ac4 = ReadInt(0xB0);
  ac5 = ReadInt(0xB2);
  ac6 = ReadInt(0xB4);
  b1 = ReadInt(0xB6);
  b2 = ReadInt(0xB8);
  mb = ReadInt(0xBA);
  mc = ReadInt(0xBC);
  md = ReadInt(0xBE);
  return(0);
}

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short BMP085::GetTemperature()
{
  long x1, x2;
  unsigned int ut = ReadUT();
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long BMP085::GetPressure()
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  unsigned long up = ReadUP();				// This was a function parameter and now internal Class function

  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}

// Read 1 byte from the BMP085 at 'address'
char BMP085::Read(unsigned char address)
{
  unsigned char data;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 1);
  int counter = 0;
  while(!Wire.available())
  {
		counter++;
		delay(1);
		if(counter > 100) return 998; //Error out
  }
    
  return Wire.read();
}



// Read the uncompensated temperature value
unsigned int BMP085::ReadUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
  delay(5);
  
  // Read two bytes from registers 0xF6 and 0xF7
  ut = ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long BMP085::ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);
  
  // Wait for data to become available
  int counter = 0;
  while(Wire.available() < 3)
  {
		counter++;
		delay(1);
		if(counter > 100) return 998; //Error out
  }
  msb = Wire.read();
  lsb = Wire.read();
  xlsb = Wire.read();
  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  
  return up;
}

