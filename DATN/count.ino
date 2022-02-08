void checkWalkIn() {
  if ( ir_right_state != ir_right_state_last ) {
    ir_right_state_last = ir_right_state;
    if ( (bWalkIn == false) && ( ir_right_state == LOW ) ) {
      bWalkIn = true;
      tm = millis();
    }
  }
  if ( (millis() - tm) > DELAY_TIMEOUT ) {
    bWalkIn = false;
  }
  if ( bWalkIn && (ir_left_state == LOW) && (ir_right_state == HIGH) ) {
    bWalkIn = false;
    in_counter++;
    total_count++;
    Status = "IN";
    for (int i=0; i<=1; i++){
      if (check[i] == false){
        check[i] = true;
        Serial.println("in" + check[i]);
        break;
      }
    }
    Serial.println("Innn");
  }
}

void checkWalkOUT() {
  if ( ir_left_state != ir_left_state_last ) {
    ir_left_state_last = ir_left_state;
    if ( (bWalkOut == false) && ( ir_left_state == LOW ) ) {
      bWalkOut = true;
      tm = millis();
    }
  }

  if ( (millis() - tm) > DELAY_TIMEOUT ) {
    bWalkOut = false;
  }

  if ( bWalkOut && (ir_right_state == LOW) && (ir_left_state == HIGH) ) {
    bWalkOut = false;
    out_counter++;
    Status = "OUT";
 
    total_count--;
    if(total_count < 0) total_count = 0;
    
    for (int i=0; i<=1; i++){
      if (check[i] == false){
        check[i] = true;
        Serial.println("ra" + check[i]);
        break;
      }
    }
    Serial.println("Outtt");
  }
}
