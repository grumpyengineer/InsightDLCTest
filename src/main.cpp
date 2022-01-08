#include <Arduino.h>

#define TXPIN   PA9

void PrintHex8(uint8_t data) // prints 8-bit data in hex with leading zeroes
{
    Serial.print("0x"); 
    if (data < 0x10)
    {
        Serial.print("0");
    } 
    Serial.print(data, HEX); 
    Serial.print(" "); 
}

void dlcInit(void)
{
    Serial1.write(0x68);  
    Serial1.write(0x6a);
    Serial1.write(0xf5);
    Serial1.write(0xaf);
    Serial1.write(0xbf);
    Serial1.write(0xb3);
    Serial1.write(0xb2);
    Serial1.write(0xc1);
    Serial1.write(0xdb);
    Serial1.write(0xb3);
    Serial1.write(0xe9);
    delay(300);
}

void dlcCommand(byte cmd, byte num, byte loc, byte len) 
{
    byte crc = (0xFF - (cmd + num + loc + len - 0x01)); // checksum FF - (cmd + num + loc + len - 0x01)

    unsigned long timeOut = millis() + 250; // timeout @ 250 ms

    Serial1.write(cmd);  // header/cmd read memory ??
    Serial1.write(num);  // num of bytes to send
    Serial1.write(loc);  // address
    Serial1.write(len);  // num of bytes to read
    Serial1.write(crc);  // checksum

    int i = 0;
    uint8_t data;

    Serial.print("Data: ");
    while (i < (len + 3 + 5) && millis() < timeOut) 
    {
        if (Serial1.available()) 
        {
            data = Serial1.read();
            PrintHex8(data);
            i++;
        }
    }
    Serial.println("END");
}

void setup() 
{
	pinMode(TXPIN, OUTPUT);
  	digitalWrite(TXPIN, HIGH);

    Serial.begin(115200);
    delay(5000);
    Serial.println("Bluepill Insight DLC Tester");

    digitalWrite(TXPIN, LOW);
    delay(75);
    digitalWrite(TXPIN, HIGH);
    delay(150);
    Serial1.begin(10400);
    dlcInit();

    Serial.println("DLC Init Done");
}

void loop() 
{
    dlcCommand(0x20, 0x05, 0x08, 0x02);
    delay(1000);
}