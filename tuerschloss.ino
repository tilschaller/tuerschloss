#include <LiquidCrystal_I2C.h>

#define SERIAL 1

LiquidCrystal_I2C lcd(0x27, 16, 2);

typedef enum {
  closed,
  open,
  service,
} state_t;

state_t state;

unsigned long timer[4];
uint8_t code[4];

bool print_new_service;

#define LOCK_TIME 500  // half a second before a button is registered again

void setup() {
#if SERIAL
  Serial.begin(9600);
#endif
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
  // piezo / speaker
  pinMode(8, OUTPUT);
  // write password as 1, 2, 3, 4
  for (int i = 0; i < 4; i++) {
    code[i] = 1 + i;
    timer[i] = 0;
  }
  state = closed;
  lcd.init();
  lcd.noCursor();
  lcd.backlight();
  lcd.clear();
  lcd.print("Enter a PIN");
  delay(1);
}

void closed_loop() { 
  static const uint8_t easter_egg_code[4] = {1,2,3,4};
 
  uint8_t i_code[4];
  get_code(i_code);

  if (memcmp(i_code, code, 4) == 0) {
#if SERIAL
    Serial.println("Correct password");
    Serial.println("Door open");
#endif
    state = open;
    // when was the right code entered
    // stored in first entry of timer
    timer[0] = millis();
    // turn on the green led
    digitalWrite(7, HIGH);
    lcd.clear();
    lcd.print("Door is open");
  } else if (memcmp(i_code, easter_egg_code, 4) == 0) { // this is executed when the code to open the door is not 1 2 3 4 and 1 2 3 4 is entered
    easter_egg();
  } else {  // wrong code
    lcd.clear();
    lcd.print("Wrong password");
#if SERIAL
    Serial.println("Wrong password");
#endif
    tone(8, 1000);
    timer[0] = millis();
    while (timer[0] + 5000 >= millis()) {
      digitalWrite(6, HIGH);
      delay(200);
      digitalWrite(6, LOW);
      delay(200);
    }
    noTone(8);
    digitalWrite(8, LOW);
    digitalWrite(6, LOW);
    lcd.clear();
    lcd.print("Try again");
#if SERIAL
    Serial.println("Try again");
#endif
  }
}

void open_loop() {
  if (timer[0] + 10000 >= millis()) {
    if (analogRead(A0) >= 1000) {
#if SERIAL
      Serial.println("Service Mode");
#endif
      lcd.cursor();
      lcd.blink_on();
      print_new_service = true;
      state = service;
    }
  } else {
    goto_closed();
  }
}

void service_loop() {
  uint16_t analog_read = analogRead(A0);

  if (print_new_service) {
    lcd.clear();
    lcd.print(" v     a      n ");
    print_new_service = false;
  }
  
  switch (analog_read) {
    case 0 ... 341: 
      lcd.setCursor(1, 0);
      break;
    case 342 ... 682: 
      lcd.setCursor(7, 0);
      break;
    case 683 ... 1024:
      lcd.setCursor(14, 0);
      break;
  }

  // if the enter button is pressed (Button 1)
  if (digitalRead(2) != LOW) return;

  lcd.noCursor();
  lcd.blink_off();

  print_new_service = true;
  switch (analog_read) {
    case 0 ... 341:
      goto_closed();
      break;
    case 342 ... 682:
      lcd.clear();
      lcd.print("Current code:");
      print_code(code);
      // show the code for 3 seconds
      delay(3000);
      lcd.blink_on();
      break;
    case 683 ... 1024:
      lcd.clear();
      lcd.print("New code:");
      get_code(code);
      tone(8, 8000, 200);
#if SERIAL
      Serial.print("New password: ");
#endif
      // print_code(code);
      delay(LOCK_TIME);
      lcd.blink_on();
      lcd.clear();
      break;
  }
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
