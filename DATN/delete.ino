uint8_t deleteFingerprint(uint8_t del) {
  uint8_t p = -1;

  p = finger.deleteModel(del);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("Deleted finger: " + String(del));
    Firebase.setInt(ledData, "/deleteFinger/chooseId", 0);
    Firebase.deleteNode(ledData, "/fingerPrintId/User" + String(del));
    finger.getTemplateCount();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}
