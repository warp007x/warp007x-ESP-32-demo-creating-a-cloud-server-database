//Include libraries
#include <HTTPClient.h>               //Download: https://electronoobs.com/eng_arduino_httpclient.php
#include <WiFi.h>                     //Download: https://electronoobs.com/eng_arduino_wifi.php


//Add WIFI data
const char* ssid = "ZZzzz(-.-)";              //Add your WIFI network name 
const char* password =  "antonov@3600";           //Add WIFI password


//Variables used in the code
String LED_id = "1";                  //Just in case you control more than 1 LED
bool toggle_pressed = false;          //Each time we press the push button    
String data_to_send = "";             //Text data to send to the server
unsigned int Actual_Millis, Previous_Millis;
int refresh_time = 200;               //Refresh rate of connection to website (recommended more than 1s)


//Inputs/outputs
int button1 = 13;                     //Connect push button on this pin
int LED = 2;                          //Connect LED on this pin (add 150ohm resistor)


//Button press interruption
void IRAM_ATTR isr() {
  toggle_pressed = true; 
}

void setup() {
  delay(10);
  Serial.begin(115200);                   //Start monitor
  pinMode(LED, OUTPUT);                   //Set pin 2 as OUTPUT
  pinMode(button1, INPUT_PULLDOWN);       //Set pin 13 as INPUT with pulldown
  attachInterrupt(button1, isr, RISING);  //Create interruption on pin 13

  WiFi.begin(ssid, password);             //Start wifi connection
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected, my IP: ");
  Serial.println(WiFi.localIP());
  Actual_Millis = millis();               //Save time for refresh loop
  Previous_Millis = Actual_Millis; 
}


void loop() {  
  //We make the refresh loop using millis() so we don't have to sue delay();
  Actual_Millis = millis();
  if(Actual_Millis - Previous_Millis > refresh_time){
    Previous_Millis = Actual_Millis;  
    if(WiFi.status()== WL_CONNECTED){                   //Check WiFi connection status  
      HTTPClient http;                                  //Create new client
      if(toggle_pressed){                               //If button was pressed we send text: "toggle_LED"
        data_to_send = "toggle_LED=" + LED_id;  
        toggle_pressed = false;                         //Also equal this variable back to false 
      }
      else{
        data_to_send = "check_LED_status=" + LED_id;    //If button wasn't pressed we send text: "check_LED_status"
      }
      
      //Begin new connection to website       
      http.begin("https://warp-07x.000webhostapp.com/esp32_update.php");   //Indicate the destination webpage 
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");         //Prepare the header
      
      int response_code = http.POST(data_to_send);                                //Send the POST. This will giveg us a response code
      
      //If the code is higher than 0, it means we received a response
      if(response_code > 0){
        Serial.println("HTTP code " + String(response_code));                     //Print return code
  
        if(response_code == 200){                                                 //If code is 200, we received a good response and we can read the echo data
          String response_body = http.getString();                                //Save the data comming from the website
          Serial.print("Server reply: ");                                         //Print data to the monitor for debug
          Serial.println(response_body);

          //If the received data is LED_is_off, we set LOW the LED pin
          if(response_body == "LED_is_off"){
            digitalWrite(LED, LOW);
          }
          //If the received data is LED_is_on, we set HIGH the LED pin
          else if(response_body == "LED_is_on"){
            digitalWrite(LED, HIGH);
          }  
        }//End of response_code = 200
      }//END of response_code > 0
      
      else{
       Serial.print("Error sending POST, code: ");
       Serial.println(response_code);
      }
      http.end();                                                                 //End the connection
    }//END of WIFI connected
    else{
      Serial.println("WIFI connection error");
    }
  }
}
