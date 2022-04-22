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
 Serial.begin(115200);
 // -- Send a diagnostic test to the Serial Monitor for the CAN shield -- //
 if (CAN.begin(CAN_500KBPS) == CAN_OK) Serial.print("can init ok!!\r\n");
 else Serial.print("Can init fail!!\r\n");
}

void loop()
{
 int i;
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
 if (geardata == 1) {
 char* neutral = "n"; // sets the character for neutral
 char* reverse = "r"; // sets the character for reverse
 if (gear >= 1 and gear < 10 ) {
 //module1.setDisplayDigit(gear, 0, false); // displays numerical value of the current gear
 }
 if (gear == 0) {
 //module1.setDisplayToString(neutral, 0, 0); // displays the character for neutral
 }
 if (gear == 255) { // -1 that represents reverse rollover to 255 so... 
 //module1.setDisplayToString(reverse, 0, 0); // displays the character for reverse 
 }
 geardata = 0;
 }
 // -- Parsers used by Simtools to send game datas for Gear, RPM, Speed, Temperature and the external LEDs to the cluster over serial port (USB) -- //
 while (Serial.available() < 1)
 {
 // -- Send Gear data to the cluster -- //
 kind_of_data = Serial.read(); // kind_of_data = Serial.read(); 
 if (kind_of_data == 'G' ) Read_Gear(); 
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
 //Temp = 100 * Temp100 + 10 * Temp10 + Temp1;
 //my_data[0] = 0x98; // Temp Gauge Data
 //my_data[1] = 0x00;
 //CAN.sendMsgBuf(0x420, 0, 8, my_data); // send Message
}
// -- Gear Data and Maths -- //
void Read_Gear()
{
 delay(1);
 int Gear100 = Serial.read() - '0';
 delay(1);
 int Gear10 = Serial.read() - '0';
 delay(1);
 int Gear1 = Serial.read() - '0';
 //Gear = 100 * Gear100 + 10 * Gear10 + Gear1;
 //Gear = map(Gear, 127, 255, 0, 7);
 //if ( Gear == 0 )
 //{
 // -- Show 'N' (Neutral) on dashboard -- //
 //my_data[2] = 0x3; // Gear: Neutral
 //my_data[3] = 0xFF;
 //CAN.sendMsgBuf(0x228, 0, 8, my_data); // send Message
 //}
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
 // -- Engine and battery lights will turn ON when engine has stopped -- //
 if (RPM < 100)
 {
 my_data[2] = 0x00; // check engine light + coolant, oil and battery ON
 
 CAN.sendMsgBuf(0x420, 0, 8, my_data); // send Message
 }
 // -- Engine and battery lights will turn off when engine has started -- //
 if (RPM > 100)
 {
 my_data[3] = 0xFF; // check engine light + coolant, oil and battery OFF
 }
 // -- Fuel gauge needle goes up when engine is off (fake value not based on game telemetry) -- //
 if (RPM < 300) // 0RPM
 {
 // -- Initialize the pin 2 to drive the fuel gauge needle up -- //
 pinMode(2, OUTPUT); // pin to which the fuel gauge is connected
 }
 // -- Fuel gauge needle goes down when engine is on (fake value not based on game telemetry) -- //
 if (RPM > 300) // 1000RPM
 {
 // -- Initialize and empty pin to drive the fuel gauge needle down -- //
 pinMode(4, OUTPUT); // empty pin
 }
 /*
 // -- RPM LEDs will turn on at increasing RPM and Fuel Gauge needle goes up when engine is off -- //
 if (RPM < 100) // 0RPM
 {
 module1.setLEDs(0b00000000 | 0b00000000); // All TM1638 LED off
 }
 if (RPM >= 101 && RPM <= 400) // 1000RPM
 {
 module1.setLEDs(0b00000001 | 0b00000000); // TM1638 Green LED 1 on
 }
 if (RPM >= 401 && RPM <= 700) // 2000RPM
 {
 module1.setLEDs(0b00000011 | 0b00000000 << 8 ); // TM1638 Green LED 1 and 2 on
 }
 if (RPM >= 701 && RPM <= 1000) // 3000RPM
 {
 module1.setLEDs(0b00000111 | 0b00000000 << 8 ); // TM1638 Green LED 1, 2 and 3 on
 }
 if (RPM >= 1001 && RPM <= 1200) // 4000RPM
 {
 module1.setLEDs(0b00001111 | 0b00000000 << 8 ); // TM1638 Green LED 1, 2, 3 and 4 on
 }
 if (RPM >= 1201 && RPM <= 1450) // 5000RPM
 {
 module1.setLEDs(0b00001111 | 0b00010000 << 8 ); // TM1638 Green LED 1, 2, 3, 4 and Red LED 1 on
 }
 if (RPM >= 1451 && RPM <= 1600) // 6000RPM
 {
 module1.setLEDs(0b00001111 | 0b00110000 << 8 ); // TM1638 Green LED 1, 2, 3, 4 and Red LED 1, 2 on
 }
 if (RPM >= 1601 && RPM <= 1940) // 7000RPM
 {
 module1.setLEDs(0b00001111 | 0b01110000 << 8 ); // TM1638 Green LED 1, 2, 3, 4 and Red LED 1, 2, 3 on
 }
 if (RPM >= 1941 && RPM <= 2200) // 8000RPM
 {
 module1.setLEDs(0b00001111 | 0b11110000 << 8 ); // TM1638 Green LED 1, 2, 3, 4 and Red LED 1, 2, 3 on
 }
 // -- RPM Red LED will turn on at 6500 RPM -- //
 if (RPM > 1600) analogWrite(ShiftLED, ShiftLEDbrightness); // turn the LED on
 if (RPM < 1600) analogWrite(ShiftLED, LOW); // turn the LED off
 */
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
