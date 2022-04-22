#include <SPI.h>
//#include <CAN.h>
#define CAN_2515
// #define CAN_2518FD

// Set SPI CS Pin according to your hardware

#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
// For Wio Terminal w/ MCP2518FD RPi Hat：
// Channel 0 SPI_CS Pin: BCM 8
// Channel 1 SPI_CS Pin: BCM 7
// Interupt Pin: BCM25
const int SPI_CS_PIN  = BCM8;
const int CAN_INT_PIN = BCM25;
#else

// For Arduino MCP2515 Hat:
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
#endif


#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif     


byte my_data[8];
char kind_of_data;
int RPM;
int Speed;
int Temp;


void setup()
{
  CAN.begin(CAN_500KBPS);
}

void loop()
{
Clear_Errors();
int i;
delay(10);
 
// -- Holds all serial data into a array -- //
 char bufferArray[20];
// -- Holds gear value data -- //
 byte gear;
// -- Marker that new data are available -- //
 byte geardata = 0;

// -- If 6 bytes available in the Serial buffer -- //
if (Serial.available() >= 9) {
 for (i = 0; i < 9; i++) { // for each byte...
 bufferArray[i] = Serial.read(); // put into array
 }
}

if (bufferArray[7] == 'G' ) {
 gear = bufferArray[8]; // retrieves the single byte of gear (0-255 value)
 geardata = 1; // we got new data!
}


// -- Parsers used by Simtools to send game datas for Gear, RPM, Speed, Temperature and the external LEDs to the cluster over serial port (USB) -- //
 while (Serial.available() < 1)
 {
 // -- Send Gear data to the cluster -- //
 //kind_of_data = Serial.read(); 
 //if (kind_of_data == 'G' ) Read_Gear(); 
 // -- Send Temp data to the cluster -- //
 kind_of_data = Serial.read();
 if (kind_of_data == 'T' ) Read_Temp(); 
 // -- Send RPM data to the cluster and flash RPM red LED when RPM hits 6500 -- //
 kind_of_data = Serial.read();
 if (kind_of_data == 'R' ) Read_RPM(); 
 // -- Send Speed data to the cluster -- //
 kind_of_data = Serial.read();
 if (kind_of_data == 'S' ) Read_Speed(); 
 }
}

void Clear_Errors()
{
  /* Clear Warning Lights */

   my_data[0] = 0xFE; //Unknown
   my_data[1] = 0xFE; //Unknown
   my_data[2] = 0xFE; //Unknown
   my_data[3] = 0x34; //DSC OFF in combo with byte 5 Live data only seen 0x34
   my_data[4] = 0x00; // B01000000; // Brake warning B00001000;  //ABS warning
   my_data[5] = 0x40; // TCS in combo with byte 3
   my_data[6] = 0x00; // Unknown
   my_data[7] = 0x00; // Unused
   CAN.sendMsgBuf(0x212, 0, 8, my_data);

 //Other gauges
 
   my_data[0] = 0x98 ; //temp gauge //~170 is red, ~165 last bar, 152 centre, 90 first bar, 92 second bar
   my_data[1] = 0x00; // something to do with trip meter 0x10, 0x11, 0x17 increments by 0.1 miles
   my_data[2] = 0x00; // unknown
   my_data[3] = 0x00; //unknown
   my_data[4] = 0x01; //Oil Pressure (not really a gauge)
   my_data[5] = 0x00; //check engine light
   my_data[6] = 0x00; //Coolant, oil and battery
   my_data[7] = 0x00; //unused
   CAN.sendMsgBuf(0x420, 0, 8, my_data);

    /* Turn off steering Warning */
   CAN.sendMsgBuf(0x300, 0, 1, 1); 
//}
}

// -- Temp Data and Maths -- //
void Read_Temp()
{
 // -- Read from serial -- //
 delay(1);
 int Temp100 = Serial.read() - '0';
 delay(1);
 int Temp10 = Serial.read() - '0';
 delay(1);
 int Temp1 = Serial.read() - '0';
 Temp = 100 * Temp100 + 10 * Temp10 + Temp1;
 my_data[0] = 0x98; // Temp Gauge Data
 my_data[1] = 0x00;
 my_data[2] = 0x00; // unknown
 my_data[3] = 0x00; //unknown
 my_data[4] = 0x01; //Oil Pressure (not really a gauge)
 my_data[5] = 0x00; //check engine light
 my_data[6] = 0x00; //Coolant, oil and battery
 my_data[7] = 0x00; //unused
 CAN.sendMsgBuf(0x420, 0, 8, my_data); // send Message
}

// -- RPM Data and Maths -- //
void Read_RPM()
{
 // -- Read from serial -- //
 delay(1);
 int RPM100 = Serial.read() - '0';
 delay(1);
 int RPM10 = Serial.read() - '0';
 delay(1);
 int RPM1 = Serial.read() - '0';
 int RPM = 100 * RPM100 + 10 * RPM10 + RPM1;
 // -- Set values -- //
 my_data[0] = (RPM * 4) / 256; // rpm
 my_data[1] = (RPM * 4) % 256; // rpm
 CAN.sendMsgBuf(0x201, 0, 8, my_data); // send Message
 delay(60);
}


// -- Speed Data and Maths -- //
void Read_Speed()
{
 // -- Read from Serial -- //
 delay(1);
 int Speed100 = Serial.read() - '0';
 delay(1);
 int Speed10 = Serial.read() - '0';
 delay(1);
 int Speed1 = Serial.read() - '0';
 int Speed = 100 * Speed100 + 10 * Speed10 + Speed1;
 // -- Set values -- //
 my_data[4] = (Speed + 1000) / 256; // speed
 my_data[5] = (Speed + 1000) % 256; // speed
 my_data[6] = 0x00;
 my_data[7] = 0x00;
 CAN.sendMsgBuf(0x201, 0, 8, my_data); // send Message
 delay(40);
 // -- Show digital speed on TM1638 display -- //
 //CarSpeedKM = Speed / 100 + 10; // calculation accounting for Simtools values used with the Mazda dashboard
 //module1.setDisplayToDecNumber(CarSpeedKM, 8, false);
}
