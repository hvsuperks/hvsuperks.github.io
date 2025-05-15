#include <WiFi.h>
#include <WebServer.h>
#include <Firebase_ESP_Client.h>
#include <Preferences.h>
#include <Update.h>
#include <HTTPClient.h>
#include "Temp_sensor.h"
// Thư viện Firebase
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ----------------- CẤU HÌNH -------------------
#define WIFI_TIMEOUT_MS 20000
#define RECONNECT_INTERVAL 300000  // 5 phút

#define NTC1_PIN 34
#define NTC2_PIN 35

#define RELAY1 23
#define RELAY2 22
#define RELAY3 21
#define RELAY4 19

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
Preferences preferences;
WebServer server(80);

// Firebase config
const char* API_KEY = "AIzaSyAOOtP5Va7zCa9lH62H20piGYCvdACt9DE";
const char* DATABASE_URL = "https://be-ca-ad623-default-rtdb.asia-southeast1.firebasedatabase.app";

// Đường dẫn node Firebase
const char* relayPath = "/relays";
const char* tempPath = "/temperature";
const char* settingPath = "/settings";

// WiFi
String ssid, pass;
unsigned long lastReconnectAttempt = 0;
unsigned long lastFirebaseUpdate = 0;
bool wifiConnected = false;



// Upload Firmware
const String currentVersion = "v1.0.0";  // Cập nhật thủ công trước khi build
const String versionURL = "https://raw.githubusercontent.com/yourusername/esp32-ota/main/version.txt";
const String firmwareURL = "https://raw.githubusercontent.com/yourusername/esp32-ota/main/firmware.bin";
unsigned long lastCheckTime = 0;
#define CHECK_INTERVAL 3600000  // 1 giờ

// ---------------------------------------------

// Đọc từ Preferences (Flash)
void loadWiFiConfig() {
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  pass = preferences.getString("pass", "");
  preferences.end();
}

// Ghi lại WiFi config
void saveWiFiConfig(String s, String p) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", s);
  preferences.putString("pass", p);
  preferences.end();
}

// Đọc Setting từ preferences
void loadSetting(){
  preferences.begin("setting", false);
  preferences.getint("temp_set",28);
  preferences.get
  preferences.end();
}

// Cầu phân áp - đọc NTC 10k
float readNTCTemperature(int analogPin) {
  int adcValue = analogRead(analogPin);
  float voltage = adcValue * 3.3 / 4095.0;
  float resistance = (3.3 * 10000.0 / voltage) - 10000.0;
  float steinhart;
  steinhart = resistance / 10000.0;
  steinhart = log(steinhart);
  steinhart /= 3950.0;
  steinhart += 1.0 / (25 + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;
  return steinhart;
}

// Khởi động Web config
void startWebConfig() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html",
      "<form method='POST'>SSID:<input name='s'><br>Password:<input name='p'><br><input type='submit'></form>");
  });

  server.on("/", HTTP_POST, []() {
    ssid = server.arg("s");
    pass = server.arg("p");
    saveWiFiConfig(ssid, pass);
    server.send(200, "text/plain", "Đã lưu. Reset lại ESP.");
    delay(1000);
    //ESP.restart();
  });

  server.on("/update", HTTP_POST, []() {
    server.send(200);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Update.begin();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Update.end();
    }
  });

  server.begin();
  Serial.println("Web config đang chạy.");
}

// Kết nối WiFi tự động
bool connectWiFi() {
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.print("Đang kết nối WiFi: ");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }
  return WiFi.status() == WL_CONNECTED;
}

// Ghi dữ liệu lên Firebase
void uploadToFirebase() {
  float t1 = readNTCTemperature(NTC1_PIN);
  float t2 = readNTCTemperature(NTC2_PIN);

  Firebase.RTDB.setFloat(&fbdo, String(tempPath) + "/t1", t1);
  Firebase.RTDB.setFloat(&fbdo, String(tempPath) + "/t2", t2);
  Firebase.RTDB.setInt(&fbdo, String(relayPath) + "/r1", digitalRead(RELAY1));
  Firebase.RTDB.setInt(&fbdo, String(relayPath) + "/r2", digitalRead(RELAY2));
  Firebase.RTDB.setInt(&fbdo, String(relayPath) + "/r3", digitalRead(RELAY3));
  Firebase.RTDB.setInt(&fbdo, String(relayPath) + "/r4", digitalRead(RELAY4));
}

// Đọc lệnh từ Firebase
void getRelayCommands() {
  for (int i = 1; i <= 4; i++) {
    String path = String(relayPath) + "/r" + i;
    if (Firebase.RTDB.getInt(&fbdo, path)) {
      digitalWrite(RELAY1 + i - 1, fbdo.intData() ? HIGH : LOW);
    }
  }
}

// UPdate FirmWare
void checkAndUpdateFirmware() {
  HTTPClient http;
  http.begin(versionURL);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String newVersion = http.getString();
    newVersion.trim(); // xóa xuống dòng hoặc khoảng trắng

    Serial.println("Phiên bản hiện tại: " + currentVersion);
    Serial.println("Phiên bản trên GitHub: " + newVersion);

    if (newVersion != currentVersion) {
      Serial.println("Có bản cập nhật mới, tiến hành OTA...");

      http.end();  // đóng kết nối version

      // Bắt đầu tải firmware
      http.begin(firmwareURL);
      int fwCode = http.GET();
      if (fwCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        WiFiClient *stream = http.getStreamPtr();

        if (!Update.begin(contentLength)) {
          Serial.println("Update.begin() thất bại");
          return;
        }

        size_t written = Update.writeStream(*stream);
        if (written == contentLength && Update.end(true)) {
          Serial.println("Cập nhật thành công! Reset ESP...");
          delay(1000);
          ESP.restart();
        } else {
          Serial.println("Lỗi khi ghi firmware");
        }
      } else {
        Serial.println("Không tìm thấy firmware.bin");
      }
    } else {
      Serial.println("Đã là bản mới nhất.");
    }
  } else {
    Serial.println("Không đọc được version.txt từ GitHub");
  }
  http.end();
}

void setup() {
  Serial.begin(115200);

  // Init chân relay
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);

  // Tải WiFi từ ROM
  loadWiFiConfig();

  // Kết nối WiFi
  if (connectWiFi()) {
    wifiConnected = true;
    Serial.println("\nWiFi connected!");
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi FAIL - mở Web config");
    WiFi.softAP("ESP32_Config");
    startWebConfig();
  }

  // Cấu hình Firebase nếu WiFi có
  if (wifiConnected) {
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = "HVsuperKS@gmail.com";
    auth.user.password = "SAObang!((#";

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  }
}

void loop() {
  if (wifiConnected) {
    if (Firebase.ready() && millis() - lastFirebaseUpdate > 5000) {
      lastFirebaseUpdate = millis();
      uploadToFirebase();
      getRelayCommands();
    }
  } else {
    server.handleClient();

    if (millis() - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = millis();
      if (connectWiFi()) {
        //ESP.restart();  // reset lại để kích hoạt Firebase
      }
    }
    // Check update FirmWare
    if (wifiConnected && millis() - lastCheckTime > CHECK_INTERVAL) {
      lastCheckTime = millis();
      checkAndUpdateFirmware();
    }
  }
}
