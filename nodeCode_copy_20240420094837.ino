float incomingPhValue;
float incomingTurbidityValue;

// void initializeDataSender();

void setup(){
  Serial.begin(115200);
  initializeDataSender();
}

void loop(){
  if(Serial.available()){
      String msg = "";
      char c = Serial.read();
      if(c == '*')
      {
          msg = Serial.readStringUntil('#');
          msg.trim();
      }

      // while(Serial.read());
      //7.88,4.43
      int sep_index = msg.indexOf(',');
      String str_val = msg.substring(0, sep_index);
      float ph_act = str_val.toFloat();
      str_val = msg.substring(sep_index+1);
      float ntu_1 = str_val.toFloat();

      sendData(ph_act, ntu_1);
  }
}
