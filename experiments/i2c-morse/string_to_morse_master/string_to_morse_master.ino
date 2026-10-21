  #include <Wire.h>     // include Wire library
  #include<stdio.h>
  #include<math.h>

  String message;
  bool val = false;

  void setup() {
    Wire.begin();       // join i2c bus (address optional for master)
    input_morse();
  }

  void loop() {
    // Wire.beginTransmission(0x08); // transmit to slave device address 8
    // Wire.write(".- .- .-");          // send five bytes, one for each character
    // Wire.endTransmission();       // end transmission
    // delay(500);
  }

  void input_morse(){
    while(!val) {         // keep asking until a valid message is entered
      Serial.println("Enter morse code: ");
      while (!Serial.available()) {}            // wait for the user to type a line
      message = Serial.readStringUntil('\n');   // Arduino has no stdin, so scanf can't be used
      message.trim();                           // drop the trailing '\r' from the serial monitor
      for(int i = 0; i < message.length(); i++){
        if(message[i] == "." || message[i] == "-" || message[i] == " "){
          val = true;
        }else{
          val = false;
          break;
        }
      }
    if(val){
      Wire.beginTransmission(0x08); // transmit to slave device address 8
      for(int i = 0; i < message.length(); i++){
        Wire.write(message[i]);
      }
      // Wire.write(message);          
      Wire.endTransmission(); 
    }
  }

  } 