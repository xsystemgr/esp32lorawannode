#include "M5Atom.h"
#include "M5_LoRaWAN.h"
#include "freertos/queue.h"

M5_LoRaWAN LoRaWAN;

String response;

unsigned int transmissionCounter = 0;  
unsigned long energyMeterReading = 100;  // Αρχική τιμή του μετρητή ενέργειας

void setup() {
    M5.begin(true, false, true);
    M5.dis.fillpix(0xffff00);
    LoRaWAN.Init(&Serial2, 32, 26);
    delay(100);

    Serial.println("Module Connect.....");
    while (!LoRaWAN.checkDeviceConnect())
        ;
    M5.dis.fillpix(0x00ff00);
    LoRaWAN.writeCMD("AT?\r\n");
    delay(100);
    Serial2.flush();

    // Disable Log Information
    LoRaWAN.writeCMD("AT+ILOGLVL=0\r\n");
    // Enable  Log Information
    LoRaWAN.writeCMD("AT+CSAVE\r\n");
    LoRaWAN.writeCMD("AT+IREBOOT=0\r\n");
    Serial.println("LoraWan Rebooting");
    delay(1000);

    Serial.println("LoraWan config");
    // Set Join Mode OTAA.
    LoRaWAN.configOTTA("2cbbe4b01d35362b",                  // Device EUI
                       "0000000000000000",                  // APP EUI
                       "2ba46d9d0d49fd557f2082232e49094a",  // APP KEY
                       "2"  // Upload Download Mode
    );
    response = LoRaWAN.waitMsg(1000);
    Serial.println(response);
    // Set ClassC mode
    LoRaWAN.setClass("2");
    LoRaWAN.writeCMD("AT+CWORKMODE=2\r\n");

    // LoRaWAN868
    // TX Freq
    // 868.1 - SF7BW125 to SF12BW125
    // 868.3 - SF7BW125 to SF12BW125 and SF7BW250
    // 868.5 - SF7BW125 to SF12BW125
    // 867.1 - SF7BW125 to SF12BW125
    // 867.3 - SF7BW125 to SF12BW125
    // 867.5 - SF7BW125 to SF12BW125
    // 867.7 - SF7BW125 to SF12BW125
    // 867.9 - SF7BW125 to SF12BW125
    // 868.8 - FSK
    LoRaWAN.setFreqMask("0001");

    // 869.525 - SF9BW125 (RX2)              | 869525000
    LoRaWAN.setRxWindow("869525000");
    LoRaWAN.startJoin();
    transmissionCounter = 0; // Αρχικοποίηση του μετρητή
}

enum systemstate {
    kIdel = 0,
    kJoined,
    kSending,
    kWaitSend,
    kEnd,
};
int system_fsm = kIdel;

int loraWanSendNUM = -1;
int loraWanSendCNT = -1;

String waitRevice() {
    String recvStr;
    do {
        recvStr = Serial2.readStringUntil('\n');
    } while (recvStr.length() == 0);
    Serial.println(recvStr);
    return recvStr;
}

void loop() {
    String recvStr = waitRevice();

    if (recvStr.indexOf("+CJOIN:") != -1) {
        if (recvStr.indexOf("OK") != -1) {
            Serial.printf("LoraWan JOIN");
            system_fsm = kJoined;
        } else {
            Serial.printf("LoraWan JOIN FAIL");
            system_fsm = kIdel;
        }
    } else if (recvStr.indexOf("OK+RECV") != -1) {
        if (system_fsm == kJoined) {
            system_fsm = kSending;
        } else if (system_fsm == kWaitSend) {
            system_fsm = kSending;
            char strbuff[128];
            if ((loraWanSendCNT < 5) && (loraWanSendNUM == 8)) {
                sprintf(strbuff, "TSET OK CNT: %d", loraWanSendCNT);
                Serial.print(strbuff);
            } else {
                sprintf(strbuff, "FAILD NUM:%d CNT:%d", loraWanSendNUM,
                        loraWanSendCNT);
                Serial.print(strbuff);
            }
        }
    } else if (recvStr.indexOf("OK+SEND") != -1) {
        String snednum = recvStr.substring(8);
        Serial.printf(" [ INFO ] SEND NUM %s \r\n", snednum.c_str());
        loraWanSendNUM = snednum.toInt();
    } else if (recvStr.indexOf("OK+SENT") != -1) {
        String snedcnt = recvStr.substring(8);
        Serial.printf(" [ INFO ] SEND CNT %s \r\n", snedcnt.c_str());
        loraWanSendCNT = snedcnt.toInt();
    }

     if (system_fsm == kSending) {
        Serial.println("LoraWan Sending");

        
        String command = "AT+DTRX=1,8,8," + formatCounter(transmissionCounter) + "\r\n";
        LoRaWAN.writeCMD(command.c_str());

        transmissionCounter++;
        system_fsm = kWaitSend;
    }
    delay(10);
}


String formatCounter(unsigned int counter) {
    // Μήκος της συμβολοσειράς
    String formattedCounter = String(counter, DEC);
    while (formattedCounter.length() < 8) {
        formattedCounter = "0" + formattedCounter;
    }

    return formattedCounter;
}

//}
