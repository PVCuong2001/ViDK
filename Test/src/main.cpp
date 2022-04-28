#include <SPI.h>
#include <MFRC522.h>
#include <string>
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

#define SS_PIN D8
#define RST_PIN D0
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

bool checkDoor = false;
int countFalse = 0;
bool trigger = false;

std::string label = "";
std::string data = "";
std::string num_str1 = "";
// Init array that will store new NUID
byte nuidPICC[4];
#define LED D3
int checkRFID = 0;
int isRfidTurnOff = 0;
unsigned long timer, timer2 = 0;

unsigned long readCardTimer = 0;
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{

  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
  {
    Serial.printf("[WSc] Connected to url: %s\n", payload);
    break;
  }

  case WStype_TEXT:
    if (payload)
    {
      Serial.printf("[WSc] get text: %s\n", payload);
      if (strcmp((char *)payload, "DoorOn") == 0)
      {
        trigger = true;
        checkDoor = true;
        countFalse = 0;
      }
      else if (strcmp((char *)payload, "DoorOff") == 0)
      {
        trigger = true;
        checkDoor = false;
        countFalse = 0;
      }
      else if (strcmp((char *)payload, "StopWarning") == 0)
      {
        countFalse = 0;
      }
    }

    break;
  }
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
}

void loop()
{
  webSocket.loop();
  if (countFalse > 3)
  {
    digitalWrite(LED, LOW);
    delay(300);
    digitalWrite(LED, HIGH);
    delay(300);
  }
  if (trigger == true)
  {
    if (checkDoor)
    {
      servo.write(180);
      digitalWrite(LED, HIGH);
      timer = millis();
      label = "door:";
      data = "on";
    }
    else
    {
      servo.write(0);
      if (countFalse <= 3)
      {
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
        label = "door:";
        data = "off";
      }
      else
      {
        label = "door:";
        data = "warning";
      }
    }
    num_str1 = label + data;
    const char *convertido = num_str1.c_str();
    webSocket.sendTXT(convertido);
    trigger = false;
  }

  if (checkDoor == true && millis() - timer > 5000)
  {
    servo.write(0);
    checkDoor = false;
    countFalse = 0;
    trigger = true;
  }

  if (millis() - readCardTimer >= 1000)
  {
    if (rfid.PICC_IsNewCardPresent())
    {
      if (rfid.PICC_ReadCardSerial())
      {
        readCardTimer = millis();
        Serial.print("UID tag :");
        String content = "";

        for (byte i = 0; i < rfid.uid.size; i++)
        {
          content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
          content.concat(String(rfid.uid.uidByte[i], HEX));
        }
        content.toUpperCase();
        trigger = true;
        if (content.substring(1) == "E1 71 88 20") // change here the UID of the card/cards that you want to give access
        {
          Serial.println("Authorized access");
          countFalse = 0;
          checkDoor = true;
        }

        else
        {
          Serial.println("Authorized denied");
          checkDoor = false;
          countFalse += 1;
        }
      }
    }
    readCardTimer = millis();
  }
}