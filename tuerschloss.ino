#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 1);

typedef enum {
  closed,
  open,
  service,
} state_t;

state_t state;

uint8_t code[4];
unsigned long timer[4];
uint8_t code_input[4];

#define LOCK_TIME 500 // half a second before a button is registered again

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  // buttons
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  // red LEB
  pinMode(6, OUTPUT);
  // green LED
  pinMode(7, OUTPUT);
  // write password as 1, 2, 3, 4
  for (int i = 0; i < 4; i++) {
    code[i] = 1+i;
    timer[i] = 0;
    code_input[i] = 0;
  }
  state = closed;
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter a PIN");
  delay(1);
}

void closed_loop() {
  bool sensor_val[4];

  // transform the raw input
  // if a button was pressed less then the time in LOCK_TIME it will not be registered
  for (int i = 0; i < 4; i++) {
    sensor_val[i] = digitalRead(2+i);
    if (sensor_val[i] == LOW) {
      if (timer[i] == 0) {
        timer[i] = millis();
      } else if (timer[i] + LOCK_TIME <= millis()) {
        sensor_val[i] = HIGH;
        timer[i] = 0;
      } else {
        sensor_val[i] = HIGH;
      }
    }
  }

  // check which buttons are pressed
  for (int i = 0; i < 4; i++) {
    // if a button is pressed
    if (sensor_val[i] == LOW) {
      Serial.print("Pressed button: ");
      Serial.println(1 + i);
      // search for the last spot in the list
      for (int j = 0; j < 4; j++) {
        if (code_input[j] == 0) {

          // this button is pressed
          code_input[j] = 1 + i;
          break;
        }
      }
      lcd.clear();
      // print the input sequence
      for (int j = 0; j < 4; j++) {
        Serial.print(code_input[j]);
        lcd.setCursor(j,0);
        lcd.print(code_input[j]);
      }
      Serial.println();
      // is the list full?
      if (code_input[3] != 0) {
        // rigth code input
        if (memcmp(code_input, code, 4) == 0) {
          Serial.println("Correct password");
          Serial.println("Door open");
          state = open;
          // when was the right code entered
          // stored in first entry of timer
          timer[0] = millis();
          digitalWrite(7, HIGH);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Door is open");
        // wrong code input
        } else {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Wrong password");
          Serial.println("Wrong password");
          timer[0] = millis();
          while (timer[0] + 5000 >= millis()) {
            digitalWrite(6, HIGH);
            delay(200);
            digitalWrite(6, LOW);
            delay(200);
          }
          digitalWrite(6, LOW);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Try again");
          Serial.println("Try again");
        }
        memset(code_input, 0, 4);
      }
      // only one button should be pressed at once
      break;
    }
  }
}

void open_loop() {
  if (timer[0] + 10000 >= millis()) {
    if (digitalRead(A0) == HIGH) {
      Serial.println("Service Mode");
      state = service;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Service mode");
    }
  } else {
    digitalWrite(7, LOW);
    memset(timer, 0, sizeof(unsigned long) * 4);
    state = closed;
    Serial.println("Door closed");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Enter a PIN");
  }
}
void service_loop() {
}

void loop() {
  switch (state) {
    case closed:
      closed_loop();
      break;
    case open:
      open_loop();
      break;
    case service:
      service_loop();
      break;
  }
}
