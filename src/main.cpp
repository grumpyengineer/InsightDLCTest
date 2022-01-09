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
    uint8_t message[10] = {0x68, 0x6a, 0xf5, 0xaf, 0xbf, 0xb3, 0xb2, 0xc1, 0xdb, 0xb3};

    // Send the init DLC message over the OBD
    bool res = obd.request(&message, 10, 4);

    delay(300);

    // Flush the serial buffer just to be sure
    while(Serial1.available()) 
    {
        Serial1.read();
    }
}

int8_t dlcRead(uint8_t address, uint8_t length, uint8_t * rxData) 
{
    uint32_t timeout = millis() + 250; // timeout @ 250 ms
    int8_t byteCount;
    uint8_t data, crc;

    // Calculate CRC
    crc = 0x20;
    crc += 0x05;
    crc += address;
    crc += length;
    crc = crc - 0x01;
    crc = 0xff - crc;

    // As we will be reading our own transmitted message
    // Start at -5
    byteCount = -5;

    // Send the DLC request
    Serial1.write(0x20);    // Read memory
    Serial1.write(0x05);    // Number of bytes to send
    Serial1.write(address); // Memory address
    Serial1.write(length);  // Number of bytes to read
    Serial1.write(crc);     // Checksum

    while (byteCount < (length + 3) && millis() < timeout) 
    {
        if (Serial1.available()) 
        {
            data = Serial1.read();
            if(byteCount > 0) // We've read back the transmitted message, the rest is real data!
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

    // Init the OBD first
    bool init_success =  obd.init();
    Serial.print("OBD2 init_success: ");
    Serial.println(init_success);

    // Then switch to DLC
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

