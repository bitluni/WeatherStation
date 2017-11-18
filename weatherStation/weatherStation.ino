 #include <Wire.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Adafruit_BMP085.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//comment the next line in if you like to publish voltage instead of light. It's an command line option.
//#define PUBLISH_VOLTAGE

#ifdef PUBLISH_VOLTAGE
ADC_MODE(ADC_VCC);
#endif

#define WLAN_SSID   "WLANNAME"
#define WLAN_PASS   "PASSWORD"

//MQTT broker settings
#define HOST        "192.168.0.10"
#define PORT        1883
#define USERNAME    "YOUR BROKER USERNAME"
#define PASSWORD    "YOUR BROKER PASSWORD"
#define DHTPIN 14 //D5 
DHT dht(DHTPIN, DHT22);

//milliseconds to sleep
const int intervall = 300000;
//time out loop count
const int timeout = 200;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, HOST, PORT, USERNAME, PASSWORD);
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, "weatherStation/temperature");
Adafruit_MQTT_Publish pressure = Adafruit_MQTT_Publish(&mqtt, "weatherStation/pressure");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, "weatherStation/humidity");
Adafruit_MQTT_Publish light = Adafruit_MQTT_Publish(&mqtt, "weatherStation/light");
Adafruit_MQTT_Publish voltage = Adafruit_MQTT_Publish(&mqtt, "weatherStation/voltage");
void MQTT_connect();

Adafruit_BMP085 bmp;

void setup() {
  WiFi.forceSleepWake();
  delay(1);
  WiFi.mode(WIFI_STA);  

  //Serial.begin(115200);
  //Serial.print("Connecting to ");
  //Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  int i = 0;
  for (; i < timeout; i++)
  {
    if(WiFi.status() == WL_CONNECTED) break;
    delay(100);
    //Serial.print(".");
  }
  if(i == timeout)
    deepSleep();

  //Serial.println("IP address: "); Serial.println(WiFi.localIP());

  bmp.begin();
  dht.begin();

  MQTT_connect();
  delay(100);
  temperature.publish(bmp.readTemperature());
  pressure.publish(bmp.readPressure());
  humidity.publish(dht.readHumidity());
#ifdef PUBLISH_VOLTAGE
  voltage.publish(ESP.getVcc());
#else
  light.publish(analogRead(A0) / 1023.0f);
#endif
  
  //Serial.println("deep sleep");
  deepSleep();
}

void deepSleep()
{
  //https://github.com/esp8266/Arduino/issues/644
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  ESP.deepSleep(intervall * 1000/*, WAKE_RF_DISABLED*/);
}

void loop() 
{
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 10;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       //Serial.println(mqtt.connectErrorString(ret));
       //Serial.println("Retrying MQTT connection in 1 second...");
       mqtt.disconnect();
       delay(1000);
       retries--;
       if (retries == 0)
         deepSleep();
  }
  //Serial.println("MQTT Connected!");
}
