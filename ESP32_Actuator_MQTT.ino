//Library
#include <Arduino.h>                                                     
//MQTT & WiFi
#include <WiFi.h>
#include <PubSubClient.h>
#include <RBDdimmer.h>

//Macro
#define DELAY_TIME_MS       100
#define GROWLIGHT_PIN       33
#define GROWLIGHT_CHANNEL   0
#define FAN_IN_PIN          23
#define FAN_IN_CHANNEL      2
#define FAN_OUT_PIN         25
#define FAN_OUT_CHANNEL     1
#define PERIS_PUMP_PIN      32 
#define PERIS_PUMP_CHANNEL  3
#define SUB_PUMP_PIN        26
#define SUB_PUMP_CHANNEL    4
#define FREQ                1000
#define RES                 12
#define HEATERPWM           14 
#define HEATERZC            27

//WiFi Credential
const char* ssid = "IC-PROC";
const char* password = "0icprocPAUME0";
const char* mqttServer = "192.168.0.63";
const int mqttPort = 1883;

//Variabel Global
WiFiClient espClient;
PubSubClient client(espClient);
dimmerLamp dimmer(HEATERPWM, HEATERZC);
volatile int growlight_dutycycle = 0;
volatile int fan_in_dutycycle = 0;
volatile int fan_out_dutycycle = 0;
volatile int peris_pump_dutycycle = 0;
volatile int sub_pump_dutycycle = 0;
volatile int heater_dutycycle = 0;
volatile int fan_frequency = 0;

void setup() {
  //Konfigurasi pin output PWM
  ledcSetup(GROWLIGHT_CHANNEL, 3000, RES);
  ledcAttachPin(GROWLIGHT_PIN, GROWLIGHT_CHANNEL);
  ledcSetup(FAN_IN_CHANNEL, 500, RES);
  ledcAttachPin(FAN_IN_PIN, FAN_IN_CHANNEL);
  ledcSetup(FAN_OUT_CHANNEL, 500, RES);
  ledcAttachPin(FAN_OUT_PIN, FAN_OUT_CHANNEL);
  ledcSetup(PERIS_PUMP_CHANNEL, 3000, RES);
  ledcAttachPin(PERIS_PUMP_PIN, PERIS_PUMP_CHANNEL);
  ledcSetup(SUB_PUMP_CHANNEL, FREQ, RES);
  ledcAttachPin(SUB_PUMP_PIN, SUB_PUMP_CHANNEL);

  dimmer.begin(NORMAL_MODE, ON);

  //Pengaturan WiFi
  delay(100);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  }

  Serial.begin(115200);
  Serial.println("Connected.");

  //Pengaturan MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  int i;
  char temp[50];

  if (strcmp(topic,"esp32/growlight") == 0) {
    
    for (i = 0; i < length; i++) {
      temp[i] = (char) payload[i];
    }
    sscanf(temp, "%d", &growlight_dutycycle);
  
    if (growlight_dutycycle > 4095) growlight_dutycycle = 4095;
    else if (growlight_dutycycle < 0) growlight_dutycycle = 0;
      
  } else if (strcmp(topic,"esp32/fan_in") == 0) {
    
    for (i = 0; i < length; i++) {
      temp[i] = (char) payload[i];
    }
    sscanf(temp, "%d", &fan_in_dutycycle);
  
    if (fan_in_dutycycle > 4095) fan_in_dutycycle = 4095;
    else if (fan_in_dutycycle < 0) fan_in_dutycycle = 0;

  } else if (strcmp(topic,"esp32/fan_out") == 0) {

    for (i = 0; i < length; i++) {
      temp[i] = (char) payload[i];
    }
    sscanf(temp, "%d", &fan_out_dutycycle);
  
    if (fan_out_dutycycle > 4095) fan_out_dutycycle = 4095;
    else if (fan_out_dutycycle < 0) fan_out_dutycycle = 0;
    
  } else if (strcmp(topic,"esp32/peris_pump") == 0) {
    
    for (i = 0; i < length; i++) {
      temp[i] = (char) payload[i];
    }
    sscanf(temp, "%d", &peris_pump_dutycycle);
  
    if (peris_pump_dutycycle > 4095) peris_pump_dutycycle = 4095;
    else if (peris_pump_dutycycle < 0) peris_pump_dutycycle = 0;
    
  } else if (strcmp(topic,"esp32/sub_pump") == 0) {

    for (i = 0; i < length; i++) {
      temp[i] = (char) payload[i];
    }
    sscanf(temp, "%d", &sub_pump_dutycycle);
  
    if (sub_pump_dutycycle > 4095) sub_pump_dutycycle = 4095;
    else if (sub_pump_dutycycle < 0) sub_pump_dutycycle = 0;
    
  } else if (strcmp(topic,"esp32/heater") == 0) {

    for (i = 0; i < length; i++) {
      temp[i] = (char) payload[i];
    }
    sscanf(temp, "%d", &heater_dutycycle);
  
    if (heater_dutycycle > 100) heater_dutycycle = 100;
    else if (heater_dutycycle < 0) heater_dutycycle = 0;
    
  } else if (strcmp(topic,"esp32/fan_freq") == 0) {

    for (i = 0; i < length; i++) {
      temp[i] = (char) payload[i];
    }
    sscanf(temp, "%d", &fan_frequency);
  
    ledcSetup(FAN_IN_CHANNEL, fan_frequency, RES);
    ledcAttachPin(FAN_IN_PIN, FAN_IN_CHANNEL);
    ledcSetup(FAN_OUT_CHANNEL, fan_frequency, RES);
    ledcAttachPin(FAN_OUT_PIN, FAN_OUT_CHANNEL);
    
  }

}

void loop() {
  long now, last;
  
  if (!client.connected()) {
    while (!client.connected()) {
        client.connect("ESP32ClientActuator");
    }
    client.subscribe("esp32/growlight");
    client.subscribe("esp32/fan_in");
    client.subscribe("esp32/fan_out");
    client.subscribe("esp32/peris_pump");
    client.subscribe("esp32/sub_pump");
    client.subscribe("esp32/heater");
    client.subscribe("esp32/fan_freq");
  }
  client.loop();
   
  now = millis();
  if (now - last > DELAY_TIME_MS) {
    
    last = now;

    ledcWrite(GROWLIGHT_CHANNEL, growlight_dutycycle);
    ledcWrite(FAN_IN_CHANNEL, fan_in_dutycycle);
    ledcWrite(FAN_OUT_CHANNEL, fan_out_dutycycle);
    ledcWrite(PERIS_PUMP_CHANNEL, peris_pump_dutycycle);
    ledcWrite(SUB_PUMP_CHANNEL, sub_pump_dutycycle);
    dimmer.setPower(heater_dutycycle);
    
  }
}
