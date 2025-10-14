static const uint8_t pacman_open_right[] = {
  0x0E,
  0x0F,
  0x1E,
  0x1C,
  0x1C,
  0x1E,
  0x0F,
  0x0E
};

static const uint8_t pacman_open_left[] = {
  0x0E,
  0x1E,
  0x0F,
  0x07,
  0x07,
  0x0F,
  0x1E,
  0x0E,
};

static const uint8_t pacman_closed[] = {
  0x0E,
  0x0E,
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x0E,
  0x0E,
};

static const uint8_t food[] = {
  0x00,
  0x00,
  0x04,
  0x0E,
  0x0E,
  0x04,
  0x00,
  0x00,
};

static const uint8_t empty[] = {
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
};

void draw_board(uint8_t board[2][16]) {
  uint8_t cursor_pos[2] = {0};
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      lcd.setCursor(cursor_pos[0], cursor_pos[1]);
      cursor_pos[0]++;
      lcd.write(board[i][j]);
    }
    cursor_pos[0] = 0;
    cursor_pos[1]++;
  }
}

bool move_player_direction(direction_t direction, uint8_t board[2][16]) {
    // first find the player
    uint8_t player_pos[2]; // 1st entry = horinzontal | 2nd entry = vetical
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 16; j++) {
        if (board[i][j] == 0 || board[i][j] == 1 || board[i][j] == 2) {
          player_pos[0] = j;
          player_pos[1] = i;
        }
      }
    } 

    uint8_t new_player_pos[2];
    memcpy(new_player_pos, player_pos, 2);
    // move the player if he doesnt touch any contrainst
    switch (direction) {
      case Left: 
        new_player_pos[0]--;
        break;
      case Right:
        new_player_pos[0]++;
        break;
      case Up: 
        new_player_pos[1]--;
        break;
      case Down:
        new_player_pos[1]++;
        break;
    }

#if SERIAL
    Serial.println(player_pos[0]);
    Serial.println(player_pos[1]);
    Serial.println(new_player_pos[0]);
    Serial.println(new_player_pos[1]);
#endif

    // player would be moved out of bounds so we return
    if (new_player_pos[0] > 15 || new_player_pos[1] > 1) {
#if SERIAL
      Serial.println("tried to move player out of bounds");
#endif 
      return false;
    }

    // write pacman sprite at new spot
    if (board[player_pos[1]][player_pos[0]] == 0) {
      switch (direction) {
        case Left:
          board[new_player_pos[1]][new_player_pos[0]] = 2;
          break;
        case Right:
          board[new_player_pos[1]][new_player_pos[0]] = 1;
          break;
        default: 
          board[new_player_pos[1]][new_player_pos[0]] = 0;
      }
    } else {
      board[new_player_pos[1]][new_player_pos[0]] = 0;
    }
    
    board[player_pos[1]][player_pos[0]] = 4;

    draw_board(board);

    // check if the game is finished
    unsigned int sum = 0; 
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 16; j++) {
        sum += board[i][j];
      }
    }

    unsigned int comp_val;
    switch (board[new_player_pos[1]][new_player_pos[0]]) {
      case 0:
        comp_val = 124;
        break;
      case 1:
        comp_val = 125;
        break;
      case 2:
        comp_val = 126;
        break;
    }
    if (sum >= comp_val) {
      goto_closed();
      return true;
    }
    return false;
}

void easter_egg() {
  lcd.clear();
  lcd.createChar(0, pacman_closed);
  lcd.createChar(1, pacman_open_right);
  lcd.createChar(2, pacman_open_left);
  lcd.createChar(3, food);
  lcd.createChar(4, empty);
  lcd.home();
  lcd.write(0);
  lcd.print(" You found the");
  lcd.setCursor(0, 1);
  lcd.print("   easter egg !");

  delay(2000);
  
  // tutorial loop
  lcd.clear();
  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Key one:Tutorial");
    lcd.setCursor(0, 1);
    lcd.print("Key two:Play");
    if (digitalRead(2) == LOW) {
      lcd.clear();
      lcd.print("Key One: left");
      lcd.setCursor(0, 1);
      lcd.print("Key Four: right");
      delay(2000);
      lcd.clear();
      lcd.print("Key Two: down");
      lcd.setCursor(0, 1);
      lcd.print("Key Three: up");
      delay(2000);
      lcd.clear();
      continue;
    }
    if (digitalRead(3) == LOW) {
      break;
    }
  }

  delay(LOCK_TIME);

  bool sensor_val[4];

  uint8_t board[2][16];
  memset(board, 3, 32);
  board[0][0] = 0;

  draw_board(board);

  while (state == closed) {
    get_button_input(sensor_val);
    for (int i = 0; i < 4; i++) {
      if (sensor_val[i] == LOW) 
        if (!move_player_direction((direction_t)i, board)) {continue;} else {return;}
    }
  }
}
