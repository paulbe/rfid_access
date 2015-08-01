//Custome rfid entrance code

#include <SoftwareSerial.h>
#include <Servo.h>

#define rxPin 8
#define txPin 6

#define BUFSIZE 12
#define RFID_Read 0x01
#define RFID_Write 0x02
#define RFID_Login 0x03  
#define RFID_SetPass 0x04 
#define RFID_Protect 0x05
#define RFID_Reset 0x06  
#define RFID_ReadLegacy 0x0F
#define ADDR_Serial 32
#define ADDR_Password 0
#define ERR_OK 0x01
#define LEGACY_StartByte  0x0A
#define LEGACY_StopByte   0x0D

byte data[4];
byte paul[4] = {0xBA,0x7A,0x7A,0x87};
int ledOn = 13;
int servoPin = 9;


//Door lock
Servo doorLock;

//Serial port setup
SoftwareSerial rfidSerial = SoftwareSerial(rxPin, txPin);


void setup() 
{
  doorLock.attach(servoPin);
  
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  //Serial monitor setup
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Ready to read");

  //Set baud rate
  rfidSerial.begin(9600);

  Serial.flush();

}

void loop() 
{
  char idx;
  uint8_t rfidData[BUFSIZE];

  //Read loop
  Serial.print("Serial Number: ");
  while (rfidRead(rfidData, ADDR_Serial) != 0);
  PrintHex(rfidData, 4);
  Serial.println();
  Serial.flush();

  //Login
  Serial.print("Logging into the tag...");
  while (rfidLogin((uint8_t*)"\xBA\x7A\x7A\x87") != 0);  // Login to tag with default password of 0x00000000
  Serial.println("Done!");
  digitalWrite(ledOn, HIGH);
  delay(1000);
  digitalWrite(ledOn, LOW);
  doorLock.write(180);
  delay(5000);
  doorLock.write(0);
  Serial.flush();
  
/*  //ID card
  boolean paul_card = true;
  Serial.print("Card found - Code: ");
  for (int i=0; i<4; i++)
  {
    if (data[i] < 16) Serial.print("0");
    Serial.print(data[i], HEX);

    //cross check
    if (data[i] != paul[i]) paul_card = false;
  }
  Serial.println();

  if (paul_card)
  {
    Serial.println("Hey");
  }
  else { Serial.println("Piss off"); }

  Serial.println();
*/

}



//snagged from arduinocc
/*** SUPPORTING FUNCTIONS ***/

char rfidRead(uint8_t *data, uint8_t address)
{
  char offset;  // offset into buffer
  
  rfidFlush();  // empties any buffered incoming serial data
  
  rfidSerial.print("!RW");       // header
  rfidSerial.write(RFID_Read);   // command
  rfidSerial.write(address);     // address
    
  while (rfidSerial.available() != 5);  // wait until data is received from RFID module
  
  if (rfidSerial.read() == ERR_OK)  // if our status byte is OK
  {
    for (offset = 0; offset < 4; offset++)
      data[offset] = rfidSerial.read();  // get the remaining data
       
    return 0; // return good
  }
  
  return -1; // return error
}


char rfidWrite(uint8_t address, uint8_t *data)
{
  char offset;  // offset into buffer
  
  rfidFlush();  // empties any buffered incoming serial data
  
  rfidSerial.print("!RW");        // header
  rfidSerial.write(RFID_Write);   // command
  rfidSerial.write(address);      // address
  for (offset = 0; offset < 4; offset++)
    rfidSerial.write(data[offset]);     // data (4 bytes) 
  
  while (rfidSerial.available() == 0); // wait until data is received from RFID module

  if (rfidSerial.read() == ERR_OK) // if our status byte is OK
    return 0; // return good
  
  return -1; // return error
}


char rfidLogin(uint8_t *password)
{
  char offset;  // offset into buffer
  
  rfidFlush();  // empties any buffered incoming serial data
  
  rfidSerial.print("!RW");        // header
  rfidSerial.write(RFID_Login);   // command
  for (offset = 0; offset < 4; offset++)
    rfidSerial.write(password[offset]);     // password (4 bytes) 
  
  while (rfidSerial.available() == 0); // wait until data is received from RFID module

  if (rfidSerial.read() == ERR_OK) // if our status byte is OK
    return 0; // return good
  
  return -1; // return error
}


char rfidSetPass(uint8_t *currentpass, uint8_t *newpass)
{
  char offset;  // offset into buffer
  
  rfidFlush();  // empties any buffered incoming serial data
  
  rfidSerial.print("!RW");          // header
  rfidSerial.write(RFID_SetPass);   // command
  for (offset = 0; offset < 4; offset++)
    rfidSerial.write(currentpass[offset]);  // current password (4 bytes) 
  for (offset = 0; offset < 4; offset++)
    rfidSerial.write(newpass[offset]);      // new password (4 bytes) 
    
  while (rfidSerial.available() == 0); // wait until data is received from RFID module

  if (rfidSerial.read() == ERR_OK) // if our status byte is OK
    return 0; // return good
  
  return -1; // return error
}


char rfidProtect(boolean enable)
{
  rfidFlush();  // empties any buffered incoming serial data
  
  rfidSerial.print("!RW");          // header
  rfidSerial.write(RFID_Protect);   // command  
  if (enable == false)              // flag
    rfidSerial.write((byte)0);
  else
    rfidSerial.write((byte)1);
  
  while (rfidSerial.available() == 0); // wait until data is received from RFID module

  if (rfidSerial.read() == ERR_OK) // if our status byte is OK
    return 0; // return good
  
  return -1; // return error    
}


char rfidReset()
{
  rfidFlush();  // empties any buffered incoming serial data
  
  rfidSerial.print("!RW");        // header
  rfidSerial.write(RFID_Reset);   // command  
  
  while (rfidSerial.available() == 0); // wait until data is received from RFID module

  if (rfidSerial.read() == ERR_OK) // if our status byte is OK
    return 0; // return good
  
  return -1; // return error    
}


char rfidReadLegacy(uint8_t *data)
{
  char offset;  // offset into buffer
  
  rfidFlush();  // empties any buffered incoming serial data
  
  rfidSerial.print("!RW");             // header
  rfidSerial.write(RFID_ReadLegacy);   // command
    
  while (rfidSerial.available() != 12);  // wait until data is received from RFID module
  
  if (rfidSerial.read() == LEGACY_StartByte)  // if our start byte is OK
  {
    for (offset = 0; offset < 10; offset++)
      data[offset] = rfidSerial.read();  // get the remaining data
       
    data[offset] = 0;  // null terminate the string of bytes we just received   
    return 0; // return good
  }
  
  return -1; // return error
}


void rfidFlush()  // Empties any buffered incoming serial data
{
    while (rfidSerial.available() > 0)
      rfidSerial.read();
}


// from http://forum.arduino.cc/index.php?topic=38107#msg282343
void PrintHex(uint8_t *data, uint8_t length) // prints 8-bit data in hex
{
  char tmp[length*2+1];
  byte first ;
  int j=0;
  for (uint8_t i=0; i<length; i++) 
  {
    first = (data[i] >> 4) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first ;
    j++;

    first = (data[i] & 0x0F) | 48;
    if (first > 57) tmp[j] = first + (byte)39; 
    else tmp[j] = first;
    j++;
  }
  tmp[length*2] = 0;
  Serial.print(tmp);
}

