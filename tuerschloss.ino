typedef enum {
  Closed,
  Open,
  Service,
} state_t;

uint8_t code[4];
unsigned long locked[4];

#define LOCK_TIME 500 // half a second before this button is registered again

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A5, OUTPUT);
  // write password as 1, 2, 3, 4
  for (int i = 0; i < 4; i++) {
    code[i] = 1+i;
    locked[i] = 0;
  }
  delay(1);
}

void loop() {
  bool sensor_val[4];
  for (int i = 0; i < 4; i++) {
    sensor_val[i] = digitalRead(i+14);
    if (sensor_val[i] == LOW) {
      if (locked[i] == 0) {
        locked[i] = millis();
      } else if (locked[i] + LOCK_TIME <= millis()) {
        sensor_val[i] = HIGH;
        locked[i] = 0;
      } else {
        sensor_val[i] = HIGH;
      }
    }
  }

  if (sensor_val[1] == LOW) {
    Serial.println("Button 2 pressed");
    digitalWrite(A5, HIGH);
  } else {
    digitalWrite(A5, LOW);
  }
}
