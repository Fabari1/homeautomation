//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

 
#include "Arduino.h"
// IMPORT ALL REQUIRED LIBRARIES
#include <rom/rtc.h>



//IMPORT IMAGES
#include "lockclose.h"
#include "lockopen.h"


#ifndef _WIFI_H 
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
// DEFINE VARIABLES
#define BUTTON1  32
#define BUTTON2  33
#define BUTTON3  25

#define analogPin A3


#define TFT_DC    17
#define TFT_CS    5
#define TFT_RST   16
#define TFT_CLK   18
#define TFT_MOSI  23
#define TFT_MISO  19

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);


// IMPORT FONTS FOR TFT DISPLAY
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h> 

 


// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620154701";                    // Add your ID number here
static const char* subtopic[]    = {"620154701_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "www.yanacreations.com";         // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;

// WIFI CREDENTIALS
const char* ssid       = "MonaConnect"; // Add your Wi-Fi ssid
const char* password   = ""; // Add your Wi-Fi password 



// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL; 


// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters ); 

void digit1(uint8_t number);
void digit2(uint8_t number);
void digit3(uint8_t number);
void digit4(uint8_t number);

void checkPasscode(void);
void showLockState(void);

 

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif


/* Initialize class objects*/
uint8_t currentDigit = 1; // Keeps track of the current digit being modified by the potentiometer
bool lockState = false; // keeps track of the Open and Close state of the lock

 uint8_t digit1Value = 0, digit2Value = 0, digit3Value = 0, digit4Value = 0; // Keeps track of the value of each digit in the 4-digit passcode. 
 
 
/* Declare your functions below */



void setup() {
    Serial.begin(115200);  // INIT SERIAL  
    tft.begin();
    tft.fillScreen(ILI9341_WHITE);
    
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
  
    
  // CONFIGURE THE ARDUINO PINS OF THE 7SEG AS OUTPUT
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);

  digit1(0);
  digit2(0);
  digit3(0);
  digit4(0);
  /* Configure all others here */

  initialize();           // INIT WIFI, MQTT & NTP 
  vButtonCheckFunction(); // UNCOMMENT IF USING BUTTONS THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION

}
  
uint16_t value = 0;
int mappedValue = 0;

void loop() {
  // put your main code here, to run repeatedly: 
  value=analogRead(analogPin);
  mappedValue = map(value, 0, 4095, 0, 9);
  Serial.println(mappedValue);
  Serial.println(value);

  if(currentDigit == 1){
    digit1Value=mappedValue;
    digit1(mappedValue);
  }
  else if(currentDigit == 2){
    digit2Value=mappedValue;
    digit2(mappedValue);
  }
  else if(currentDigit == 3){
    digit3Value=mappedValue;
    digit3(mappedValue);
  }
  else if(currentDigit == 4){
    digit4Value=mappedValue;
    digit4(mappedValue);
  }
  
 

  vTaskDelay(1000 / portTICK_PERIOD_MS);  
}



  
//####################################################################
//#                          UTIL FUNCTIONS                          #       
//####################################################################
void vButtonCheck( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );     
      
    for( ;; ) {
        // Add code here to check if a button(S) is pressed
        // then execute appropriate function if a button is pressed  

        // 1. Implement button1  functionality
        if (digitalRead(BUTTON1) == LOW)
        {
          /* code */
          currentDigit++;
          if (currentDigit > 4)
          {
            currentDigit = 1;
          }
        }
      
        // 2. Implement button2  functionality

        if (digitalRead(BUTTON2) == LOW)
        {
          /* code */
          checkPasscode();
        }

        // 3. Implement button3  functionality

        if (digitalRead(BUTTON3) == LOW)
        {
          /* code */
          lockState = false;
          showLockState();
        }
       
        vTaskDelay(200 / portTICK_PERIOD_MS);  
    }
}

void vUpdate( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );    
 
    for( ;; ) {
          // Task code goes here.   
          // PUBLISH to topic every second.  
            
        vTaskDelay(1000 / portTICK_PERIOD_MS);  
    }
}

unsigned long getTimeStamp(void) {
          // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
          time_t now;         
          time(&now); // Retrieve time[Timestamp] from system and save to &now variable
          return now;
}

void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  for (int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  StaticJsonDocument<1200> doc;
  DeserializationError error = deserializeJson(doc, received);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }




  // PROCESS MESSAGE
  const char* type= doc["type"];

  if (strcmp(type, "password")==0){

    const char* code= doc["code"];
  }

  

}

bool publish(const char *topic, const char *payload){   
     bool res = false;
     try{
        res = mqtt.publish(topic,payload);
        // Serial.printf("\nres : %d\n",res);
        if(!res){
          res = false;
          throw false;
        }
     }
     catch(...){
      Serial.printf("\nError (%d) >> Unable to publish message\n", res);
     }
  return res;
}

//***** Complete the util functions below ******
  
void digit1(uint8_t number){
  // CREATE BOX AND WRITE NUMBER IN THE BOX FOR THE FIRST DIGIT
  // 1.Set font to FreeSansBold18pt7b
  tft.setFont(&FreeSansBold18pt7b);
  // 2. Draw a filled rounded rectangle close to the bottom of the screen. Give it any colour you like 
  int rectX = 5; // X-coordinate of the rectangle
  int rectY = 260; // Y-coordinate of the rectangle
  int rectWidth = 50; // Width of the rectangle
  int rectHeight = 50; // Height of the rectangle
  int cornerRadius = 5; // Radius of the rounded corners
  int fillColor = ILI9341_RED; // Color of the filled rectangle

  tft.fillRoundRect(rectX, rectY, rectWidth, rectHeight, cornerRadius, fillColor);
  // 3. Set cursor to the appropriate coordinates in order to write the number in the middle of the box 
 tft.setCursor(20, 295);
  // 4. Set the text colour of the number. Use any colour you like 
  tft.setTextColor(ILI9341_WHITE);
  // 5. Set font size to one 
  tft.setTextSize(1);
  // 6. Print number to the screen
  tft.print(number);
}
 
void digit2(uint8_t number){
  // CREATE BOX AND WRITE NUMBER IN THE BOX FOR THE SECOND DIGIT
  // 1. Set font to FreeSansBold18pt7b 
  tft.setFont(&FreeSansBold18pt7b);
  // 2. Draw a filled rounded rectangle close to the bottom of the screen. Give it any colour you like
  int rectX = 65; // X-coordinate of the rectangle
   int rectY = 260; // Y-coordinate of the rectangle
  int rectWidth = 50; // Width of the rectangle
  int rectHeight = 50; // Height of the rectangle
  int cornerRadius = 5; // Radius of the rounded corners
  int fillColor = ILI9341_RED; // Color of the filled rectangle 

  tft.fillRoundRect(rectX, rectY, rectWidth, rectHeight, cornerRadius, fillColor);
  // 3. Set cursor to the appropriate coordinates in order to write the number in the middle of the box 
 

  tft.setCursor(80, 295);
  // 4. Set the text colour of the number. Use any colour you like 
  tft.setTextColor(ILI9341_WHITE);
  // 5. Set font size to one
  tft.setTextSize(1);
  // 6. Print number to the screen 
  tft.print(number);
}

void digit3(uint8_t number){
   // CREATE BOX AND WRITE NUMBER IN THE BOX FOR THE SECOND DIGIT
  // 1. Set font to FreeSansBold18pt7b 
  tft.setFont(&FreeSansBold18pt7b);
  // 2. Draw a filled rounded rectangle close to the bottom of the screen. Give it any colour you like
  int rectX = 125; // X-coordinate of the rectangle
   int rectY = 260; // Y-coordinate of the rectangle
  int rectWidth = 50; // Width of the rectangle
  int rectHeight = 50; // Height of the rectangle
  int cornerRadius = 5; // Radius of the rounded corners
  int fillColor = ILI9341_RED; // Color of the filled rectangle

  tft.fillRoundRect(rectX, rectY, rectWidth, rectHeight, cornerRadius, fillColor);
  // 3. Set cursor to the appropriate coordinates in order to write the number in the middle of the box 
 tft.setCursor(140, 295);
  // 4. Set the text colour of the number. Use any colour you like 
  tft.setTextColor(ILI9341_WHITE);
  // 5. Set font size to one
  tft.setTextSize(1);
  // 6. Print number to the screen 
  tft.print(number);
}

void digit4(uint8_t number){
  // CREATE BOX AND WRITE NUMBER IN THE BOX FOR THE SECOND DIGIT
  // 1. Set font to FreeSa9341nsBold18pt7b 
  tft.setFont(&FreeSansBold18pt7b);
  // 2. Draw a filled rounded rectangle close to the bottom of the screen. Give it any colour you like
  int rectX = 185; // X-coordinate of the rectangle
 int rectY = 260; // Y-coordinate of the rectangle
  int rectWidth = 50; // Width of the rectangle
  int rectHeight = 50; // Height of the rectangle
  int cornerRadius = 5; // Radius of the rounded corners
  int fillColor =ILI9341_RED;
  tft.fillRoundRect(rectX, rectY, rectWidth, rectHeight, cornerRadius, fillColor);
  // 3. Set cursor to the appropriate coordinates in order to write the number in the middle of the box 
  tft.setCursor(200, 295);
  // 4. Set the text colour of the number. Use any colour you like 
  tft.setTextColor(ILI9341_WHITE);
  // 5. Set font size to one
  tft.setTextSize(1);
  // 6. Print number to the screen 
  tft.print(number);
}
 
 
void checkPasscode(void){
    // THE APPROPRIATE ROUTE IN THE BACKEND COMPONENT MUST BE CREATED BEFORE THIS FUNCTION CAN WORK
    WiFiClient client;
    HTTPClient http;

    if(WiFi.status()== WL_CONNECTED){ 
      
      // 1. REPLACE LOCALHOST IN THE STRING BELOW WITH THE IP ADDRESS OF THE COMPUTER THAT YOUR BACKEND IS RUNNING ON
      http.begin(client, "http://:8080/api/check/combination"); // Your Domain name with URL path or IP address with path 
 
      
      http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header      
      char message[20];  // Store the 4 digit passcode that will be sent to the backend for validation via HTTP POST
      
      //2. Insert all four (4) digits of the passcode into a string with 'passcode=1234' format
      sprintf(message, "passcode=%d%d%d%d", digit1Value, digit2Value, digit3Value, digit4Value);
       
                      
      int httpResponseCode = http.POST(message);  // Send HTTP POST request and then wait for a response

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String received = http.getString();
       
        // 3. CONVERT 'received' TO JSON. 
        StaticJsonDocument<1200> doc;
        DeserializationError error = deserializeJson(doc, received);

        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return;
        }


        

        // 4. PROCESS MESSAGE. The response from the route that is used to validate the passcode
        // will be either {"status":"complete","data":"complete"}  or {"status":"failed","data":"failed"} schema.
        // (1) if the status is complete, set the lockState variable to true, then invoke the showLockState function
        // (2) otherwise, set the lockState variable to false, then invoke the showLockState function

        if (doc["status"] == "complete") {
          lockState = true;
          showLockState();
        } else {
          lockState = false;
          showLockState();
        }
              
      }     
        
      // Free resources
      http.end();

    }
             
 }



void showLockState(void){
  
    // Toggles the open and close lock images on the screen based on the lockState variable  
    tft.setFont(&FreeSansBold9pt7b);  
    tft.setTextSize(1);
    

    if(lockState == true){
      tft.drawRGBBitmap(68,10, lockopen, 104, 97); 
      tft.setCursor(50, 200);  
      tft.setTextColor(ILI9341_WHITE); 
      tft.printf("Access Denied"); 
      tft.setCursor(50, 200);  
      tft.setTextColor(ILI9341_GREEN); 
      tft.printf("Access Granted");
      
    }
    else {
      tft.drawRGBBitmap(68,10, lockclose, 104, 103); 
      tft.setCursor(50, 200);  
      tft.setTextColor(ILI9341_WHITE); 
      tft.printf("Access Granted"); 
      tft.setCursor(50, 200);  
      tft.setTextColor(ILI9341_RED); 
      tft.printf("Access Denied"); 
    }
    
}
 