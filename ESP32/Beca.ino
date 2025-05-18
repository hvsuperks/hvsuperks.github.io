void(* resetFunc) (void) = 0; //reset
#include <WiFi.h>
#include <WebServer.h>
#include <Ticker.h>
#include <Update.h>
#include <Preferences.h>
#include <time.h>
#include <Firebase_ESP_Client.h>
//#include "update_firmware.h"

//------------------ Khai báo biến ..............//

String ssid = ""; //----- Wifi --------//
String pass = ""; //----- Wifi --------//

//---------- Khai báo các chân PIN ----------//
#define Pin_temp_tank 34 //----- Sensor nhiệt NTC của bể -----//
#define Pin_temp_room 35 //----- Sensor nhiệt NTC Phòng -----//
#define RELAY_Chiller 23
#define RELAY_Filter 22
#define RELAY_Co2 21
#define RELAY_Water 19
#define off 1
#define on 0

Preferences eeprom; //----- Khai báo biến lưu eeprom -----//
WebServer server(80); //----- Khai báo WebSever -----//
Ticker Ticker; //----- Khai báo bộ hẹn giờ -----//

//---------- Khai báo các biến đọc FireBase JSON ----------//
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseData stream;
FirebaseConfig config;
#define API_KEY = "AIzaSyAOOtP5Va7zCa9lH62H20piGYCvdACt9DE";
#define DATABASE_URL = "https://be-ca-ad623-default-rtdb.asia-southeast1.firebasedatabase.app";

//---------- Khai báo Version FirmWare và Sever Update FirmWare ----------//
const String currentVersion = "v1.0.0";  // Cập nhật thủ công trước khi build
const String versionURL = "https://raw.githubusercontent.com/yourusername/esp32-ota/main/version.txt";
const String firmwareURL = "https://raw.githubusercontent.com/yourusername/esp32-ota/main/firmware.bin";

//---------- Khai báo các biến Setting ----------//
float temp_set, temp_diff, temp_offset, temp_tank, temp_room; // Khai báo các biến nhiệt độ cài đặt
int filter_pause, co2_on_1,co2_on_2, co2_off_1, co2_off_2, water_day, water_volume, water_speed; // Khai báo các biến liên quan thời gian

//---------- Khai báo biến thời gian ----------//
int h, m, s, thu, ngay, thang;
struct tm timeinfo;

//---------- Đọc các giá trị Setting trong EEPROM ----------//
void loadSetting(){
  eeprom.begin("wifi", true);
  ssid = eeprom.getString("ssid", "");
  pass = eeprom.getString("pass", "");
  eeprom.end();

  eeprom.begin("setting", false);
  eeprom.getFloat("temp_set",28);
  eeprom.getFloat("temp_diff",3);
  eeprom.getFloat("temp_offset",3);
  eeprom.getInt("filter_pause",10);
  eeprom.getInt("co2_on_1",300);
  eeprom.getInt("co2_on_2",1020);
  eeprom.getInt("co2_off_1",600);
  eeprom.getInt("co2_off_2",1320);
  eeprom.getInt("water_day",1);
  eeprom.getInt("water_volume",30);
  eeprom.getInt("water_speed",1320);
  eeprom.end();
}

// Ghi lại WiFi config
void saveWiFiConfig(String s, String p) {
  eeprom.begin("wifi", false);
  eeprom.putString("ssid", s);
  eeprom.putString("pass", p);
  eeprom.end();
}

FirebaseData stream_setting;
FirebaseData stream_relay;

//----------- Xử lý sự kiện thay đổi Json ----------//
void onDataChanged(String path, String value) {
  Serial.println("🔥 Dữ liệu Firebase thay đổi:");
  Serial.println("Path: " + path);
  Serial.println("Giá trị mới: " + value);

  // Thực hiện hành động gì đó ở đây...
}

//---------- Lấy Data khi Json thay đổi ----------//
void stream_setting_Callback(FirebaseStream data) {
  String path = data.dataPath();
  String value = data.stringData();
  if (path == "/setting/temp_set") {
    temp_set = value.toFloat();
  } else if (path == "/setting/temp_diff") {
    temp_diff = value.toFloat();
  }else if (path == "/setting/temp_offset") {
    temp_offset = value.toFloat();
  }else if (path == "/setting/filter_pause") {
    filter_pause = value.toInt();
  }else if (path == "/setting/co2_on_1") {
    co2_on_1 = value.toInt();
  }else if (path == "/setting/co2_on_2") {
    co2_on_2 = value.toInt();
  }else if (path == "/setting/co2_off_1") {
    co2_off_1 = value.toInt();
  }else if (path == "/setting/co2_off_2") {
    co2_off_2 = value.toInt();
  }else if (path == "/setting/water_day") {
    water_day = value.toInt();
  }else if (path == "/setting/water_volume") {
    water_volume = value.toInt();
  }else if (path == "/setting/water_speed") {
    water_speed = value.toInt();
  }

}

void stream_relay_Callback(FirebaseStream data) {
  String path = data.dataPath();
  if (data.stringData() == "ON"){
    value = on;
  }else{
    value = off;
  }
  if (path == "/relay/chiller") {
    digitalWrite(RELAY_Chiller, value);
  }else if(path == "/relay/fillter"){
    digitalWrite(RELAY_Fillter, value);
    if (value == off){
      digitalWrite(RELAY_Co2, off);
      digitalWrite(RELAY_Chiller, off);
    }
  }
}

//---------- Kiểm tra kết nối Stream Json ----------//
void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("⏳ Firebase stream timeout, đang kết nối lại...");
  }
}

//---------- Config Sever Firebase ----------//
void setupFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = "HVsuperKS@gmail.com";
  auth.user.password = "SAObang!((#";
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
    // Bắt đầu stream ở đường dẫn bạn muốn theo dõi
  Firebase.RTDB.beginStream(&stream_setting, "/setting")
  Firebase.RTDB.setStreamCallback(&stream_setting, stream_setting_Callback, streamTimeoutCallback);
  Firebase.RTDB.beginStream(&stream_relay, "/relay")
  Firebase.RTDB.setStreamCallback(&stream_relay, stream_relay_Callback, streamTimeoutCallback);
}


//---------- Đồng bộ thời gian ----------//
void updateTime() {
  if (WiFi.status() == WL_CONNECTED) {
    configTime(7 * 3600, 0, "pool.ntp.org");  // GMT+7
  }
}

//---------- Kiểm tra kết nối Internet ----------//
bool testwifi(){
  int c = 0;
  while ( c < 80 )       
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Wifi connected");
      IPAddress ip = WiFi.localIP();
      IPAddress sub = WiFi.subnetMask();
      IPAddress defau = WiFi.gatewayIP();
      IPAddress ipnew(ip[0],ip[1],ip[2],200);
      WiFi.config(ipnew,defau,sub);
      return true;
    }
    delay(200);
    c++;
    Serial.print(".");
  }
    Serial.println("Wifi NG");
    return false;
}

//---------- Setup Wifi ---------- //
void Setup_Wifi(){
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  if(testwifi()){
    updateTime();
    while (!getLocalTime(&timeinfo)) {
      Serial.println("Đang đồng bộ thời gian...");
      delay(1000);
    }
    WiFi.setAutoReconnect(true);
    setupFirebase();
    //setupOTA();
    Ticker.attach(60,updateTime);
  }else{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.softAP("NTAnh");
    Serial.println("Mở Wifi NTAnh");
    Ticker.once(600,Setup_Wifi);
  }
}

//---------- Đọc giá trị nhiệt độ ----------//
float Read_temp(int analogPin) {
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

//---------- Cài đặt WebSever ----------//
void Web_sever(){
  server.on("/",[]{
    Serial.println("Yêu cầu Web");
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200,"text/html","");

    server.client().stop();
    Serial.println("Seen");
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
  sever.begin();
  Serial.println("Web Start");
}

//---------- Lấy thời gian hệ thống mỗi 1s ----------//
void get_Time(){
  if(getLocalTime(&timeinfo)){
    int h = timeinfo.tm_hour;
    int m = timeinfo.tm_min;
    int s = timeinfo.tm_sec;
    int thu = timeinfo.tm_wday;      // 0 = Chủ Nhật, 1 = Thứ 2, ..., 6 = Thứ 7
    int ngay = timeinfo.tm_mday;     // 1 - 31
    int thang = timeinfo.tm_mon + 1; // 0 - 11 → cộng 1 thành 1 - 12
  }
}

//---------- Khai báo các chân Inout vs Trạng thái ----------//
void Setup_pin(){
  pinMode(Pin_temp_tank,INPUT);
  pinMode(Pin_temp_room,INPUT);
  pinMode(RELAY_Chiller,OUTPUT);
  pinMode(RELAY_Filter,OUTPUT);
  pinMode(RELAY_Co2,OUTPUT);
  pinMode(RELAY_Water,OUTPUT);
  digitalWrite(RELAY_Chiller,off);
  digitalWrite(RELAY_Filter,off);
  digitalWrite(RELAY_Co2,off);
  digitalWrite(RELAY_Water,off);
}

void temp_rt(){
  temp_tank = Read_temp(Pin_temp_tank) - temp_offset_tank;
  temp_room = Read_temp(Pin_temp_room) temp_offset_room;
}

int Chiller_last_on;
int Chiller_time_today;

//---------- Điều khiển Chiller ----------//
void Chiller(){
  if(digitalRead(RELAY_Chiller) == off) {
    if((temp_tank > (temp_set + temp_diff)) && (digitalRead(RELAY_Fillter) == on)){
      digitalWrite(RELAY_Chiller, on);
      Chiller_last_on = millis();
      Firebase.RTDB.setString(&fbdo, "Status/Chiller", "ON");
    }
  }else{
    if(temp_tank < (temp_set - temp_diff)){
      digitalWrite(RELAY_Chiller, off);
      Chiller_time_today += ((millis() - Chiller_last_on)/60);
      Firebase.RTDB.setInt(&fbdo, "time/Chiller_today", Chiller_time_today);
      Firebase.RTDB.setString(&fbdo, "Status/Chiller", "OFF");
    }
  }

}

//---------- Điều khiển lọc ----------//
void Fillter(){
  digitalWrite(RELAY_Fillter, on);
  Firebase.RTDB.setString(&fbdo, "Status/Fillter", "ON");
}


bool dk_co2 = 1;

//---------- Điều khiển Co2 ----------//
void Co2(){
  if(digitalRead(RELAY_Fillter) == "ON"){
    if(dk_co2){
      if((h*60 + m >= co2_on_1) && if(h*60 + m <= co2_off_1)) || ((h*60 + m >= co2_on_2) && if(h*60 + m <= co2_off_2)) {
        digitalWrite(RELAY_Co2, on);
        Firebase.RTDB.setString(&fbdo, "Status/Co2", "ON");
      }else{
        digitalWrite(RELAY_Co2, off);
        Firebase.RTDB.setString(&fbdo, "Status/Co2", "OFF");
      }
    }
  }else{
    digitalWrite(RELAY_Co2, off)
    Firebase.RTDB.setString(&fbdo, "Status/Co2", "OFF");
  }
}



//------------------------------------------------------- Setup ----------------------------------------------//
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("start");
  Setup_pin();
  loadSetting();
  Setup_Wifi();
  Web_sever();
  Ticker.attach(1,get_Time); //----- Đọc thời gian hệ thống mỗi 1s -----//
  Ticker.attach(10,Temp_rt); //----- Đọc nhiệt độ mỗi 10s -----//
}


void loop() {

  server.handleClient();
  
  }
