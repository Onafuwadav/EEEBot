#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Wire.h>
#include <stdio.h>
#include <math.h>

String str = "";
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
  lcd.print("hello");
}

void loop(){
  char key = keypad.getKey();
  if(key){
    if(key=='*' && str.length() >= 2){
      send_arduino();
      Serial.println("sent");

    }else if(key=='#'){
      Serial.println(key);
      lcd.print(key);
      str = "";
      Serial.println(str);

    }else if(key!='#'){
      lcd.print(key);
      str.concat(key);
      Serial.println(str);
    }
  }
}

void send_arduino(){
  char arr[32]; 
  strcpy(arr, str.c_str());
  Wire.beginTransmission(0x08); // transmit to slave device address 8
    for(int i = 0; i < str.length(); i++){
      Wire.write(arr[i]);
    }      
  Wire.endTransmission(); 
  str = "";
}

