/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

*********************************************************************/

/*********** SCREEN *************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN //was 4, which is not advisable on ESPs
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/********* Networking **********/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> // JSON parsing

const char* ssid = "YOUR_SSID";
const char* pw = "YOUR_PASSWORD";

const char* CITY = "Tokyo";

const int SECOND = 1000;
const int HOUR = SECOND * 60 * 60;

void setup()   {                
  Serial.begin(115200); // was 9600

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64);
  
  WiFi.begin(ssid, pw);
  while(WiFi.status() != WL_CONNECTED) {
    delay(SECOND);
    displayText("Connecting...", 3, true);
  }
}

void loop() {
  
  if (WiFi.status() == WL_CONNECTED) { // check connection
    
    displayText("Connected", 2, true);

    const String baseURL = "http://api.openweathermap.org/data/2.5/weather";
    const String params = "?q=Tokyo&units=metric&APPID=YOUR_API_KEY";
    const String url = baseURL + params;
    
    HTTPClient http;
    http.begin(url);
    
    int httpCode = http.GET();

    // Checking returned code
    if (httpCode > 0) {

      displayText("HTTP OK", 2, true);
      delay(1000);

      /*********** PARSING *************/
          
      const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 400;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      
      JsonObject& root = jsonBuffer.parseObject(http.getString());

      http.end(); // Close connection
      
      JsonObject& weather0 = root["weather"][0];
      const char* weatherType = weather0["main"]; // "Clouds"
      const char* weatherDescription = weather0["description"]; // "scattered clouds"
              
      JsonObject& main = root["main"];
      float temp = main["temp"]; // 16.63
      int pressure = main["pressure"]; // 1016
      int humidity = main["humidity"]; // 77
      int tempMin = main["temp_min"]; // 15
      int tempMax = main["temp_max"]; // 19
      
      const char* name = root["name"]; // "Tokyo"

      /*********** PARSING END *************/

      String space = " ";
      String returnChar = "\n";
      String text1 = String(returnChar + name + space + weatherType + returnChar) ;

      String tempLabel = String("\nTemp ");
      String text2 = String(tempLabel + temp + returnChar);
     
      String pressureLabel = String("\nPressure\n\n");
      String text3 = String(pressureLabel + pressure + returnChar);

      String humidityLabel = String("\nHumidity\n\n");
      String text4 = String(humidityLabel + humidity + returnChar);

      String lowLabel = String("\nLow ");
      String text5 = String(lowLabel + tempMin + returnChar);

      String highLabel = String("\nHigh ");
      String text6 = String(highLabel + tempMax + returnChar);

      String messages[] = {text1, text2, text3, text4, text5, text6};

      int sizeOfMessages = sizeof(messages) / sizeof(messages[0]); // Only works when arrays are NOT pointers

      for (int i = 0; i < HOUR; i++) {
        for (int z = 0; i < sizeOfMessages; z++) {
          displayText(messages[z], 2, true);
          delay(SECOND * 10);  
        }
      }      
    }
    http.end();
    
  } else {
    displayText("HTTP not OK", 2, false);
  }

}

void displayText(String text, int textSize, bool willScroll) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(textSize);
  display.println(text);
  display.display();

  setScroll(willScroll);
}

void setScroll(bool willScroll) {
    if (willScroll == true) {
    display.startscrollleft(0x00, 0x0F);
  } else {
    display.stopscroll();
  }
}

