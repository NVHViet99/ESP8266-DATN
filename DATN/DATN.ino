#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Fingerprint.h>
#define DELAY_TIMEOUT 1500
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(D5, D6);
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

#include <TimeLib.h>         // for Date/Time operations
#include <WiFiUdp.h>

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
hd44780_I2Cexp lcd;

//lcd
byte Lock[] = {
  B01110,
  B10101,
  B10101,
  B10111,
  B10001,
  B10001,
  B01110,
  B00000
};
// NTP Servers:
IPAddress timeServer(195, 186, 4, 101); // 195.186.4.101 (bwntp2.bluewin.ch)
const char* ntpServerName = "ch.pool.ntp.org";
const int timeZone = 7;     // Central European Time (summer time)

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

int num;
int del;
String DateofTime;
String a;
String Status = "";
String idAndDate;

int ir_right_pin = D0;
int ir_left_pin = D7;

int ir_right_state = 0;
int ir_left_state  = 0;

int ir_right_state_last = -1;
int ir_left_state_last  = -1;

int in_counter = 0;
int out_counter = 0;
int total_count;
bool bWalkIn = false;
bool bWalkOut = false;

bool check[2] = {false, false};

unsigned long tm;

#define FIREBASE_HOST "dht11-29479-default-rtdb.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "fOzWRh1XAiomyXh2OSEcG9519AdH5C69plklMPc6"
#define WIFI_SSID "vinh vinh"
#define WIFI_PASSWORD "matkhaumoi"

//Define FirebaseESP8266 data object
FirebaseData ledData;
FirebaseData stream;
FirebaseData stream2;

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.createChar(7, Lock);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode( ir_right_pin, INPUT);
  pinMode( ir_left_pin , INPUT);
  lcd.clear();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");


  while (WiFi.status() != WL_CONNECTED)
  {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("Wifi Connecting...");
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Sever Connecting...");
  Serial.println();
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());

  Serial.print("Using NTP Server ");
  Serial.println(ntpServerName);
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServer);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  setSyncProvider(getNtpTime);
  setSyncInterval(60);
  finger.begin(57600);
  finger.getTemplateCount();
  if (finger.verifyPassword()) {
    Serial.println("Found `fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Didn't connect!");
    lcd.setCursor(2, 1);
    lcd.print("Please Reset!");
    while (1) {
      delay(1);
    }
  }
  Serial.println("Start!");

#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif

  if (!Firebase.beginStream(stream, "/signUpFinger"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);

#if defined(ESP8266)
  stream2.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif

  if (!Firebase.beginStream(stream2, "/deleteFinger"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream2, streamCallback2, streamTimeoutCallback);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting System");
  delay(1500);
}
