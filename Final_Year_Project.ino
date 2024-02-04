#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RF24.h>

// gsm variables
#define RX 7
#define TX 6
SoftwareSerial gsm(RX,TX);// 6,7

// Variables for timing sending values to thingspeak
long writeTimingSeconds = 20; // ==> Define Sample time in seconds to send data
long startWriteTiming = 0;
long elapsedWriteTime = 0;

// Thingspeak variables
String statusChWriteKey = "WYCYZTPB8OF6HZ3T";  // Status Channel id: 385184

// Thingspeak Connection Reset variables
int spare = 0;
boolean error;

// turbidity sensor variables
int sensorPin = A0;                
float volt;
float ntu; 
float ntu_1;

// ph sensor variables
float calibration_value = 21.34 - 0.4;
int ph_pin = A1;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;
float ph_act;

//flow sensor
float vol;

//LCD variables
LiquidCrystal_I2C lcd(0x27,16,2);

// RF24 variables
byte node_A_address[6] = "NodeA";
byte node_B_address[6] = "NodeB";

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
RF24 radio(10,9);// 9,8
/**********************************************************/

void setup() {
  Serial.begin(9600);
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(node_A_address);
  radio.openReadingPipe(1, node_B_address);

  // Start the radio listening for data
  radio.startListening();
  gsm.begin(9600);
  init_gsm();
  Wire.begin();
  startWriteTiming = millis(); // checking the number of ms passed since the program started running
  lcd_init();
}

void loop() {
  water_quality_display();
}

void writeThingSpeak(void)// writing to thingspeak
{
  startThingSpeakCmd();

  // preparacao da string GET
  String getStr = "GET /update?api_key=";
  getStr += statusChWriteKey;
  getStr += "&field1=";
  getStr += String(ntu_1);
  getStr += "&field2=";
  getStr += String(ph_act);
  getStr += "&field3=";
  getStr += String(vol);
  getStr += "\r\n\r\n";

  sendThingSpeakGetCmd(getStr);
  
}

void startThingSpeakCmd(void)
{
  // initializing the connection to internet through data connection 
  gsm.flush();//ensures characters are sent 
  // starting TCP/IP connection to thingspeak
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // This is the IP for api.thingspeak.com
  cmd += "\",80";
  gsm.println(cmd);
  Serial.print("Sent ==> Start cmd: ");
  Serial.println(cmd);
  ShowSerialData(); 
}

String sendThingSpeakGetCmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  gsm.println(cmd);
  Serial.print("Sent ==> lenght cmd: ");
  Serial.println(cmd);

  if (gsm.find((char *)">"))
  {
    gsm.print(getStr);
    Serial.print("enviado ==> getStr: ");
    Serial.println(getStr);
    delay(500);

    String messageBody = "";
    while (gsm.available())
    {
      String line = gsm.readStringUntil('\n');
      if (line.length() == 1)
      { //actual content starts after empty line (that has length 1)
        messageBody = gsm.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    gsm.println("AT+CIPSHUT");// close the connection
    Serial.println("GSM CIPSEND ERROR: RESENDING"); //Resend...
    spare = spare + 1;
    error = 1;
    return "error";
  }
}

void ShowSerialData()// viewing communication to the gsm
{
  while(gsm.available()!=0)
  Serial.write(gsm.read());
  delay(1000); 
}

// turbidity sensor
void turbidity() {
  volt = 0;
  for (int i=0; i<800; i++)
  {
    volt += ((float)analogRead(sensorPin)/1023)*5;// converting analog reading to volt
  }
  volt = volt/800;
  volt = round_to_dp(volt,2);
  if(volt < 2.5){
    ntu = 3000;
    }
    else{
      ntu = -1120.4*square(volt)+5742.3*volt-4353.9;// converitng to ntu in the range of 0-3000
      ntu_1 = (ntu/3000)*5; // converitng to ntu in the range of 0-5
    }
    Serial.print("ntu: ");
    Serial.println(ntu_1);
    delay(500);
}
float round_to_dp( float in_value, int decimal_place )// returns a specific number of decimal places
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

// ph sensor
void ph_sensor(){
 for(int i=0;i<10;i++) 
 { 
 buffer_arr[i]=analogRead(ph_pin);
 delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buffer_arr[i]>buffer_arr[j])
 {
 temp=buffer_arr[i];
 buffer_arr[i]=buffer_arr[j];
 buffer_arr[j]=temp;
 }
 }
 }
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i];
 float volt=(float)avgval*5.0/1024/6; 
 ph_act = -5.70 * volt + calibration_value;

 Serial.print("pH Val: ");
 Serial.println(ph_act);
 delay(1000);
}
void init_gsm(){
  gsm.println("AT");// gsm status
  ShowSerialData();
  gsm.println("AT+CPIN?");// pin code
  ShowSerialData();
  gsm.println("AT+CREG?");// checks the registration status
  ShowSerialData();
  gsm.println("AT+CFUN=1");// full functionality of the gsm
  ShowSerialData();
  gsm.println("AT+CGATT=1");// attach or deattach device to packet domain in order to send data packets on network
  ShowSerialData();
  gsm.println("AT+CIPSTATUS");// returns the connection status
  ShowSerialData();
  gsm.println("AT+CIPMUX=0"); // configures device for a single IP connection
  ShowSerialData();
  gsm.println("AT+CSTT=\"AIRTEL\"");//start task and setting the APN,
  ShowSerialData();
  gsm.println("AT+CIICR");//bring up wireless connection
  ShowSerialData();
  gsm.println("AT+CIFSR");//get local IP adress
  ShowSerialData();  
}

void interval_write_thingspeak() {
  start: //label
  error = 0;
  nrf_receiver();
  turbidity();
  ph_sensor();

  elapsedWriteTime = millis() - startWriteTiming;

  if (elapsedWriteTime > (writeTimingSeconds * 1000))
  {
    writeThingSpeak();
    startWriteTiming = millis();
  }

  if (error == 1) //Resend if transmission is not completed
  {
    Serial.println(" <<<< ERROR >>>>");
    goto start; //go to label "start"
  }  
}

//--------------------------------------LCD
void lcd_init(){
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Water Quality");
  lcd.setCursor(0,1);
  lcd.print("Circuit Digest");
}
void water_quality_display(){
  interval_write_thingspeak();
  if (ph_act < 5.5 || ph_act > 8.5){
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Quality: Bad");
     lcd.setCursor(0,1);
     lcd.print("Vol:");
     lcd.print(vol);
     lcd.print(" L");
  }
  else if (ntu_1 > 5.0){

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Quality: Bad");
    lcd.setCursor(0,1);
    lcd.print("Vol:");
    lcd.print(vol);
    lcd.print(" L");
  }
  else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Quality: Good");
    lcd.setCursor(0,1);
    lcd.print("Vol:");
    lcd.print(vol);
    lcd.print(" L");
  }
}

// receiving nrf data
void nrf_receiver() {
  if ( radio.available()) {
    // Variable for the received timestamp
    while (radio.available()) {                                   // While there is data ready
      radio.read(&vol, sizeof(vol));             // Get the payload
    }
    radio.stopListening();                                        // First, stop listening so we can talk

    radio.write(&ntu_1, sizeof(ntu_1));              // Send the final one back.
    radio.write(&ph_act, sizeof(ph_act));              // Send the final one back.
    radio.startListening();                                       // Now, resume listening so we catch the next packets.
    
    Serial.print(F("Got message, Volume '"));
    Serial.print(vol);
    Serial.print(F("', Sent response '"));
    Serial.print("Ph: ");
    Serial.print(ph_act);
    Serial.print("Turbidity: ");
    Serial.print(ntu_1);
    Serial.println(F("'"));
  }
}
//---------------------------------------------- nrf receiver
