#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Wire.h>
#include <stdio.h>
#include <math.h>

String str = "";
const byte MAX_CMD_LEN = 16; // one LCD line, and well inside the 32-byte I2C buffer
const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[rows] = {2, 0, 4, 17}; //connect to the row pinouts of the keypad
byte colPins[cols] = {5, 18, 19}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );
LiquidCrystal lcd(27, 14, 32, 33, 25, 26);

void setup()
{
  Wire.begin(); 
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 1);
  lcd.print("1F 2B 3CW 4ACW");  // key for the order digit
  lcd.setCursor(0, 0);           // commands are typed on the top line
}

void loop(){
  char key = keypad.getKey();
  if(key){
    if(key=='*' && str.length() >= 2){
      if(valid_command(str)){
        String sent = str;
        send_arduino();
        Serial.println("sent");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Sent " + sent);
        lcd.setCursor(0, 0);
      }else{
        Serial.println("invalid command");
        lcd.setCursor(0, 1);
        lcd.print("Invalid command ");
        lcd.setCursor(str.length(), 0);
      }

    }else if(key=='#'){
      Serial.println(key);
      lcd.clear();          // wipe the old command off the screen
      str = "";
      Serial.println(str);

    }else if(key!='*' && key!='#' && str.length() < MAX_CMD_LEN){
      lcd.print(key);
      str.concat(key);
      Serial.println(str);
    }
  }
}

// A command is <order><quantity>: order 1 = forward, 2 = back,
// 3 = clockwise, 4 = anticlockwise. Forward/back quantities are 10 cm
// steps (capped at 99 so quantity * 100 still fits the Nano's 16-bit int);
// turns must be 90 or 180 degrees.
bool valid_command(String cmd){
  if(cmd.length() < 2){
    return false;
  }
  char order = cmd.charAt(0);
  int quantity = cmd.substring(1).toInt();
  if(order == '1' || order == '2'){
    return quantity > 0 && quantity <= 99;
  }
  if(order == '3' || order == '4'){
    return quantity == 90 || quantity == 180;
  }
  return false;
}

void send_arduino(){
  Wire.beginTransmission(0x08); // transmit to slave device address 8
  Wire.write((const uint8_t *)str.c_str(), str.length());
  Wire.endTransmission(); 
  str = "";
}

