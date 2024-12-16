#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Thinh"
#define WIFI_PASSWORD "12345678"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCm-HmgnmGyfeBlXdZy0AZuDN7HMHzmOVY"

// Insert RTDB URL
#define DATABASE_URL "https://esp32-database-7b3e1-default-rtdb.asia-southeast1.firebasedatabase.app/" 

// IP address of the target ESP32
#define TARGET_IP "192.168.221.80" // Thay bằng địa chỉ IP của ESP32 cần kiểm tra
#define TARGET_PORT 80 // Cổng thường dùng cho HTTP

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long checkPrevMillis = 0;
unsigned long checkInterval = 1000; // Rút ngắn thời gian kiểm tra còn 1 giây
bool signupOK = false;

void setup() {
  Serial.begin(115200);

  // Kết nối Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Setup Firebase credentials
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Đăng ký Firebase
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Sign-Up OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase Sign-Up Error: %s\n", config.signer.signupError.message.c_str());
  }

  // Gán hàm callback trạng thái token
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - checkPrevMillis > checkInterval || checkPrevMillis == 0)) {
    checkPrevMillis = millis();

    // Kiểm tra kết nối đến địa chỉ IP mục tiêu
    Serial.print("Checking target IP: ");
    Serial.println(TARGET_IP);

    WiFiClient client;
    bool state = false;

    // Kết nối đến địa chỉ IP mục tiêu
    if (client.connect(TARGET_IP, TARGET_PORT)) {
      delay(100); // Đợi một chút để xác nhận trạng thái kết nối
      if (client.connected()) {
        Serial.println("Target IP is reachable!");
        state = true;
      } else {
        Serial.println("Target IP is NOT reachable!");
      }
      client.stop();
    } else {
      Serial.println("Failed to connect to target IP!");
    }

    // Upload trạng thái lên Firebase
    if (Firebase.RTDB.setBool(&fbdo, "sensor/state", state)) {
      Serial.println("State uploaded successfully!");
    } else {
      Serial.println("Failed to upload state!");
      Serial.println(fbdo.errorReason());
    }
  }
}
