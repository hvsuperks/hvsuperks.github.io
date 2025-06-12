/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete instructions at https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
*********/
#include <WiFi.h>
#include "config.h"
#include "Input_var.h"

void Scan_wifi(){    // Ngắt kết nối để quét được tất cả Wi-Fi
  delay(100);  
  int n = WiFi.scanNetworks();
  Serial.print("Tìm thấy "); Serial.println(n);

  String json = "<option value=\"" + ssid + "\">" + ssid + "</option>";
  for (int i = 0; i < n; i++) {
    json += "<option value=\"";
    json += WiFi.SSID(i);
    json += "\">" + WiFi.SSID(i) + "</option>";;
  }
    json_scan_wifi = json;
}

bool testwifi() {
  int c = 0;
  while (c < 80) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wifi connected");
      delay(1000);
      return true;
    }
    delay(200);
    Serial.print(".");
    c++;
  }
  Serial.println("Wifi NG");
  return false;
}

IPAddress dns1(8, 8, 8, 8);
bool wifi_setup(String s, String p){
  WiFi.mode(WIFI_STA);   // Bắt buộc
  WiFi.disconnect();     // Ngắt kết nối để quét được tất cả Wi-Fi
  delay(100);   
  if(WiFi.status() != WL_CONNECTED){
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns1 );
    WiFi.begin(s.c_str(), p.c_str());
    if(testwifi()){
      Serial.print("Connecting to Wi-Fi");
      WiFi.setAutoReconnect(true);
      Serial.println("Wifi thành công");
      return true;
    }else {
      return false;
    }
  }else{
    return true;
  }
}
void Wifi_AP(){
  // Đợi 1 chút cho ổn định
  WiFi.scanDelete(); // Xoá kết quả cũ
  WiFi.mode(WIFI_STA);   // Bắt buộc
  WiFi.disconnect(); 
  Scan_wifi();
  WiFi.softAP("NTAnh");
  Serial.println("Mở Wifi AP NTAnh");
  Serial.println(WiFi.softAPIP());
}


  