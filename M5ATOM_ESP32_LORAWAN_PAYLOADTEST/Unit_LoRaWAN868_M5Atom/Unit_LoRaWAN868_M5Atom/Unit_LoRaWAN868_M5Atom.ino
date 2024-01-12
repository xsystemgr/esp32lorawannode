#include "M5Atom.h"
#include "M5_LoRaWAN.h"
#include "freertos/queue.h"

M5_LoRaWAN LoRaWAN;

String response;

unsigned int transmissionCounter = 0;  // Αυξονόμος αριθμός
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
    LoRaWAN.configOTTA("0bfea6decf80ad4d",                  // Device EUI
                       "0000000000000000",                  // APP EUI
                       "54b90c764a3767ce6c7c62a28d98e70b",  // APP KEY
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
    // 868.300 - SF9BW125 (RX2)              | 868300000

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
        //Note: The following data is displayed in Hex format.

        //Step1: 00 BC 61 4E 01 0C 43 5C 00 00 42 C8 00 00 46 AB E0 00 2D A6, 2D A6 means CRC;
        //Step2: 00 BC 61 4E 02 0C 42 F4 AE 14 00 00 00 00 00 00 00 00 82 3E, 82 3E means CRC;
        //00 BC 61 4E means the serial number （0x 00 BC 61 4E = 12345678）;
       // 43 5C 00 00 42 C8 00 00 46 AB E0 00 means L1 Voltage, L1 Amps, Total active power；
       // 42 F4 AE 14 means Total kWh；


        

        // Tιμή μετρητή kWh
        //float totalKWh = 122.34;
        // Κλήση της συνάρτησης generateTestMeterReading() για να λάβετε μια εναλλακτική τιμή
        float totalKWh = generateTestMeterReading();


        // Στοιχεία δεδομένων που στέλνονται στο LoRaWAN
        String payload = "00BC614E020C" + floatToHex(totalKWh);

        // Υπολογισμός CRC
        String crc = calculateCRC(payload);

        // Τελική εντολή UPLINK
        String command = "AT+DTRX=1," + String(payload.length()) + "," + String(payload.length()) + "," + payload + crc + "\r\n";
        LoRaWAN.writeCMD(command.c_str());

        system_fsm = kWaitSend;
    }
    delay(10);
}


float generateTestMeterReading() {
    // Υποθετική συνάρτηση που παράγει εναλλακτικές τιμές για τον μετρητή
    static float testMeterReading = 100.0; // Αρχική τιμή

    // Προσομοιώνουμε μια αύξηση του μετρητή σε κάθε κλήση της συνάρτησης
    testMeterReading += 1.5;

    return testMeterReading;
}

String floatToHex(float value) {
    // Μετατροπή του float σε δεκαεξαδική μορφή
    uint32_t intValue = *(uint32_t*)&value;
    char buffer[9];
    sprintf(buffer, "%08X", intValue);
    return String(buffer);
}

String calculateCRC(String data) {
    // Υπολογισμός CRC
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < data.length(); i++) {
        crc ^= data.charAt(i);
        for (int j = 0; j < 8; j++) {
            if ((crc & 0x01) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    char buffer[5];
    sprintf(buffer, "%04X", crc);
    return String(buffer);
}

