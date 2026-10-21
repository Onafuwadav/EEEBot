#include <Wire.h>  
#include <stdio.h>
#include <string.h>

String message;
String token;
volatile bool newMessage = false;  // set by receiveEvent, cleared by loop
// Here is where I create the library of morse code
char morse_a[] = ".-";
char morse_b[] = "-...";
char morse_c[] = "-.-.";
char morse_d[] = "-..";
char morse_e[] = ".";
char morse_f[] = "..-.";
char morse_g[] = "--.";
char morse_h[] = "....";
char morse_i[] = "..";
char morse_j[] = ".---";
char morse_k[] = "-.-";
char morse_l[] = ".-..";
char morse_m[] = "--";
char morse_n[] = "-.";
char morse_o[] = "---";
char morse_p[] = ".--.";
char morse_q[] = "--.-";
char morse_r[] = ".-.";
char morse_s[] = "...";
char morse_t[] = "-";
char morse_u[] = "..-";
char morse_v[] = "...-";
char morse_w[] = ".--";
char morse_x[] = "-..-";
char morse_y[] = "-.--";
char morse_z[] = "--..";

// Here is where I convert regular ASCII characters into morse code
char* char2morse(char c) {
  switch (c) {
    case 'a':
      return morse_a;
      break;

    case 'b':
      return morse_b;
      break;

    case 'c':
      return morse_c;
      break;

    case 'd':
      return morse_d;
      break;

    case 'e':
      return morse_e;
      break;

    case 'f':
      return morse_f;
      break;

    case 'g':
      return morse_g;
      break;

    case 'h':
      return morse_h;
      break;

    case 'i':
      return morse_i;
      break;

    case 'j':
      return morse_j;
      break;

    case 'k':
      return morse_k;
      break;

    case 'l':
      return morse_l;
      break;

    case 'm':
      return morse_m;
      break;

    case 'n':
      return morse_n;
      break;

    case 'o':
      return morse_o;
      break;

    case 'p':
      return morse_p;
      break;

    case 'q':
      return morse_q;
      break;

    case 'r':
      return morse_r;
      break;

    case 's':
      return morse_s;
      break;

    case 't':
      return morse_t;
      break;

    case 'u':
      return morse_u;
      break;

    case 'v':
      return morse_v;
      break;

    case 'w':
      return morse_w;
      break;

    case 'x':
      return morse_x;
      break;

    case 'y':
      return morse_y;
      break;

    case 'z':
      return morse_z;
      break;

    case ' ':
      return " ";
    default:
      return "#";
  }
}
//Here is where I convert morse code into regular ASCII characters
char morse2char(String m) {
  if (m.equals(morse_a)) {
    return 'a';
  } else if (m.equals(morse_b)) {
    return 'b';
  } else if (m.equals(morse_c)) {
    return 'c';
  } else if (m.equals(morse_d)) {
    return 'd';
  } else if (m.equals(morse_e)) {
    return 'e';
  } else if (m.equals(morse_f)) {
    return 'f';
  } else if (m.equals(morse_g)) {
    return 'g';
  } else if (m.equals(morse_h)) {
    return 'h';
  } else if (m.equals(morse_i)) {
    return 'i';
  } else if (m.equals(morse_j)) {
    return 'j';
  } else if (m.equals(morse_k)) {
    return 'k';
  } else if (m.equals(morse_l)) {
    return 'l';
  } else if (m.equals(morse_m)) {
    return 'm';
  } else if (m.equals(morse_o)) {
    return 'o';
  } else if (m.equals(morse_p)) {
    return 'p';
  } else if (m.equals(morse_q)) {
    return 'q';
  } else if (m.equals(morse_r)) {
    return 'r';
  } else if (m.equals(morse_s)) {
    return 's';
  } else if (m.equals(morse_t)) {
    return 't';
  } else if (m.equals(morse_u)) {
    return 'u';
  } else if (m.equals(morse_v)) {
    return 'v';
  } else if (m.equals(morse_w)) {
    return 'w';
  } else if (m.equals(morse_x)) {
    return 'x';
  } else if (m.equals(morse_y)) {
    return 'y';
  } else if (m.equals(morse_z)) {
    return 'z';
  } else if (m.equals(" ")) {
    return ' ';
  } else {
    return '#';
  }
}

void setup() {
  Wire.begin(0x08);              // join i2c bus with address 8
  Wire.onReceive(receiveEvent);  // create a receive event
  Serial.begin(9600);            // start serial to visualise data
  // received via serial monitor in the IDE
}

void loop() {
  if (newMessage) {
    noInterrupts();           // copy the message so the next I2C write can't change it mid-decode
    String received = message;
    newMessage = false;
    interrupts();
    decodeMessage(received);
  }
}


// Runs inside the I2C interrupt: only store the bytes and flag them.
// Serial printing and delay() don't belong here.
void receiveEvent(int howMany) {
  message = "";
  while (Wire.available()) {  // loop whilst bus is busy
    char c = Wire.read();     // receive data byte by byte
    message += c;             // form complete string
  }
  newMessage = true;
}

void decodeMessage(String message) {
  Serial.println(message);  // write string to serial monitor

  char str[message.length() + 1];
  strcpy(str, message.c_str());
  char * token;
  token = strtok(str, " ");
  while (token != NULL) {
    Serial.print(morse2char(token));
    token = strtok (NULL, " ");
  }
  Serial.println("");
}
