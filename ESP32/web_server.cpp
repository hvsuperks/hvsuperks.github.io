#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include "Input_var.h"
#include "web_server.h"
#include "config.h"
#include "firebase.h"

WebServer server(80);

void handleScan() {
  Scan_wifi();
  server.send(200, "text/plain", json_scan_wifi);
}


void handleRelayStatus() {
  String j = "{";
  j += "\"fillter\":" + String(digitalRead(pin_fillter)) + ",";
  j += "\"chiller\":" + String(digitalRead(pin_chiller)) + ",";
  j += "\"co2\":" + String(digitalRead(pin_co2)) + ",";
  j += "\"water\":" + String(digitalRead(pin_water));
  j += "}";
  server.send(200, "application/json", j);
}

void handleRelay() {
  if (server.hasArg("name")) {
    String name = server.arg("name");
    if (name == "fillter") relayFillter(!digitalRead(pin_fillter));
    else if (name == "chiller") relayChiller(!digitalRead(pin_chiller));
    else if (name == "co2") relayCo2(!digitalRead(pin_co2));
    else if (name == "water") relayWater(!digitalRead(pin_water));
  }
  handleRelayStatus();
}


void Web_sever() {
    server.on("/", HTTP_GET, []() {
    String html = R"rawliteral(
    <html><head>
    <meta charset='UTF-8'><title>ESP32 Setup</title>
    <style>
      input {
        width: 550px;
        padding: 4px 8px;
        font-size: 16px;
        border: 1px solid #ccc;
        border-radius: 4px;
        box-sizing: border-box;
        margin-bottom: 8px;
        display: block;
      }

      label {
        display: inline-block;
        width: 100px;
        font-weight: bold;
        margin-bottom: 4px;
      }

      .form-group {
        margin-bottom: 12px;
      }
          .relay-button {
      padding: 10px 20px;
      margin: 10px;
      font-size: 16px;
      

      .relay-status {
        display: inline-block;
        width: 20px;
        height: 20px;
        border-radius: 50%;
        margin-left: 10px;
      }

      .on { background-color: green; }
      .off { background-color: red; }

    </style>

    </head>
    <body>
    <h2>Wi-Fi Config</h2>
    <form id='wifiForm' action='/wifi' method='POST'>
      SSID: <input type="text" list="ssidList" id='ssid' name='ssid' value=')rawliteral";
      html += ssid;
      html += "'><br><datalist id='ssidList'>";
      html += json_scan_wifi;
      html += "</datalist>Mật khẩu: <input type='text' name='pass' value='";
      html += pass ;
      html += R"rawliteral('><br>
      <input type='submit' value='Lưu Wi-Fi'>
      <button onclick="scanWifi()">scanWifi</button>
    </form>
    <h2>Firebase Config</h2>
    <form id='firebaseForm' action='/firebase' method='POST'>
      API KEY: <input name='auth' value=')rawliteral" ;
      html += Web_API_KEY_ ;
      html += "'><br> Data Base URL: <input name='path' value='" ;
      html += DATABASE_URL_ ;
      html += "'><br>Email: <input name='email_' value='" ;
      html += USER_EMAIL_ ;
      html += "'><br>Pass: <input name='pass_' value='" ;
      html += USER_PASS_ ;
      html += R"rawliteral('><br>
      <input type='submit' value='Lưu Firebase'>
    </form>
      <h2>Điều khiển Relay</h2>
      <button class="relay-button" onclick="toggleRelay('fillter')">Fillter</button>
      <span id="status-fillter" class="relay-status"></span><br>

      <button class="relay-button" onclick="toggleRelay('chiller')">Chiller</button>
      <span id="status-chiller" class="relay-status"></span><br>

      <button class="relay-button" onclick="toggleRelay('co2')">CO₂</button>
      <span id="status-co2" class="relay-status"></span><br>

      <button class="relay-button" onclick="toggleRelay('water')">Water</button>
      <span id="status-water" class="relay-status"></span><br>
    <script>
      function scanWifi() {
      fetch("/scan")
        .then(res => res.text())
        .then(data => {
          document.getElementById("ssidList").innerHTML = data;
        })
        .catch(err => console.error("Lỗi:", err));
    }
      function toggleRelay(name) {
      fetch("/relay?name=" + name)
        .then(response => response.json())
        .then(data => {
          for (const key in data) {
            const el = document.getElementById("status-" + key);
            el.className = "relay-status " + (data[key] === 0 ? "on" : "off");
          }
        });
    }

    window.onload = () => {
      fetch("/relay_status")
        .then(res => res.json())
        .then(data => {
          for (const key in data) {
            const el = document.getElementById("status-" + key);
            el.className = "relay-status " + (data[key] === 0 ? "on" : "off");
          }
        });
    };

    </script>
    </body></html>)rawliteral";

    server.send(200, "text/html", html);
  });

  server.on("/wifi", HTTP_POST, []() {
    ssid = server.arg("ssid");
    pass = server.arg("pass");
    eeprom.begin("config_wifi", false);
    eeprom.putString("ssid", ssid);
    eeprom.putString("pass", pass);
    eeprom.end();

    server.send(200, "text/html", "Đã lưu Wi-Fi. <a href='/'>Quay lại</a>");
    wifi_state = true;  // Gọi lại hàm kết nối Wi-Fi
  });

  server.on("/firebase", HTTP_POST, []() {
    Web_API_KEY_ = server.arg("auth");
    DATABASE_URL_ = server.arg("path");
    USER_EMAIL_ = server.arg("email_");
    USER_PASS_ = server.arg("pass_");

    eeprom.begin("config_wifi", false);
    eeprom.putString("API_KEY", Web_API_KEY_);
    eeprom.putString("DATABASE_URL", DATABASE_URL_);
    eeprom.putString("email_", USER_EMAIL_);
    eeprom.putString("pass_", USER_PASS_);
    eeprom.end();
    
    server.send(200, "text/html", "Đã lưu Firebase. <a href='/'>Quay lại</a>");
  });

  server.on("/update", HTTP_POST,[]() {
    if (Update.hasError()) {
      server.send(500, "text/plain", "Update Failed");
    } else {
      server.send(200, "text/plain", "Update Success. Rebooting...");
      delay(100);
      ESP.restart();
    }
    },[]() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update Start: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Update Success: %u bytes\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    }
  );

  server.on("/scan", handleScan);
  server.on("/relay", handleRelay);
  server.on("/relay_status", handleRelayStatus);
  server.begin();
  Serial.println("Web Start");
}


void server_handleClient(){
server.handleClient();
}