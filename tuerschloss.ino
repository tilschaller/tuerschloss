#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// 0 to disable serial output and 1 to enable
#define SERIAL 0

LiquidCrystal_I2C lcd(0x27, 16, 2);

typedef enum {
  closed,
  open,
  service,
} state_t;

state_t state;

unsigned long timer[4];
// stored in eeprom
int code_addr = 0;
uint8_t code[4];


typedef enum{
  Left = 0,
  Down,
  Up,
  Right,
} direction_t;

typedef enum {
  Switch,
  Verlassen,
  Anzeigen,
  Neu,
} service_state_t;

service_state_t service_state;

#define LOCK_TIME 500  // half a second before a button is registered again

void(* reboot)(void) = 0;

void setup() {
#if SERIAL
  Serial.begin(9600);
  while (!Serial) {}
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
  memset(timer, 0, sizeof(unsigned long) * 4);
  state = closed;
  lcd.init();
  lcd.noCursor();
  lcd.backlight();
  lcd.clear();
  lcd.print("Enter a PIN");
  delay(1);
}

static const uint8_t easter_egg_code[4] = {1,1,4,2};

void closed_loop() {
  // fetch the code from eeprom
  EEPROM.get(code_addr, code);
 
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
  } else if (memcmp(i_code, easter_egg_code, 4) == 0) {
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
      service_state = Switch;
      state = service;
    }
  } else {
    goto_closed();
  }
}

void service_loop() {
  uint16_t analog_read = analogRead(A0);

  if (service_state == Switch) {
    lcd.clear();
    lcd.print(" v     a      n ");
    switch (analog_read) {
      case 0 ... 341:
        lcd.setCursor(0, 1);
        lcd.print("Verlassen");
        service_state = Verlassen;
        lcd.setCursor(1, 0);
        break;
      case 342 ... 682:
        lcd.setCursor(0, 1);
        lcd.print("    Anzeigen");
        service_state = Anzeigen;
        lcd.setCursor(7, 0);
        break;
      case 683 ... 1024:
        lcd.setCursor(0, 1);
        lcd.print("      Neuer Code");
        service_state = Neu;
        lcd.setCursor(14, 0);
        break;
    }
  }

  switch (analog_read) {
    case 0 ... 341:
        if (service_state != Verlassen) 
          service_state = Switch;
        break;
    case 342 ... 682:
      if (service_state != Anzeigen)
        service_state = Switch;
      break;
    case 683 ... 1024:
      if (service_state != Neu) 
        service_state = Switch;
      break;
  }

  // if the enter button is not pressed (Button 1)
  if (digitalRead(2) != LOW) return;

  lcd.noCursor();
  lcd.blink_off();

  service_state = Switch;
  switch (analog_read) {
    case 0 ... 341:
      goto_closed();
      break;
    case 342 ... 682:
      lcd.clear();
      lcd.print("Current code:");
      EEPROM.get(code_addr, code);
      print_code(code);
      // show the code for 3 seconds
      delay(3000);
      lcd.blink_on();
      break;
    case 683 ... 1024:
      lcd.clear();
      lcd.print("New code:");
      get_code(code);
      EEPROM.put(code_addr, code);
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
