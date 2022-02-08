void streamCallback(StreamData data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data);

  if (data.stringData() == "d1_on") {
    digitalWrite(D1, LOW);
    Serial.println("led 1 on");
  }
  else if (data.stringData() == "d1_off") {
    digitalWrite(D1, HIGH);
    Serial.println("led 1 off   ");
  }
  else if (data.stringData() == "d2_on") {
    digitalWrite(D2, LOW);
  }
  else if (data.stringData() == "d2_off") {
    digitalWrite(D2, HIGH);
  }
  else if (data.stringData() == "d3_on") {
    digitalWrite(D3, LOW);
  }
  else if (data.stringData() == "d3_off") {
    digitalWrite(D3, HIGH);
  }
  else if (data.stringData() == "d4_on") {
    digitalWrite(D4, LOW);
  }
  else if (data.stringData() == "d4_off") {
    digitalWrite(D4, HIGH);
  }

  num = data.intData();

  Serial.println();
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}

void streamCallback2(StreamData data2)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data2.streamPath().c_str(),
                data2.dataPath().c_str(),
                data2.dataType().c_str(),
                data2.eventType().c_str());
  printResult(data2);

  del = data2.intData();

  Serial.println();
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data2.payloadLength(), data2.maxPayloadLength());
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}
