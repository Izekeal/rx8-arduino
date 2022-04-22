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
int RPM;
int kmhspeed;


void setup()
{
  //message_id=0x000;
  CAN.begin(CAN_500KBPS);
  Serial.begin(9600);
  RPM = 5500;
  kmhspeed = 10000;

}

void loop()
{

set_Speed();


delay(10);


/* Speedo / tacho */

   my_data[0] = (RPM * 4) / 256; // rpm
   my_data[1] = (RPM * 4) % 256; // rpm
   my_data[2] = 0xFF; // Unknown, 0xFF from 'live'.
   my_data[3] = 0xFF; // Unknown, 0xFF from 'live'.
   my_data[4] = (kmhspeed + 10000) / 256; //speed
   my_data[5] = (kmhspeed + 10000) % 256; //speed
   my_data[6] = 0x00; // Unknown possible accelerator pedel if Madox is correc
   my_data[7] = 0x00; //Unknown
   Serial.println("In loop");
   
   Serial.println(my_data[4]);
   Serial.println(my_data[5]);
   CAN.sendMsgBuf(0x201, 0, 8, my_data); 
   delay(40);

clear_Warnings();


}

void set_Speed(){

  Serial.println("Enter new KPH: ");

  while (Serial.available() == 0) {
    }
  
  kmhspeed = Serial.parseInt();
  Serial.println(kmhspeed);
  
}

void clear_Warnings(){
  
  /* Turn off steering Warning */
   CAN.sendMsgBuf(0x300, 0, 1, 1); 
   
   my_data[0] = 0xFE; //Unknown
   my_data[1] = 0xFE; //Unknown
   my_data[2] = 0xFE; //Unknown
   my_data[3] = 0x34; //DSC OFF in combo with byte 5 Live data only seen 0x34
   my_data[4] = 0x00; // B01000000; // Brake warning B00001000;  //ABS warning
   my_data[5] = 0x40; // TCS in combo with byte 3
   my_data[6] = 0x00; // Unknown
   my_data[7] = 0x00; // Unused
   CAN.sendMsgBuf(0x212, 0, 8, my_data);

   my_data[0] = 0x98 ; //temp gauge //~170 is red, ~165 last bar, 152 centre, 90 first bar, 92 second bar
   my_data[1] = 0x00; // something to do with trip meter 0x10, 0x11, 0x17 increments by 0.1 miles
   my_data[2] = 0x00; // unknown
   my_data[3] = 0x00; //unknown
   my_data[4] = 0x01; //Oil Pressure (not really a gauge)
   my_data[5] = 0x00; //check engine light
   my_data[6] = 0x00; //Coolant, oil and battery
   my_data[7] = 0x00; //unused
   CAN.sendMsgBuf(0x420, 0, 8, my_data);
   
}
