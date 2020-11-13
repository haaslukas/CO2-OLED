#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library
#include <SparkFun_SCD30_Arduino_Library.h>

#define my_Serial

//#include "Free_Fonts.h" // Include the header file attached to this sketch

// TFT Pins has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
// #define TFT_MOSI            19
// #define TFT_SCLK            18
// #define TFT_CS              5
// #define TFT_DC              16
// #define TFT_RST             23
// #define TFT_BL              4   // Display backlight control pin

/*
#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_1            35
#define BUTTON_2            0
*/

#include <Credentials.h>


/************************* WiFi Access Point *********************************/

//#define WLAN_SSID       // create Credentials.h File in lib Folder Credentials
//#define WLAN_PASS       // create Credentials.h File in lib Folder Credentials

/************************* MQTT Setup *********************************/

#define MQTT_SERVER     "192.168.1.20"
#define MQTT_PORT       1883
//#define MQTT_USER     // create Credentials.h File in lib Folder Credentials
//#define MQTT_PW       // create Credentials.h File in lib Folder Credentials

#define HOSTNAME        "co2-oled-demo"

#define SUB_TOPIC       "co2-sensor/status"             // messages: request
#define PUB_TOPIC       "co2-sensor/status/response"    // messages: online

#define PUB_TOPIC_CO2   "co2-sensor/values"    // message: {}

#define WILL_TOPIC      "co2-sensor/status/lastwill"
#define WILL_QOS        0
#define WILL_RETAIN     0
#define WILL_MESSAGE    "connection lost"


#define I2C_SDA             21    
#define I2C_SCL             22

//TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

SCD30 airSensor;

/************ Global State (you don't need to change this!) ******************/
// Create an ESP WiFiClient class to connect to the MQTT server.
WiFiClient espClient;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
PubSubClient client(espClient);


// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    #ifdef my_Serial  
      Serial.print("Attempting MQTT connection...");
      
    #endif

    // Create a random client ID
    String clientId = HOSTNAME + '-';
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PW, WILL_TOPIC, WILL_QOS, WILL_RETAIN, WILL_MESSAGE)) {
      #ifdef my_Serial  
        Serial.println("connected");
      #endif
      // Once connected, publish an announcement...
      client.publish(PUB_TOPIC, "online");
      //client.publish(PUB_TOPIC, String(ESP.getChipId()).c_str());
      client.subscribe(SUB_TOPIC);
    } else {
      #ifdef my_Serial  
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      #endif
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  #ifdef my_Serial  
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
  #endif
  String message = "";
  for (int i=0;i<length;i++) {
    message += (char)payload[i];
  }
  #ifdef my_Serial  
    Serial.print(message);
    Serial.println();
  #endif

  if (strcmp(topic, SUB_TOPIC)==0){
    if (message == "request") {
      #ifdef my_Serial  
        Serial.println("requesting state");
      #endif
      client.publish(PUB_TOPIC, "online");
    }
  }
}

void setup() {
  Serial.begin(115200);

   // Connect to WiFi access point.
  #ifdef my_Serial
    Serial.println(); Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);
  #endif

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef my_Serial
      Serial.print(".");
    #endif
  }

  #ifdef my_Serial
    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address: "); Serial.println(WiFi.localIP());
  #endif
  
  WiFi.mode(WIFI_STA);

  // set MQTT Server:
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);   

  /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
  */
  //pinMode(ADC_EN, OUTPUT);
  //digitalWrite(ADC_EN, HIGH);

  #ifdef my_Serial
    Serial.println("Init TFT");
  #endif

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_VIOLET);

  
  Wire.begin(I2C_SDA, I2C_SCL); //Start the wire hardware that may be supported by your platform

  if (airSensor.begin(Wire) == false) //Pass the Wire port to the .begin() function
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    //tft.drawString("SCD30 Sensor nicht erkannt", 20, 20, 4);
    while (1);
  }

}

void loop() {

  // Ensure the WiFI connection is alive
  if (WiFi.status() != WL_CONNECTED) { WiFi.begin(WLAN_SSID, WLAN_PASS); }
  
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  
  
  if (airSensor.dataAvailable())
  {
    int co2_value = airSensor.getCO2();
    float temp_value = airSensor.getTemperature();
    float humi_value = airSensor.getHumidity();

    DynamicJsonDocument JSONdoc(100);

    JSONdoc["co2"] = co2_value;
    JSONdoc["temp"] = temp_value;
    JSONdoc["humi"] = humi_value;

    // Generate the minified JSON and send it to the Serial port.
    #ifdef my_Serial
      serializeJson(JSONdoc, Serial);
      Serial.println();
    #endif

    // Publish the JSON via MQTT
    char buffer[256];
    size_t n = serializeJson(JSONdoc, buffer);
    client.publish(PUB_TOPIC_CO2, buffer, n);

    #ifdef my_Serial
      Serial.print("co2(ppm):");
      Serial.print(co2_value);

      Serial.print(" temp(C):");
      Serial.print(temp_value, 1);

      Serial.print(" humidity(%):");
      Serial.print(humi_value, 1);

      Serial.println();
    #endif

    if (co2_value < 700) {
      tft.fillScreen(TFT_GREEN);
       
    } else if (co2_value < 1000) {
      tft.fillScreen(TFT_ORANGE);
       
    } else {
      tft.fillScreen(TFT_RED);
    }

    // Find centre of screen
    uint16_t x = tft.width()/2;
    uint16_t y = tft.height()/2;

    tft.setRotation(1);
    tft.setTextColor(TFT_WHITE);
    //tft.setCursor(20, 50);

    int padding = 25;
    //tft.setTextDatum(ML_DATUM); // Top Left
    //tft.drawString("CO2: ", 5, y-padding, 1);
    tft.setTextDatum(MR_DATUM); // Set datum to Middle Right
    tft.drawNumber(co2_value, x, y-padding, 4);
    tft.setTextDatum(ML_DATUM); // Set datum to Middle Left
    tft.drawString(" ppm", x, y-padding, 4);

    
    //tft.setTextDatum(ML_DATUM); // Top Left
    //tft.drawString("Temperatur: ", 5, y, 1);
    tft.setTextDatum(MR_DATUM); // Set datum to Middle Right
    tft.drawFloat(temp_value, 2, x, y, 4);
    tft.setTextDatum(ML_DATUM); // Set datum to Middle Left
    tft.drawString(" ` C", x, y, 4);

    
    //tft.setTextDatum(ML_DATUM); // Top Left
    //tft.drawString("Feuchtigkeit: ", 5, y+padding, 1);
    tft.setTextDatum(MR_DATUM); // Set datum to Middle Right
    tft.drawFloat(humi_value, 2, x, y+padding, 4);
    tft.setTextDatum(ML_DATUM); // Set datum to Middle Left
    tft.drawString(" %", x, y+padding, 4);

    //tft.setFreeFont(sFF14);     // Select the orginal small TomThumb font
  }

  delay(5000);
}