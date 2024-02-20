#include <RTClib.h>
#include<ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>

RTC_DS1307 rtc;
char Week_days[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const char* ssid = "Ster";
const char* password = "Ster0756";
int sensorPin = D3;
volatile long pulse;
unsigned long lastTime;
float volume;
float consumption;
File myFile;

void setup() {

  Serial.begin(115200);
  Serial.print("yeah");
  if (!rtc.begin()) {
//    #ifndef ESP8266
    while (!Serial); // wait for serial port to connect. Needed for native USB
    Serial.println("\nCouldn't find RTC");
//    #endif
    
    while(1) yield();
  }
  else{
    Serial.println("\n RTC found\n");
  }
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
 
  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), increase, RISING);
  
  wifiSetup();
  delay(200);
  SDInnit();
  delay(200);
}

void loop() {

  consumption = 2.663 * pulse;
  
  volume = 2.663 * pulse / 1000 * 30;
  if (millis() - lastTime > 2000) {
    pulse = 0;
    lastTime = millis();
  }
  Serial.print(volume);
  Serial.println(" L/m");
  if (volume > 0){
    sdWrite();
  }
  else{
    Serial.println("No value to log");
  }

  delay(1000);
}

ICACHE_RAM_ATTR void increase() {
  pulse++;
}


void SDInnit(){
  if (!SD.begin(D0)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void sdWrite(){
  DateTime now = rtc.now();
  myFile = SD.open("log.csv", FILE_WRITE);

  if (myFile) {
    Serial.print("Writing to file...");

    print2Digits(now.year());
    myFile.print('-');
    print2Digits(now.month());
    myFile.print('-');
    print2Digits(now.day());
    myFile.print("T");
    print2Digits(now.hour());
    myFile.print(':');
    print2Digits(now.minute());
    myFile.print(':');
    print2Digits(now.second());
    myFile.print("Z");
    myFile.print(',');
    myFile.println(consumption);
    myFile.println("mL");
    myFile.print(',');
    myFile.println(volume);
    myFile.println("mL/s");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening log.csv");
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

void print2Digits(int num)
{
  if (num<10){
    myFile.print('0');
  }
  myFile.print(num);
}
