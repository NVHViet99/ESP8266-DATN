uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();

  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("Detecting...");
      digitalWrite(D4, LOW);
      //      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("Detecting...");
      digitalWrite(D4, LOW);
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    lcd.clear();
    lcd.setCursor(4, 2);
    lcd.print("Found a print!");
    digitalWrite(D4, HIGH); // sets the digital pin 13 on
    for (int i=0; i<=1; i++){
      if (check[i] == false){
        check[i] = true;
        Serial.println("fingerrrrr" + check[i]);
        break;
      }
    }
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    lcd.clear();
    lcd.setCursor(5, 2);
    lcd.print("Not match!");
    digitalWrite(D3, HIGH); // sets the digital pin 13 on
    delay(500);            // waits for a second
    digitalWrite(D3, LOW);  // sets the digital pin 13 off
    delay(500);            // waits for a second
    return p;
  } else {
    Serial.println("Unknown error");
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("Detecting...");
    digitalWrite(D4, LOW);
    return p;
  }

  digitalClockDisplay();

  Serial.println(Status);
  idAndDate = String("User ") + String(finger.fingerID) + String(" sign in on ") + DateofTime;
  if ( Firebase.getString(ledData, "/fingerPrintId/" + String("User") + String(finger.fingerID) + String("/fullName"))) {
    String stringValue = ledData.stringData();
    a = stringValue;
    lcd.clear();
    lcd.setCursor(0, 2);
    int lastIndex = a.length() - 1;
    lcd.print("Hi " + a.substring(1, lastIndex));
    Serial.println(a.substring(1, lastIndex));
    lcd.setCursor(0, 3);
    lcd.print(DateofTime);
    digitalWrite(D4, HIGH); // sets the digital pin 13 on
    delay(3000);
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  return finger.fingerID;

}
