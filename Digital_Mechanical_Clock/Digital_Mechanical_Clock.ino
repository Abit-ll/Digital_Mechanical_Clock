#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_PWMServoDriver.h>

const char *ssid = "ll";
const char *passwd = "fgak98765";

#define RED_LED_PIN             D1
#define GREEN_LED_PIN           D2
#define BLUE_LED_PIN            D4
#define RELAY_IN_PIN            D0

#define DEVICENAME              "LD_001_Led"
#define PRODUCTID               "VCRW1OP7O1"
#define DEVICEKEY               "vYdZL8gTWEYYgFNjbd5k4Q=="
#define MQTT_SERVER             PRODUCTID ".iotcloud.tencentdevices.com"
#define MQTT_PORT               1883
#define CLIENT_ID               PRODUCTID DEVICENAME
#define MQTT_USERNAME           "VCRW1OP7O1LD_001_Led;12010126;QCXH0;1629097050"
#define MQTT_PASSWD             "98fc94b78ddf9b62ac816d60bbdf251ec51d7f7ed9ea58ee81c0da7c3e445a85;hmacsha256"

#define MQTT_CTRL_TOPIC         "$thing/down/property/" PRODUCTID"/"DEVICENAME
#define MQTT_CTRL_REPLY_TOPIC   "$thing/up/property/" PRODUCTID"/"DEVICENAME

typedef enum
{
    red = 0,
    green,
    blue
}color;

uint16_t brightness;
uint8_t power_switch;
color light_color;

WiFiClient espClient;
PubSubClient client(espClient);

void decodeJson(char msg[100])
{
    DynamicJsonDocument jsonBuffer(1024);
    deserializeJson(jsonBuffer, msg);
    power_switch = jsonBuffer["params"]["power_switch"];
    brightness = jsonBuffer["params"]["brightness"];
    light_color = jsonBuffer["params"]["color"];
}

void light_ctrl()
{    
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(RELAY_IN_PIN, LOW);

    if(power_switch)
    {
        digitalWrite(RELAY_IN_PIN, HIGH);
        switch(light_color)
        {
            case red:
                digitalWrite(RED_LED_PIN, LOW);
                break;

            case green:
                digitalWrite(GREEN_LED_PIN, LOW);
                break;

            case blue:
                digitalWrite(BLUE_LED_PIN, LOW);
                break;

            defalut:
                break;
        }
    }
}

void mqtt_callback(char *topic, byte *payload, unsigned int len)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.print("\r\n");
    decodeJson((char *)payload);
    light_ctrl();
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, passwd);
    while(!WiFi.isConnected())
    {
        Serial.print("connect wifi failed!\r\n");
        delay(1000);
    }
    Serial.print("connect wifi successed!\r\n");
    Serial.print("IP Address: ");
    Serial.print(WiFi.localIP());
    Serial.print("\r\n");
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RELAY_IN_PIN, OUTPUT);
}

void loop() {
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqtt_callback);
    while(!client.connected())
    {
        Serial.print("Attempting MQTT connection...\r\n");
        if(client.connect(CLIENT_ID, MQTT_USERNAME, MQTT_PASSWD))
        {
            Serial.print("Connected...\r\n");
        }
        delay(1000);
    }
    client.subscribe(MQTT_CTRL_TOPIC);
    client.loop();
}