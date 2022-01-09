#include <Arduino.h>

#include "OBD9141.h"

#define RX_PIN PA11
#define TX_PIN PA9

OBD9141 obd;


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
    while(Serial1.available()) 
    {
        Serial1.read();
    }
}

int8_t dlcRead(uint8_t address, uint8_t length, uint8_t * rxData) 
{
    uint8_t crc;
    uint32_t timeout = millis() + 250; // timeout @ 250 ms
    int8_t byteCount;
    uint8_t data;

    // Calculate CRC
    crc = 0x20;
    crc += 0x05;
    crc += address;
    crc += length;
    crc = crc - 0x01;
    crc = 0xff - crc;

    byteCount = -5;

    Serial1.write(0x20);     // header/cmd read memory
    Serial1.write(0x05);        // num of bytes to send
    Serial1.write(address);  // address
    Serial1.write(length);  // num of bytes to read
    Serial1.write(crc);  // checksum

    while (byteCount < (length + 3) && millis() < timeout) 
    {
        if (Serial1.available()) 
        {
            data = Serial1.read();
            if(byteCount > 0)
                rxData[byteCount] = data;
            byteCount++;
        }
    }

    return byteCount;
}

void setup() 
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("Bluepill Insight DLC Tester");

    obd.begin(Serial1, RX_PIN, TX_PIN);

    bool init_success =  obd.init();
    Serial.print("OBD2 init_success: ");
    Serial.println(init_success);

    delay(500);
    dlcInit();

    Serial.println("DLC Init Done");

}

void loop() 
{
    uint8_t rxData[6];
    int8_t res;
    
    // Get Clutch and Brake Pedal
    res = dlcRead(0x08, 0x02, rxData);

    if(res > 0)
    {
        Serial.print("Brake Pedal: ");
        if(rxData[2] & 0x08)
            Serial.println("Pressed");
        else   
            Serial.println(" ");
        Serial.print("Clutch Pedal: ");
        if(rxData[3] & 0x02)
            Serial.println("Pressed");
        else   
            Serial.println(" ");            
    }

    res = dlcRead(0x14, 0x01, rxData);

    if(res > 0)
    {
        Serial.print("TPS: ");
        Serial.print((double)(rxData[2]/2.6));
        Serial.println("%");
    }
    delay(1000);
}

