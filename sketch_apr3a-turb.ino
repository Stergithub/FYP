// turbidity sensor variables
const int sensorPin = A0;
const int digitalPin = 2; // Digital pin D2
float volt;
float ntu;
float ntu_1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(digitalPin, OUTPUT); // Set D2 as output
  digitalWrite(digitalPin, HIGH); // Set D2 to high
}

void loop() {
  // put your main code here, to run repeatedly:
  volt = 0;
  for (int i=0; i<800; i++)
  {
    volt += ((float)analogRead(sensorPin)/1023)*3.0; // Adjusted for 3V reference
  }
  volt = volt/800;
  volt = round_to_dp(volt,2);
  if(volt < 1.5){ // Adjusted threshold accordingly
    ntu = 3000;
  }
  else {
    ntu = -1120.4 * square(volt)+5742.3*volt-4353.9; // Converting to NTU in the range of 0-3000
    ntu_1 = (ntu/3000)*5; // Converting to NTU in the range of 0-5
  }
  Serial.print("ntu: ");
  Serial.println(ntu_1);
  delay(500);
}

float round_to_dp( float in_value, int decimal_place ) {
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

float square(float in_value) {
  return pow(in_value, 2);
}
