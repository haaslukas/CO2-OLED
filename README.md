# Indoor CO2 concentration reading
###### Components
* SDC30 CO2 Sensor
* TTGO T-Display (ESP32 microcontroller)

###### Visualisation
CO2 [ppm], temperature [°C) and humidity [%] are read out every 5 seconds and shown on the OLED Display.
- For ppm values below 1000, the display shows a **green** status (*everything ok*).
- For ppm values between 1000 and 1500, the display shows a **orange** status (*you should let in some fresh air*).
- For ppm values above 1500, the display shows a **re**d status (*max. indoor reference value exceeded; this can lead to dizziness and drowsiness*).

###### Credentials
The ESP32 connects to your local Wifi and publishes the values to a MQTT broker (broker IP can be set in the main.cpp). WiFi and MQTT credentials are defined in the Credentials.h library, which you won't find in the repo (see .gitignore). Just add the Credentials folder and the Credentials.h file to your lib folder and add the following lines:
```
#define WLAN_SSID       "your-ssid"
#define WLAN_PASS       "your-password"
#define MQTT_USER       "your-username"
#define MQTT_PW         "your-pw"
```
Happy windows opening ;-)
