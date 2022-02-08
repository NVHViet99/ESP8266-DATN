void loop() {
  ir_right_state = digitalRead( ir_right_pin );
  ir_left_state =  digitalRead( ir_left_pin );
  checkWalkIn();
  checkWalkOUT();
  getFingerprintID();
  if (check[0] && check[1]) {
    Serial.println(check[0] + "asdf" + check[1]);
    Firebase.pushString(ledData, "/return/Matched_Id", idAndDate + " " + Status);
    //    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    check[0] = false;
    check[1] = false;
  }
  lcd.setCursor(0, 0);
  lcd.print("Welcome to my office");
  lcd.setCursor(0, 2);
  lcd.print("People inside: ");
  lcd.print(total_count);
  //  lcd.print("Hi, " + a.substring(1, a.length() - 1));
  digitalClockDisplay();
  lcd.setCursor(1, 3);
  lcd.print(DateofTime);
  if (num != 0) {// ID #0 not allowed, try again!
    Serial.print("Enrolling ID #");
    Serial.println(num);
    getFingerprintEnroll();
  }

  if (del != 0) {
    Serial.print("Deleting ID #");
    Serial.println(del);
    deleteFingerprint(del);
  }

}
