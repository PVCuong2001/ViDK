#include <SPI.h>
#include <MFRC522.h>
#include <string>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Servo.h>

WebSocketsClient webSocket;
const char *ssid = "Wifi";              //Đổi thành wifi của bạn
const char *password = "PVCuong016867"; //Đổi pass luôn
const char *ip_host = "192.168.1.19";
const uint16_t port = 3000;

const uint8_t servoPin = D2;
Servo servo;

//----DHT11----//
#define DHTTYPE DHT11
#define DHTPIN 13
DHT dht(DHTPIN, DHTTYPE);

bool is_auto = false;
float T1 = 25;
float T2 = 28;
float T3 = 30;

float total_temp = 0;
float total_hum = 0;
float mycount = 0;
unsigned long mytime = 0;
//----DHT 11----//

#define SS_PIN D8
#define RST_PIN D0
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

bool checkDoor = false;

// Init array that will store new NUID
byte nuidPICC[4];
#define LED D3
int checkRFID = 0;
int isRfidTurnOff = 0;
unsigned long timer, timer2 = 0;

void SpeedState(int status)
{
  // switch (status)
  // {
  // case 0:
  //   digitalWrite(D5, 0); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
  //   digitalWrite(D6, 0);
  //   digitalWrite(D7, 0);
  //   break;
  // case 1:
  //   digitalWrite(D5, 1); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
  //   digitalWrite(D6, 0);
  //   digitalWrite(D7, 0);
  //   break;
  // case 2:
  //   digitalWrite(D5, 1); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
  //   digitalWrite(D6, 1);
  //   digitalWrite(D7, 0);
  //   break;
  // case 3:
  //   digitalWrite(D5, 1); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
  //   digitalWrite(D6, 1);
  //   digitalWrite(D7, 1);
  //   break;
  // }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  std::string label = "";
  std::string data = "";
  std::string num_str1 = "";
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
  {
    Serial.printf("[WSc] Connected to url: %s\n", payload);
  }
  break;
  case WStype_TEXT:

    Serial.printf("[WSc] get text: %s\n", payload);
    if (strcmp((char *)payload, "DoorOn") == 0)
    {
      servo.write(180);
      checkDoor = true;
      digitalWrite(LED, HIGH);
      timer = millis();
      label = "door:";
      data = "on";
    }
    else if (strcmp((char *)payload, "DoorOff") == 0)
    {
      servo.write(0);
      checkDoor = false;
      digitalWrite(LED, LOW);
      label = "door:";
      data = "off";
    }

    break;
  case WStype_BIN:
    Serial.printf("[WSc] get binary length: %u\n", length);
    Serial.printf("[WSc] get text: %s\n", payload);

    break;
  }
  num_str1 = label + data;
  const char *convertido = num_str1.c_str();
  webSocket.sendTXT(convertido);
}

void setup()
{
  Serial.begin(9600); // Initiate a serial communication
  SPI.begin();        // Initiate  SPI bus
  rfid.PCD_Init();    // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  pinMode(LED, OUTPUT);

  WiFi.begin(ssid, password);
  // WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  webSocket.begin(ip_host, port);
  webSocket.onEvent(webSocketEvent);

  servo.attach(servoPin);
  // dht.begin();
  // digitalWrite(LED, LOW);
}
void loop()
{
  webSocket.loop();
  if (isRfidTurnOff == 1)
  {
    digitalWrite(LED, LOW);
    delay(2000);
    isRfidTurnOff = 0;
    checkRFID = 0;
  }
  else if (checkRFID == 1)
  {
    Serial.println("High");
    digitalWrite(LED, HIGH);
    delay(300);
  }
  else if (checkRFID == 2)
  {
    Serial.println("Low-low");
    digitalWrite(LED, LOW);
    delay(300);
    Serial.println("Low-high");
    digitalWrite(LED, HIGH);
    delay(300);
  }

  if (rfid.PICC_IsNewCardPresent())
  {
    if (rfid.PICC_ReadCardSerial())
    {
      Serial.print("UID tag :");
      String content = "";

      for (byte i = 0; i < rfid.uid.size; i++)
      {
        content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(rfid.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      if (content.substring(1) == "E1 71 88 20") // change here the UID of the card/cards that you want to give access
      {
        Serial.println("Authorized access");
        if (checkRFID == 1)
        {
          if (millis() - timer2 < 1000)
          {
            isRfidTurnOff = 1;
          }
        }
        timer2 = millis();
        checkRFID = 1;
        // isturnoff +=1;
        Serial.println(isRfidTurnOff);
      }

      else
      {
        Serial.println("Authorized denied");
        checkRFID = 2;
        isRfidTurnOff = 0;
      }
    }
  }

  if (checkDoor == true && millis() - timer > 5000)
  {
    servo.write(0);
    checkDoor = false;
    webSocket.sendTXT("door:off");
  }
}