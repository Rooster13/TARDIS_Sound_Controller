/* TARDIS Dress Sound Generator 
    This code is designed to work with the Adafruit WAVE shield
    and the SparkFun MMA8452 Breakout board accelerometer. It is
    designed to sense movement (aka traveling) and to play the
    TARDIS materialization sound on a loop while moving.
    Otherwise, it will play the interrior hum if no movement is
    sensed.
    
    Pin set-up for accelerometer:
     Hardware setup:
      MMA8452 Breakout -------- Arduino
      3.3V --------------------- 3.3V
      SDA -------^^(330)^^------- A4
      SCL -------^^(330)^^------- A5
      GND ---------------------- GND
 */

// including libraries for WAVE shield
#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"

// including library required for I2C reading from the accelerometer
#include <Wire.h>

// including library for watchdog timer
#include "TimerOne.h"

// Defining accelerometer address
#define MMA8452_ADDRESS 0x1D

// Defining registers to be used on the MMA8452
#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG 0x0E
#define WHO_AM_I 0x0D
#define CTRL_REG1 0x2A

#define GSCALE 2 // Sets full-scale range to +/- 2, 4, or 8g

SdReader card;
FatVolume vol;
FatReader root;
FatReader f;

WaveHC wave;

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

void setup()
{
  // set up serial port for debugging
  Serial.begin(9600);
  putstring_nl("Initializing TARDIS Console...");
  
  Wire.begin(); // Join bus as a master
  
  initMMA8452(); // Test and initialize the accelerometer
  
  // Set output pins for DAC control. Pins defined in library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  if (!card.init())
  {
    putstring_nl("Card init. failed!");
    sdErrorCheck(); // Print out error
    while(1);
  }
  
  // enable optimized part reading...disable if problems arise
  card.partialBlockRead(true);
  
  // Search for FAT partition
  uint8_t part;
  for (part = 0; part < 5; part++)
  {
    if (vol.init(card, part))
      break; // Found it! Let's move on!
  }
  if (part==5)
  {
    putstring_nl("No valid FAT partition found!");
    sdErrorCheck();  // Print out error
    while(1);
  }
  
  // Try and open root directory
  if (!root.openRoot(vol))
  {
    putstring_nl("Can't open root directory!");
    while(1);
  }
  
  // Begin Timer interrupt for Accelerometer checks
  Timer1.initialize();
  Timer1.attachInterrupt(CheckAccelData);
  
  // All systems check out, let's tell the user
  putstring_nl("All systems go, TARDIS is ready for flight!");
}

void CheckAccelData()
{
  int accelCount[3]; // Stores the 12-bit signed value read
  readAccelData(accelCount); // Read the x/y/z adc values

  // Calculate G values for better analyzing...
  float accelG[3]; // Stores the real accel value in g's
  for (int i = 0 ; i < 3 ; i++)
  {
    accelG[i] = (float) accelCount[i] / ((1<<12)/(2*GSCALE)); // get actual g value, this depends on scale being set
  }  

  for (int i = 0; i < 3; i++)
  {
    Serial.print (accelG[i], 4); // Print accelerometer readings
    Serial.print("\t"); // tab between axes for visibility
  }
  Serial.println();
  
  if((accelG[1] > 0.5) | (accelG[1] < -0.5))
    playcomplete("TARDISMV.WAV");
}

void loop()
{
  playcomplete("TARDISHM.WAV");
}

void playcomplete(char *name)
{
  playfile(name);
  while (wave.isplaying);
}

void playfile(char *name)
{
  // See if wave object is currently busy
  if (wave.isplaying)
  {
    wave.stop();
  }
  // look in root directory and open file
  if (!f.open(root, name))
  {
    putstring("Couldn't open file ");
    Serial.print(name);
    return;
  }
  // read file and turn it into a wave object
  if (!wave.create(f))
  {
    putstring_nl("Not a valid WAV file!");
    return;
  }
  // all systems check out, begin playback
  wave.play();
}

void readAccelData(int *destination)
{
  byte rawData[6]; // x/y/z accel register data stored here
  
  readRegisters(OUT_X_MSB, 6, rawData); // Read the six raw data registers into array
  
  // Loop to calculate 12-bit ADC and g values for each axis
  for(int i = 0; i < 3; i++)
  {
    int gCount = (rawData[i*2] << 8) | rawData[(i*2)+1]; // Combine two 8-bit numbers into one 12-bit number
    gCount >>= 4; // manually right-align new number
    
    // If number is 2's compliment negative, we must reverse it manually
    if (rawData[i*2] > 0x7F) // MSB is a 1, and thus number is negative
    {
      gCount = ~gCount + 1; // Take 2's compliment
      gCount *= -1;
    }
    
    destination[i] = gCount; // Record this gCount into the 3 int array
  }
}

// Initialize the registers on the accelerometer
void initMMA8452()
{
  byte c = readRegister(WHO_AM_I); // Read the WHO_AM_I register
  if (c == 0x2A) // This register should always be 0x2A
  {
    Serial.println("TARDIS is online...");
  }
  else
  {
    Serial.print("Could not connect to TARDIS mainframe: 0x");
    Serial.println(c, HEX); // Could not open communications with accelerometer
    while(1);
  }
  
  MMA8452Standby(); // Must be in standby mode to change registers
  
  // Set up the full scale range to 2, 4, or 8g.
  byte fsr = GSCALE;
  if(fsr > 8) fsr = 8; // Error check to make sure scale is never greater than 8\
  fsr >>= 2;
  writeRegister(XYZ_DATA_CFG, fsr);
  
  MMA8452Active(); // Set to active mode to begin reading
}

void MMA8452Standby()
{
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c & ~(0x01)); // Clears active bit to force into standby
}

void MMA8452Active()
{
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c | 0x01);
}

void readRegisters(byte addressToRead, int bytesToRead, byte * dest)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); // ends transmission but keeps connection active
  
  Wire.requestFrom(MMA8452_ADDRESS, bytesToRead);
  
  while(Wire.available() < bytesToRead);
  
  for(int x = 0; x < bytesToRead; x++)
    dest[x] = Wire.read();
}

byte readRegister(byte addressToRead)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false);
  
  Wire.requestFrom(MMA8452_ADDRESS, 1);
  
  while(!Wire.available());
  return Wire.read();
}

void writeRegister(byte addressToWrite, byte dataToWrite)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToWrite);
  Wire.write(dataToWrite);
  Wire.endTransmission();
}
