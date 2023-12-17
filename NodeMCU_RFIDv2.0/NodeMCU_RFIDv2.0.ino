#include <MFRC522Extended.h>
#include <deprecated.h>
#include <require_cpp11.h>

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SS_PIN D2
#define RST_PIN D1

MFRC522 mfrc522(SS_PIN, RST_PIN);

const char *ssid = "Kelly";
const char *password = "gwapoko123";
const char *device_token = "a6a8d66dabd5dd9c";

String URL = "http://172.20.10.7/rfidattendance/getdata.php"; // Replace with your server URL
String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;

void setup() {
  delay(1000);
  Serial.begin(9600);
  SPI.begin();
  Serial.println("Initializing RFID module...");
  mfrc522.PCD_Init();
  connectToWiFi();
}

void loop() {
  // Check if there's a connection to Wi-Fi or not
  if (!WiFi.isConnected()) {
    connectToWiFi(); // Retry to connect to Wi-Fi
  }

  // Delay to prevent continuous scanning
  delay(5000);

  // Print message to serial monitor
  Serial.println("Scanning for RFID cards...");

  // Check if a new RFID card is present
  if (!mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("Initializing RFID module...");
    return; // Go to the start of the loop if there is no card present
  }

  // Read the serial number of the card
  if (!mfrc522.PICC_ReadCardSerial()) {
    return; // If reading card serial returns false, go to the start of the loop
  }

  // Extract the UID of the card
  String CardID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }

  // Check if the card ID is the same as the previous one
  if (CardID == OldCardID) {
    return;
  } else {
    OldCardID = CardID;
  }

  // Send the card ID to the server
  SendCardID(CardID);
  delay(1000);
}

void SendCardID(String Card_uid) {
  Serial.println("Sending the Card ID");

  if (WiFi.isConnected()) {
    WiFiClient client;
    HTTPClient http;

    // Construct the URL with the data to send
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token);
    Link = URL + getData;

    Serial.print("Sending URL: ");
    Serial.println(Link);

    Serial.print("Sending data: ");
    Serial.println(getData);

    // Initiate HTTP request
    http.begin(client, Link);

    // Send the request
    int httpCode = http.GET();

    // Get the response payload
    String payload = http.getString();

    Serial.print("HTTP Response Code: ");
    Serial.println(httpCode);

    Serial.print("Server Response: ");
    Serial.println(payload);

    // Process the response
    if (httpCode == 200) {
      if (payload.substring(0, 5) == "login") {
        String user_name = payload.substring(5);
        Serial.print("Logged in as: ");
        Serial.println(user_name);
      } else if (payload.substring(0, 6) == "logout") {
        String user_name = payload.substring(6);
        Serial.print("Logged out as: ");
        Serial.println(user_name);
      } else if (payload == "successful") {
        // Handle successful response
      } else if (payload == "available") {
        // Handle available response
      }
      delay(100);
      http.end(); // Close connection
    }
  }
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Connected to Wi-Fi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Failed to connect to Wi-Fi. Please check your credentials.");
  }
}
