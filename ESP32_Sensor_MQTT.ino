//Library
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
//Sensor Cahaya TSL2561
#include <Adafruit_TSL2561_U.h>
//Sensor Suhu & Kelembaban DHT11
#include <DHT.h>
#include <DHT_U.h>
//Sensor CO2 MH-Z19B                                                  
#include <MHZ19.h>
//MQTT & WiFi
#include <WiFi.h>
#include <PubSubClient.h>

//Macro
#define SDA                23
#define SCL                22
#define DHTPIN             4
#define DHTTYPE            DHT11
#define CO2PWM             32                                    
#define DELAY_TIME_DHT11   5000
#define DELAY_TIME_TSL2561 1000
#define DELAY_TIME_MHZ19B  15000

//WiFi Credential
const char* ssid = "IC-PROC";
const char* password = "0icprocPAUME0";
const char* mqttServer = "192.168.0.63";
const int mqttPort = 1883;

//Variabel Global
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
DHT_Unified dht(DHTPIN, DHTTYPE);
sensors_event_t event;
MHZ19 mhz(&Serial2);
float temp_measured = 0.0;
float hum_measured = 0.0;
float lux_measured = 0.0;
int CO2_measured = 0;
char tempString[20];
char humString[20];
char luxString[20];
char CO2String[20];
unsigned long lastMsgDHT11 = 0;
unsigned long lastMsgTSL2561 = 0;
unsigned long lastMsgSoil = 0;
unsigned long lastMsgMHZ19B = 0;

void setup() {
  //TSL2561
  Wire.begin(SDA, SCL);
  tsl.begin();
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);
  
  //DHT11
  dht.begin();

  //MH-Z19B
  Serial2.begin(9600);
  mhz.setRange(MHZ19_RANGE_2000);
  mhz.setAutoCalibration(true);
  
  //Pengaturan WiFi
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  }

  //Pengaturan MQTT
  client.setServer(mqttServer, mqttPort);

  Serial.begin(115200);
}

void loop() {
  long now;
  if (!client.connected()) {
  	while (!client.connected()) {
        client.connect("ESP32ClientSensor");
  	}
  }
  client.loop();
   
  now = millis();
  if (now - lastMsgDHT11 > DELAY_TIME_DHT11) {
    lastMsgDHT11 = now;
    
    //Baca suhu
    dht.temperature().getEvent(&event);
    if (!isnan(event.temperature)) {
      temp_measured = event.temperature;
      Serial.print("Suhu: ");
      Serial.println(temp_measured);
    }
    dtostrf(temp_measured, 1, 2, tempString);
    client.publish("esp32/temperature", tempString);

    //Baca kelembaban
    dht.humidity().getEvent(&event);
    if (!isnan(event.relative_humidity)) {
      hum_measured = event.relative_humidity;
      Serial.print("Kelembaban: ");
      Serial.println(hum_measured);
    }
    dtostrf(hum_measured, 1, 2, humString);
    client.publish("esp32/humidity", humString);
  }

  now = millis();
  if (now - lastMsgTSL2561 > DELAY_TIME_TSL2561) {
    lastMsgTSL2561 = now;
    
    //Baca intensitas cahaya
    tsl.getEvent(&event);
    if (event.light) {
      lux_measured = event.light;
      Serial.print("Cahaya: ");
      Serial.println(lux_measured);
    }
    dtostrf(lux_measured, 1, 2, luxString);
    client.publish("esp32/light_intensity", luxString);
  }

  now = millis();
  if (now - lastMsgMHZ19B > DELAY_TIME_MHZ19B)                                 
  {
    lastMsgMHZ19B = now;

    MHZ19_RESULT response = mhz.retrieveData();
    if (response == MHZ19_RESULT_OK)
    {
      CO2_measured = mhz.getCO2();
      Serial.print("CO2: ");
      Serial.println(CO2_measured);
    }
    dtostrf(CO2_measured, 1, 2, CO2String);
    client.publish("esp32/CO2", CO2String); 
  }
}
