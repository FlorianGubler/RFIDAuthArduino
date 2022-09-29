//RFID
#include <MFRC522.h>

#define PIN_RESET  6 // SPI Reset Pin
#define PIN_SS    7 // SPI Slave Select Pin

MFRC522 mfrc522(PIN_SS, PIN_RESET);

//Wifi
#include <WiFiNINA.h>

#define SECRET_SSID "iPhone"
#define SECRET_PASS "123456789"

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
char server[] = "172.20.10.4";    // IP Adress of API Host
int port = 8080;          //Port of API
WiFiClient client;

void setup()
{
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  
    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE) {
      Serial.println("Communication with WiFi module failed!");
      // don't continue
      while (true);
    }
  
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
      Serial.println("Please upgrade the firmware");
    }
  
    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
      delay(10000);
    }
  
    Serial.println("Connected to wifi");

    //Init RFID
    Serial.begin(9600);
    SPI.begin();
    Serial.println("Init RFID");
    mfrc522.PCD_Init();
    Serial.println("RFID ready");
}

void loop()
{
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        String rfidUID = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            rfidUID += mfrc522.uid.uidByte[i];
        }
        verifyRFIDCard(rfidUID);
        Serial.println();
        mfrc522.PICC_HaltA();
        delay(1000);
   }
   if(client.available()){
        String res = "";
        while (client.available()) {          // loop while the client's available
          char c = client.read();
          res += c;
        }
        String firstline = res.substring(0, res.indexOf('\n'));
        String ResponseCode = firstline.substring(res.indexOf(' ') + 1, firstline.length() - 2); // RM Line Space
        
        if(ResponseCode.equals("200")){
            Serial.println("CARD VALID");
        } else if(ResponseCode.equals("401")){
            Serial.println("CARD INVALID");
        } else{
            Serial.println("UNKOWN RESPONSE");
        }
   }
}

boolean verifyRFIDCard(String rfid_UID){
    Serial.println("Verifying RFID Card " + rfid_UID);
    // if you get a connection, report back via serial:
    if (client.connect(server, port)) {
      client.println("GET /api/members/verify/" + rfid_UID + " HTTP/1.1");
      client.println("Host: " + String(server));
      client.println("Connection: close");
      client.println();
    } else{
      Serial.println("Couldn't connect to server: " + String(server) + ":" + port);
    }
    return false;
}
