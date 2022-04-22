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

//const int message_id_201 = 0x201;
//const int message_id_212 = 0x212;
//const int message_id_231 = 0x231;
//const int message_id_420 = 0x420;
//int message_id ;


byte message_to_send;

//int id;
//byte b;
//int i;
//char s[8];
byte my_data[8];
byte BitMap[8];
int RPM;
int kmhspeed;


void setup()
{
  //message_id=0x000;
  CAN.begin(CAN_500KBPS);
  //CAN.setMode (CAN_MODE_NORMAL);
  RPM = 5500;
  kmhspeed = 10000;

}

void loop()
{


delay(10);

/* Turn off steering Warning */

//if(steering == 0 ) {
   //CAN.setMessageID (message_id_300);
   CAN.sendMsgBuf(0x300, 0, 1, 1); 
//}


/* Speedo / tacho */

   //CAN.setMessageID (message_id_201);
   my_data[0] = (RPM * 4) / 256; // rpm
   my_data[1] = (RPM * 4) % 256; // rpm
   my_data[2] = 0xFF; // Unknown, 0xFF from 'live'.
   my_data[3] = 0xFF; // Unknown, 0xFF from 'live'.
   my_data[4] = (kmhspeed + 10000) / 256; //speed
   my_data[5] = (kmhspeed + 10000) % 256; //speed
   my_data[6] = 0x00; // Unknown possible accelerator pedel if Madox is correc
   my_data[7] = 0x00; //Unknown
   CAN.sendMsgBuf(0x201, 0, 8, my_data); 
   delay(40);

/* Warning Lights */

   //CAN.setMessageID (message_id_212);
   my_data[0] = 0xFE; //Unknown
   my_data[1] = 0xFE; //Unknown
   my_data[2] = 0xFE; //Unknown
   my_data[3] = 0x34; //DSC OFF in combo with byte 5 Live data only seen 0x34
   my_data[4] = 0x00; // B01000000; // Brake warning B00001000;  //ABS warning
   my_data[5] = 0x40; // TCS in combo with byte 3
   my_data[6] = 0x00; // Unknown
   my_data[7] = 0x00; // Unused
   CAN.sendMsgBuf(0x212, 0, 8, my_data);

/* Other gauges / warning lights */

   //CAN.setMessageID (message_id_420);
   my_data[0] = 0x98 ; //temp gauge //~170 is red, ~165 last bar, 152 centre, 90 first bar, 92 second bar
   my_data[1] = 0x00; // something to do with trip meter 0x10, 0x11, 0x17 increments by 0.1 miles
   my_data[2] = 0x00; // unknown
   my_data[3] = 0x00; //unknown
   my_data[4] = 0x01; //Oil Pressure (not really a gauge)
   my_data[5] = 0x00; //check engine light
   my_data[6] = 0x00; //Coolant, oil and battery
   my_data[7] = 0x00; //unused
   CAN.sendMsgBuf(0x420, 0, 8, my_data); 


/* Cruise Control Light */

/*
  CAN.setMessageID (message_id_650);
  my_data[0] = 0xFF; // cruise control light 0x80 is yellow, 0xFF is green
  CAN.sendData(my_data,1); 
*/


}
