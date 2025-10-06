static const uint8_t heart[] = {
  0x00,
  0x0a, 
  0x1f,
  0x1f,
  0x1f,
  0x0e,
  0x04,
  0x00,
};

void easter_egg() {
  lcd.clear();
  lcd.createChar(0, heart);
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
      lcd.write(0);
      delay(200);
    }
  }

  goto_closed();
}
