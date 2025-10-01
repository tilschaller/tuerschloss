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
  // buttons
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  // red LEB
  pinMode(A4, OUTPUT);
  // green LED
  pinMode(A5, OUTPUT);
  // write password as 1, 2, 3, 4
  for (int i = 0; i < 4; i++) {
    code[i] = 1+i;
    timer[i] = 0;
    code_input[i] = 0;
  }
  state = closed;
  delay(1);
}

void closed_loop() {
  bool sensor_val[4];

  // transform the raw input
  // if a button was pressed less then the time in LOCK_TIME it will not be registered
  for (int i = 0; i < 4; i++) {
    sensor_val[i] = digitalRead(i+14);
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
      // print the input sequence
      for (int j = 0; j < 4; j++) {
        Serial.print(code_input[j]);
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
          // wrong code input
        } else {
          Serial.println("Wrong password");
          timer[0] = millis();
          while (timer[0] + 5000 >= millis()) {
            digitalWrite(A4, HIGH);
            delay(200);
            digitalWrite(A4, LOW);
            delay(200);
          }
          digitalWrite(A4, LOW);
          Serial.println("Try again");
        }
        memset(timer, 0, sizeof(unsigned long) * 4);
        memset(code_input, 0, 4);
      }
    }
  }
}

void open_loop() {
  if (timer[0] + 10000 >= millis()) {
    digitalWrite(A5, HIGH);
  } else {
    digitalWrite(A5, LOW);
    memset(timer, 0, sizeof(unsigned long) * 4);
    state = closed;
    Serial.println("Door closed");
  }
}
void service_loop() {}

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
