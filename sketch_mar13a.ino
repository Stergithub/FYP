// #include<ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

SoftwareSerial nodeSerial(2,3); //2--->Tx, 3---->Rx

#define PH_POWER_PIN A0
#define TURBIDITY_POWER_PIN A1            

float calibration_value = 21.34 + 0.95;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;

float ph_act;

float volt;
float ntu; 
float ntu_1;

// const char* ssid = "LEGO";
// const char* password = "y3nz3artx";

void setup(){
  pinMode(PH_POWER_PIN, INPUT);
  pinMode(TURBIDITY_POWER_PIN, INPUT);
  Serial.begin(115200);
  Wire.begin();
  delay(20);
  
  nodeSerial.begin(115200);
}

void loop(){
   getPhSensorValue();
   delay(1000);
   getTurbiditySensorValue();
   delay(1000);
   String nodeMsg = "*";
   nodeMsg += String(ph_act,2) + ",";
   nodeMsg += String(ntu_1,2) + "#";
   nodeSerial.print(nodeMsg);
}

void getPhSensorValue(){
  for(int i=0;i<10;i++){ 
    buffer_arr[i]=analogRead(PH_POWER_PIN);
    delay(30);
  }
  for(int i=0;i<9;i++){
    for(int j=i+1;j<10;j++){
      if(buffer_arr[i]>buffer_arr[j]){
        temp=buffer_arr[i];
        buffer_arr[i]=buffer_arr[j];
        buffer_arr[j]=temp;
      }
    }
  }
  avgval=0;
  for(int i=2;i<8;i++){
    avgval+=buffer_arr[i];
    float volt=(float)avgval*5.0/1024/6; 
    ph_act = -5.70 * volt + calibration_value;
  }
  Serial.print("pH Val: ");
  Serial.println(ph_act);
}

void getTurbiditySensorValue(){
  int sensorValue;
  volt = 0;
  for (int i=0; i<800; i++)
  {
    sensorValue = analogRead(TURBIDITY_POWER_PIN);
    volt += ((float)sensorValue/1023)*5.0;// converting analog reading to volt
  }
  volt = volt/800;
  volt = round_to_dp(volt,2);
  if(volt < 2.5){
    ntu = 3000;
  }
  else{
    ntu = -1120.4 * square(volt)+5742.3*volt-4353.9;// converitng to ntu in the range of 0-3000
    ntu_1 = (ntu/3000)*5; // converitng to ntu in the range of 0-5
  }
  Serial.print("ntu: ");
  Serial.println(ntu_1);
}

float round_to_dp( float in_value, int decimal_place )// returns a specific number of decimal places
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

float square(float in_value){
  return pow(in_value, 2);
}

/*void connectToWiFi(){
  Serial.begin(115200);              //SET THE BAUD RATE TO 115200 
  WiFi.mode(WIFI_STA);               //ESP works in two Wifi modes: as an individual station and as a soft access point. Here its as a station.
  WiFi.begin(ssid, password);        //a function that gets the ssid and password to connect to the router.
  Serial.print("Connecting to WiFi"); 
  
  //a check of internet connection
  while(WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(200);
  }
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());   
}

int analogReadOne() {
    digitalWrite(PH_POWER_PIN, HIGH); //  Turn D1 On
    digitalWrite(TURBIDITY_POWER_PIN, LOW); // Turn D2 Off
    return analogRead(0);
}

int analogReadTwo() {
    digitalWrite(PH_POWER_PIN, LOW); //  Turn D1 On
    digitalWrite(TURBIDITY_POWER_PIN, HIGH); // Turn D2 Off
    return analogRead(0);
}*/