static const uint8_t pacman_open[] = {
  0x0E,
  0x0F,
  0x1E,
  0x1C,
  0x1C,
  0x1E,
  0x0F,
  0x0E
};

static const uint8_t pacman_closed[] = {
  0x0E,
  0x0E,
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x0E,
  0x0E
};

void easter_egg() {
  lcd.clear();
  lcd.createChar(0, pacman_closed);
  lcd.createChar(1, pacman_open);
  lcd.home();
  lcd.write(0);
  lcd.print(" You found the");
  lcd.setCursor(0, 1);
  lcd.print("   easter egg !");

  delay(2000);

  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t j = 0; j < 16; j++) {
      if (j > 0) {
        lcd.setCursor(-1 + j, i);
        lcd.print(" ");
      }
      if (j == 0 && i == 1) {
        lcd.setCursor(15, 0);
        lcd.write(" ");
      }
      lcd.setCursor(j, i);
      if ((1 & j) == 0) {
        lcd.write(0);
      } else {
        lcd.write(1);
      }
      delay(400);
    }
  }

  goto_closed();
}
