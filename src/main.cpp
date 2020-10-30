#include <Arduino.h>

//#include <WiFi.h>
//#include <WiFiClientSecure.h>

#include <Wire.h>
#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library
#include <SparkFun_SCD30_Arduino_Library.h>

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

#define I2C_SDA             21    
#define I2C_SCL             22

//TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

SCD30 airSensor;

void setup() {
  Serial.begin(115200);

  /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
  */
  //pinMode(ADC_EN, OUTPUT);
  //digitalWrite(ADC_EN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  
  Wire.begin(I2C_SDA, I2C_SCL); //Start the wire hardware that may be supported by your platform

  if (airSensor.begin(Wire) == false) //Pass the Wire port to the .begin() function
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1);
  }

}

void loop() {
  
  if (airSensor.dataAvailable())
  {
    int co2_value = airSensor.getCO2();
    float temp_value = airSensor.getTemperature();
    float humi_value = airSensor.getHumidity();

    Serial.print("co2(ppm):");
    Serial.print(co2_value);

    Serial.print(" temp(C):");
    Serial.print(temp_value, 1);

    Serial.print(" humidity(%):");
    Serial.print(humi_value, 1);

    Serial.println();

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