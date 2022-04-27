#include <Arduino.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets
#include <string>
using namespace std;
#define DHTTYPE DHT11
#define DHTPIN 15 // DHT 11
const uint8_t servoPin = D2;
Servo servo;
DHT dht(DHTPIN, DHTTYPE);
float total_temp = 0;
float total_hum = 0;
float mycount = 0;

WebSocketsClient webSocket;
// const char *ssid = "Wifi";              //Đổi thành wifi của bạn
// const char *password = "PVCuong016867"; //Đổi pass luôn
// const char *ip_host = "192.168.1.19";   //Đổi luôn IP host của PC nha
// const char *ssid = "pass=MD5('12341234')"; //Đổi thành wifi của bạn
// const char *password = "loi!1234";         //Đổi pass luôn
// const char *ip_host = "192.168.43.218";    //Đổi luôn IP host của PC nha
// const char *ssid = "Redmi 10";
// const char *password = "2001Cuong"; //Đổi thành wifi của bạn        //Đổi pass luôn
// const char *ip_host = "192.168.45.218";

const char *ssid = "???";               //Đổi thành wifi của bạn
const char *password = "06202001";      //Đổi pass luôn
const char *ip_host = "192.168.43.131"; //Đổi luôn IP host của PC nha
const uint16_t port = 3000;             // Port thích đổi thì phải đổi ở server nữa
bool is_auto = false;
float T1 = 25;
float T2 = 28;
float T3 = 30;
unsigned long timer = millis();
bool checkDoor = false;
unsigned long mytime = 0;
void SpeedState(int status)
{
    switch (status)
    {
    case 0:
        digitalWrite(D5, 0); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
        digitalWrite(D6, 0);
        digitalWrite(D7, 0);
        break;
    case 1:
        digitalWrite(D5, 1); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
        digitalWrite(D6, 0);
        digitalWrite(D7, 0);
        break;
    case 2:
        digitalWrite(D5, 1); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
        digitalWrite(D6, 1);
        digitalWrite(D7, 0);
        break;
    case 3:
        digitalWrite(D5, 1); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
        digitalWrite(D6, 1);
        digitalWrite(D7, 1);
        break;
    }
}
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    string label = "";
    string data = "";
    string num_str1 = "";
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

        if (std::string((char *)payload).find("SetT1") != string::npos)
        {
            Serial.println("SetT1");
            string s = string((char *)payload);
            T1 = stoi(string((char *)payload).substr(6, s.size()));
            label = "T1:";
            data = std::to_string(T1);
        }
        else if (std::string((char *)payload).find("SetT2") != string::npos)
        {
            Serial.println("SetT2");
            string s = string((char *)payload);
            T2 = stoi(string((char *)payload).substr(6, s.size()));
            label = "T2:";
            data = std::to_string(T2);
        }
        else if (std::string((char *)payload).find("SetT3") != string::npos)
        {
            Serial.println("SetT3");
            string s = string((char *)payload);
            T3 = stoi(string((char *)payload).substr(6, s.size()));
            label = "T3:";
            data = std::to_string(T3);
        }
        else if (strcmp((char *)payload, "AutoOff") == 0)
        {
            Serial.println("AutoOFf");
            is_auto = false;
            label = "auto:";
            data = "off";
        }
        else if (strcmp((char *)payload, "AutoOn") == 0)
        {
            Serial.println("AutoOn");
            is_auto = true;
            label = "auto:";
            data = "on";
        }
        else if (strcmp((char *)payload, "Speed0") == 0)
        {
            Serial.printf("Speed0");
            SpeedState(0);
            label = "Speed0:";
            data = "none";
        }
        else if (strcmp((char *)payload, "Speed1") == 0)
        {
            Serial.printf("Speed1");
            SpeedState(1);
            label = "Speed1:";
            data = "none";
        }
        else if (strcmp((char *)payload, "Speed2") == 0)
        {
            Serial.printf("Text off");
            SpeedState(2);
            label = "Speed2:";
            data = "none";
        }
        else if (strcmp((char *)payload, "Speed3") == 0)
        {
            Serial.printf("Speed3");
            SpeedState(3);
            label = "Speed3:";
            data = "none";
        }
        else if (strcmp((char *)payload, "Init") == 0)
        {
            label = "init:";
            data = std::to_string(is_auto) + " " + std::to_string(T1) + " " + std::to_string(T2) + " " + std::to_string(T3);
        }
        else if (strcmp((char *)payload, "DoorOn") == 0)
        {
            servo.write(180);
            checkDoor = true;
            timer = millis();
            label = "door:";
            data = "on";
        }
        else if (strcmp((char *)payload, "DoorOff") == 0)
        {
            servo.write(0);
            checkDoor = false;
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

    // pinMode(LED, OUTPUT);
    servo.attach(servoPin);
    pinMode(D5, OUTPUT);
    // pinMode(2, OUTPUT);
    pinMode(D7, OUTPUT);
    pinMode(D6, OUTPUT);
    Serial.begin(9600);
    Serial.println("ESP8266 Websocket Client");
    WiFi.begin(ssid, password);
    // WiFi.begin(ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    webSocket.begin(ip_host, port);
    webSocket.onEvent(webSocketEvent);
    dht.begin();
}
void loop()
{
    if (checkDoor == true && millis() - timer > 5000)
    {
        servo.write(0);
        checkDoor = false;
        webSocket.sendTXT("door:off");
    }

    webSocket.loop();
    float h = dht.readHumidity();

    float t = dht.readTemperature();

    Serial1.println(h);
    Serial1.println(t);

    // Check if any reads failed and exit early(to try again).if (isnan(h) || isnan(t))
    // {
    //     Serial.println("Failed to read from DHT sensor!");
    //     return;
    // }
    // Compute heat index in Celsius (isFahreheit = false)
    // float hic = dht.computeHeatIndex(t, h, false);
    total_temp += t;
    total_hum += h;
    mycount += 1;

    if (millis() - mytime >= 2000)
    {
        string num_str1("temphum:" + std::to_string(total_temp / mycount) + " " + std::to_string(total_hum / mycount));
        const char *convertido = num_str1.c_str();
        webSocket.sendTXT(convertido);
        mytime = millis();

        if (is_auto)
        {
            float avg_temp = total_temp / mycount;
            Serial.println(avg_temp);
            if (avg_temp > T3)
                SpeedState(3);
            else if (avg_temp > T2)
                SpeedState(2);
            else if (avg_temp > T1)
                SpeedState(1);
            else
                SpeedState(0);
        }
        total_hum = 0;
        total_temp = 0;
        mycount = 0;
    }
}
