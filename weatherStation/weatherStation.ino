#include <Wire.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Adafruit_BMP085.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


#define WLAN_SSID       "............"
#define WLAN_PASS       "............"

#define HOST        "192.168.0.10"
#define PORT        1883
#define USERNAME    "..........."
#define PASSWORD    "..........."
#define DHTPIN 2 
DHT dht(DHTPIN, DHT11);

const int intervall = 10000;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, HOST, PORT, USERNAME, PASSWORD);
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, "weatherStation/temperature");
Adafruit_MQTT_Publish pressure = Adafruit_MQTT_Publish(&mqtt, "weatherStation/pressure");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, "weatherStation/humidity");
Adafruit_MQTT_Publish light = Adafruit_MQTT_Publish(&mqtt, "weatherStation/light");
void MQTT_connect();

Adafruit_BMP085 bmp;

void setup() {
  WiFi.forceSleepWake();
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  if(!bmp.begin()) Serial.println("BMP not detected");
  dht.begin();

  MQTT_connect();
  delay(200);
  temperature.publish(bmp.readTemperature());
  pressure.publish(bmp.readPressure());
  humidity.publish(dht.readHumidity());
  light.publish(analogRead(A0) / 1023.0f);
  
  Serial.println("deep sleep");
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
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
