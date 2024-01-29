#include<ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>

const char* ssid = "IOT";
const char* password = "Mwanje12";
int sensorPin = D3;
volatile long pulse;
unsigned long lastTime;
float volume;
File myFile;

void setup() {
  pinMode(sensorPin, INPUT);
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(sensorPin), increase, RISING);
  wifiSetup();
  delay(200);
  SDInnit();
  delay(200);
}

void loop() {
  volume = 2.663 * pulse / 1000 * 30;
  if (millis() - lastTime > 2000) {
    pulse = 0;
    lastTime = millis();
  }
  Serial.print(volume);
  Serial.println(" L/m");
  if (volume > 0){
    sdWrite(volume); 
  }
}

ICACHE_RAM_ATTR void increase() {
  pulse++;
}

void SDInnit(){
  if (!SD.begin(D2)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void sdWrite(float data){
  myFile = SD.open("log.txt", FILE_WRITE);

  if (myFile) {
    Serial.print("Writing to file...");
    myFile.println(data);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening log.txt");
  }
}

void wifiSetup(){
  Serial.begin(115200);              //SET THE BAUD RATE TO 115200 
  WiFi.mode(WIFI_STA);               //ESP works in two Wifi modes: as an individual station and as a soft access point. Here its as a station.
  WiFi.begin(ssid, password);        //a function that gets the ssid and password to connect to the router.
  Serial.print("Connecting to WiFi"); 
  
  //a check of internet connection
  while(WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(200);}

    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());    //method to print the IP Address
}

