//RFID
#include <MFRC522.h>

#define PIN_RESET  6 // SPI Reset Pin
#define PIN_SS    7 // SPI Slave Select Pin

MFRC522 mfrc522(PIN_SS, PIN_RESET);

//Servo
#include <Servo.h>
Servo servo;  // create servo object to control a servo
int pos = 0; 
int opendeg = 90;

//Display
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

const int color_defaultR = 0;
const int color_defaultG = 0;
const int color_defaultB = 255;

const int color_errorR = 255;
const int color_errorG = 0;
const int color_errorB = 0;

const int color_warnR = 255;
const int color_warnG = 165;
const int color_warnB = 0;

const int color_validR = 0;
const int color_validG = 255;
const int color_validB = 0;

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

    //Servo
    servo.attach(7); 
    Serial.println("Attached Servo");

    //Display
    lcd.begin(16, 2);
    lcd.setRGB(color_defaultR, color_defaultG, color_defaultB);
    printToDisplay("READY", 1, true);

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
            lcd.setRGB(color_validR, color_validG, color_validB);
            printToDisplay("ACCESS GRANTED", 1, true);
            delay(1000);
            OpenWithServo();
        } else if(ResponseCode.equals("401")){
            Serial.println("CARD INVALID");
            lcd.setRGB(color_errorR, color_errorG, color_errorB);
            printToDisplay("ACCESS DENIED", 1, true);
            delay(2000);
            lcd.setRGB(color_defaultR, color_defaultG, color_defaultB);
            printToDisplay("READY", 1, true);
        } else{
            Serial.println("UNKOWN RESPONSE");
            lcd.setRGB(color_warnR, color_warnG, color_warnB);
            printToDisplay("INTERNAL ERROR", 1, true);
            printToDisplay("WAITING...", 1, false);
            lcd.setCursor(0, 0);
        }
        Serial.println("");
   }
}

void verifyRFIDCard(String rfid_UID){
    lcd.setRGB(color_defaultR, color_defaultG, color_defaultB);
    printToDisplay("WAITING...", 1, true);
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
}

void OpenWithServo(){
  for (pos = 0; pos <= opendeg; pos += 1) {
    // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
}

void printToDisplay(String inp, int line, boolean clear){
  if(clear){
      lcd.clear();
  }
  lcd.print(inp);
  lcd.setCursor(0, line);
}
