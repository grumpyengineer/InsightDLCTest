#include <Arduino.h>

#include "OBD9141.h"

#define RX_PIN PA11
#define TX_PIN PA9
#define EN_PIN PA10

#define TXPIN   PA9

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
    while (millis() < timeOut) 
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
//	pinMode(TXPIN, OUTPUT);
//  	digitalWrite(TXPIN, HIGH);

    Serial.begin(115200);
    delay(5000);
    Serial.println("Bluepill Insight DLC Tester");
/*
    digitalWrite(TXPIN, LOW);
    delay(75);
    digitalWrite(TXPIN, HIGH);
    delay(150);
    Serial1.begin(9600);
    dlcInit();
*/

    obd.begin(Serial1, RX_PIN, TX_PIN);

    Serial.println("DLC Init Done");

    bool init_success =  obd.init();
    Serial.print("init_success:");
    Serial.println(init_success);

    delay(500);
    dlcInit();

}

void loop() 
{
    dlcCommand(0x20, 0x05, 0x08, 0x02);
    delay(1000);
}

/*
void loop(){
    Serial.println("Looping");



    //init_success = true;
    // Uncomment this line if you use the simulator to force the init to be
    // interpreted as successful. With an actual ECU; be sure that the init is 
    // succesful before trying to request PID's.

    // Trouble code consists of a letter and then four digits, we write this
    // human readable ascii string into the dtc_buf which we then write to the
    // serial port.
    uint8_t dtc_buf[5];

    if (init_success){
        uint8_t res;
        while(1){
            // res will hold the number of trouble codes that were received.
            // If no diagnostic trouble codes were retrieved it will be zero.
            // The ECU may return trouble codes which decode to P0000, this is
            // not a real trouble code but instead used to indicate the end of
            // the trouble code list.
            res = obd.readTroubleCodes();
            if (res){
                Serial.print("Read ");
                Serial.print(res);
                Serial.println(" codes:");
                for (uint8_t index = 0; index < res; index++)
                {
                  // retrieve the trouble code in its raw two byte value.
                  uint16_t trouble_code = obd.getTroubleCode(index);

                  // If it is equal to zero, it is not a real trouble code
                  // but the ECU returned it, print an explanation.
                  if (trouble_code == 0)
                  {
                    Serial.println("P0000 (reached end of trouble codes)");
                  }
                  else
                  {
                    // convert the DTC bytes from the buffer into readable string
                    OBD9141::decodeDTC(trouble_code, dtc_buf);
                    // Print the 5 readable ascii strings to the serial port.
                    Serial.write(dtc_buf, 5);
                    Serial.println();
                  }
                }
            }
            else
            {
              Serial.println("No trouble codes retrieved.");
            }
            Serial.println();

            delay(200);
        }
    }
    delay(3000);
}
*/
