#include "M5Atom.h"
#include "M5_LoRaWAN.h"
#include "freertos/queue.h"
<<<<<<< Updated upstream
=======
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <EEPROM.h>
>>>>>>> Stashed changes

M5_LoRaWAN LoRaWAN;

String response;
<<<<<<< Updated upstream

unsigned int transmissionCounter = 0;  // Αυξονόμος αριθμός
unsigned long energyMeterReading = 100;  // Αρχική τιμή του μετρητή ενέργειας
=======
String serialNumber = "";

// WiFi credentials
const char *ssid = "KALAMARAKI_1";
const char *password = "t2024!@#";

AsyncWebServer server(80);

struct Config {
    float voltage;
    float current;
    float powerFactor;
    float frequency;
};

Config config;

unsigned int transmissionCounter = 0;

void loadConfig() {
    EEPROM.get(0, config);
}

void saveConfig() {
    EEPROM.put(0, config);
    EEPROM.commit();
}

void loadDefaultValues() {
    loadConfig();

    if (config.voltage == 0.0) {
        config.voltage = 237;
    }
    if (config.current == 0.0) {
        config.current = 0.0;
    }
    if (config.powerFactor == 0.0) {
        config.powerFactor = 1.0;
    }
    if (config.frequency == 0.0) {
        config.frequency = 50;
    }
}
>>>>>>> Stashed changes

void setup() {
    M5.begin(true, false, true);
    M5.dis.fillpix(0xffff00);
    LoRaWAN.Init(&Serial2, 32, 26);
    delay(100);

<<<<<<< Updated upstream
    Serial.println("Module Connect.....");
    while (!LoRaWAN.checkDeviceConnect())
        ;
=======
    // WiFi connection
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Print IP address
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Web server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html><body>";
        html += "<h1>Device Configuration</h1>";
        html += "<form action='/save' method='post'>";
        html += "Voltage: <input type='text' name='voltage' value='" + String(config.voltage) + "'><br>";
        html += "Current: <input type='text' name='current' value='" + String(config.current) + "'><br>";
        html += "Power Factor: <input type='text' name='powerFactor' value='" + String(config.powerFactor) + "'><br>";
        html += "Frequency: <input type='text' name='frequency' value='" + String(config.frequency) + "'><br>";
        html += "<input type='submit' value='Save'>";
        html += "</form>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
        String voltage = request->getParam("voltage")->value();
        String current = request->getParam("current")->value();
        String powerFactor = request->getParam("powerFactor")->value();
        String frequency = request->getParam("frequency")->value();

        // Process and save the values
        config.voltage = voltage.toFloat();
        config.current = current.toFloat();
        config.powerFactor = powerFactor.toFloat();
        config.frequency = frequency.toFloat();

        saveConfig();

        request->send(200, "text/plain", "Values saved successfully");
    });

    server.begin();

    // Load default values
    loadDefaultValues();

    

	Serial.println("Module Connect.....");
    while (!LoRaWAN.checkDeviceConnect());
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    LoRaWAN.configOTTA("2cbbe4b01d35362b",                  // Device EUI
                       "0000000000000000",                  // APP EUI
                       "2ba46d9d0d49fd557f2082232e49094a",  // APP KEY
                       "2"  // Upload Download Mode
    );
=======
    LoRaWAN.configOTTA("0bfea6decf80ad4d", "0000000000000000", "54b90c764a3767ce6c7c62a28d98e70b", "2");
>>>>>>> Stashed changes
    response = LoRaWAN.waitMsg(1000);
    Serial.println(response);
    // Set ClassC mode
    LoRaWAN.setClass("2");
    LoRaWAN.writeCMD("AT+CWORKMODE=2\r\n");

<<<<<<< Updated upstream
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

=======
    LoRaWAN.setFreqMask("0001");

    // 869.525 - SF9BW125 (RX2)
    // 869525000
    LoRaWAN.setRxWindow("869525000");

    // 868.300 - SF9BW125 (RX2)
    // 868300000

    LoRaWAN.startJoin();

    transmissionCounter = 0;
}




>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
=======
// Συνάρτηση για την ανάγνωση του σειριακού αριθμού
String readSerialNumber() {
    LoRaWAN.writeCMD("AT+CGSN?\r\n");
    String response = LoRaWAN.waitMsg(1000);

    // Εξαγωγή του σειριακού αριθμού από την απάντηση
    int index = response.indexOf("+CGSN=");
    if (index != -1) {
        serialNumber = response.substring(index + 7, index + 23);
    } else {
        serialNumber = "Unknown";
    }

    return serialNumber;
}

>>>>>>> Stashed changes
String waitRevice() {
    String recvStr;
    do {
        recvStr = Serial2.readStringUntil('\n');
    } while (recvStr.length() == 0);
    Serial.println(recvStr);
    return recvStr;
}

<<<<<<< Updated upstream
=======



>>>>>>> Stashed changes
void loop() {
    String recvStr = waitRevice();

    if (recvStr.indexOf("+CJOIN:") != -1) {
        if (recvStr.indexOf("OK") != -1) {
<<<<<<< Updated upstream
            Serial.printf("LoraWan JOIN");
            system_fsm = kJoined;
        } else {
            Serial.printf("LoraWan JOIN FAIL");
=======
            Serial.println("LoraWan JOIN");
            system_fsm = kJoined;
        } else {
            Serial.println("LoraWan JOIN FAIL");
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
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
=======
                sprintf(strbuff, "FAILD NUM:%d CNT:%d", loraWanSendNUM, loraWanSendCNT);
                Serial.print(strbuff);
            }
        }
    }

    if (system_fsm == kSending) {
        Serial.println("LoraWan Sending");

        // Υποθετικές τιμές για τις μεταβλητές
        float voltage = 237.217163;
        float current = 0.0;
        float powerFactor = 1.0;
        float frequency = 50.0488777;

        // Τιμή μετρητή kWh
        float totalKWh = generateTestMeterReading();

        // Τελική εντολή UPLINK
       // String payload1 = serialNumber + "010C" + floatToHex(totalKWh) + floatToHex(voltage);
        String payload1 = "01354BEC010C" + floatToHex(totalKWh) + floatToHex(voltage);
        String command1 = "AT+DTRX=1," + String(payload1.length()) + "," + String(payload1.length()) + "," + payload1 + "\r\n";
        LoRaWAN.writeCMD(command1.c_str());

        // Τελική εντολή UPLINK για Step2
        String payload2 = "01354BEC020C00000000";
        String command2 = "AT+DTRX=2," + String(payload2.length()) + "," + String(payload2.length()) + "," + payload2 + "\r\n";
        LoRaWAN.writeCMD(command2.c_str());  

        system_fsm = kWaitSend;
    }
    delay(50);
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
>>>>>>> Stashed changes
