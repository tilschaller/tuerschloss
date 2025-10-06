void get_button_input(bool sensor_val[4]) {
  // transform the raw input
  // if a button was pressed less time age then to time in LOCK_TIME it will not be registered
  for (int i = 0; i < 4; i++) {
    sensor_val[i] = digitalRead(2 + i);
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
}

void print_code(uint8_t i_code[4]) {
  for (int j = 0; j < 4; j++) {
#if SERIAL
    Serial.print(i_code[j]);
#endif
    lcd.setCursor(j, 1);
    lcd.print(i_code[j]);
  }
#if SERIAL
  Serial.println();
#endif
}

void goto_closed() {
  digitalWrite(7, LOW);
  memset(timer, 0, sizeof(unsigned long) * 4);
  state = closed;
#if SERIAL 
  Serial.println("Door closed");
#endif
  delay(LOCK_TIME);
  lcd.clear();
  lcd.print("Enter a PIN");
}

void get_code(uint8_t i_code[4]) {
  bool sensor_val[4];

  memset(i_code, 0, 4);
  delay(LOCK_TIME);
  for (;;) {
    get_button_input(sensor_val);

    // check which buttons are pressed
    for (int i = 0; i < 4; i++) {
      // if a button is pressed
      if (sensor_val[i] == LOW) {
#if SERIAL
        Serial.print("Pressed button: ");
        Serial.println(1 + i);
#endif
        // search for the last spot in the list
        for (int j = 0; j < 4; j++) {
          if (i_code[j] == 0) {
            // this button is pressed
            i_code[j] = 1 + i;
            print_code(i_code);
            break;
          }
        }
      }
    }
    if (i_code[3] != 0) return;
  }
}
