#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>         // for Date/Time operations
#include <WiFiUdp.h>         // for UDP NTP
#include <DHT.h>
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// NTP Servers:
IPAddress timeServer(195, 186, 4, 101); // 195.186.4.101 (bwntp2.bluewin.ch)
const char* ntpServerName = "ch.pool.ntp.org";
const int timeZone = 7;     // Central European Time (summer time)

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

/////// for sensor PM 2.5
int measurePin = A0;
int ledPower   = D0;
/************* Flags and Counts***********************/
int checkSensorDoor = 1;
/////// for sensor PM 2.5
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9620;
float voMeasured = 0;
float calcVoltage = 0;
float pm = 0;
float dustDensity = 0.0;
unsigned long voMeasuredTotal = 0;
int voCount = 0; //biến lấy mẫu
// Use the typical sensitivity in units of V per 100ug/m3.
const float K = 1;
float Voc = 0.6;

static uint32_t  currentmls = millis(), intervalPM = currentmls;

#define FIREBASE_HOST "dht11-29479-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "fOzWRh1XAiomyXh2OSEcG9519AdH5C69plklMPc6"
#define WIFI_SSID "vinh vinh"
#define WIFI_PASSWORD "matkhaumoi"

//Define FirebaseESP8266 data object
FirebaseData stream;
FirebaseData sensor;

String flag_temp = "";
int flag_hum;
float flag_pm;

String Day;
String Month;
String Year;
String Hour ;
String Minute;
String Second;

void setup()
{
  Serial.begin(9600);
  pinMode(RX, OUTPUT);
  pinMode(TX, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D4, OUTPUT);

  digitalWrite(RX, HIGH);
  digitalWrite(TX, HIGH);
  digitalWrite(D7, HIGH);
  digitalWrite(D4, HIGH);

  pinMode(ledPower, OUTPUT);

  dht.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Udp.begin(localPort);
  WiFi.hostByName(ntpServerName, timeServer);
  setSyncProvider(getNtpTime);
  setSyncInterval(60);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Serial.println("Start!");

#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif

  if (!Firebase.beginStream(stream, "/signUpFinger"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);

}

void streamCallback(StreamData data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data);

  if (data.stringData() == "d1_on") {
    digitalWrite(RX, LOW);
    Serial.println("led 1 on");
  }
  else if (data.stringData() == "d1_off") {
    digitalWrite(RX, HIGH);
    Serial.println("led 1 off   ");
  }
  else if (data.stringData() == "d2_on") {
    digitalWrite(TX, LOW);
  }
  else if (data.stringData() == "d2_off") {
    digitalWrite(TX, HIGH);
  }
  else if (data.stringData() == "d3_on") {
    digitalWrite(D7, LOW);
  }
  else if (data.stringData() == "d3_off") {
    digitalWrite(D7, HIGH);
  }
  else if (data.stringData() == "d4_on") {
    digitalWrite(D4, LOW);
  }
  else if (data.stringData() == "d4_off") {
    digitalWrite(D4, HIGH);
  }
  else {
    return;
  }

  Serial.println();
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}


void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void loop() {
  sensorDHT11();
  if ( millis() - intervalPM > 10000) {
    // save the last time you updated the PM 2.5 values
    intervalPM = millis();
    PM();
  }
}

void sensorDHT11()
{
  int h = dht.readHumidity();
  int t = dht.readTemperature();
  String temp = String(t);
  String hum = String(h);
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  if (temp != flag_temp && t != 2147483647 && Year != "1970")
  {
    digitalClockDisplay();
    String tempOfTime = temp + String(" ") + Hour + ":"  + Minute + ":"  + Second + String(" ") + Day + String("-") + Month + String("-") + Year;
    Firebase.pushString(sensor, "/sensor/matching", tempOfTime);
    flag_temp = temp;
    Serial.println(temp);
  }
  if (abs(h - flag_hum) >= 2 && h != 2147483647 && Year != "1970")
  {
    digitalClockDisplay();
    String tempOfTime = hum + String(" ") + Hour + ":"  + Minute + ":"  + Second + String(" ") + Day + String("-") + Month + String("-") + Year;
    Firebase.pushString(sensor, "/sensor/matchingHum", tempOfTime);
    flag_hum = h;
  }
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

//------Function-----
void digitalClockDisplay()  {
  Year = String(year());
  Month = String(month());
  Day = String(day());
  Hour = String(hour());
  Minute = String(minute());
  Second = String(second());
  String DateofTime = Day + String("-") + Month + String("-") + Year + ", " + Hour + ":"  + Minute + ":" + Second;
  Serial.println(DateofTime);
}

void PM() {
  voCount = 0;
  voMeasuredTotal = 0;
  while (voCount <= 10) {
    digitalWrite(ledPower, LOW);     // Bật IR LED
    delayMicroseconds(samplingTime);   //Delay 0.28ms
    voMeasured = analogRead(measurePin); // Đọc giá trị ADC V0 mất khoảng 0.1ms
    digitalWrite(ledPower, HIGH);    // Tắt LED
    delayMicroseconds(sleepTime);     //Delay 9.62ms
    voMeasuredTotal += voMeasured;    // Tính tổng lần lấy mẫu
    voCount ++;              // Đếm số lần lấy mẫu
  }
  voMeasured = 1.0 * voMeasuredTotal / 100; //Tính trung bình
  //****************************

  calcVoltage = voMeasured / 1024 * 5; //Tính điện áp Vcc của cảm biến (5.0 hoặc 3.3)
  dustDensity = calcVoltage / K * 100.0;
  pm = dustDensity * 10;
  Serial.println(pm);
  if (abs(pm - flag_pm) >= 2) {
    digitalClockDisplay();
    String pmSensor = String(pm) + String(" ") + Hour + ":"  + Minute + ":"  + Second + String(" ") + Day + String("-") + Month + String("-") + Year;
    Firebase.pushString(sensor, "/sensor/matchingPM", pmSensor);
    flag_pm = pm;
  }
}
