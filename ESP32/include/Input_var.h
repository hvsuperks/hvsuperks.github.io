#pragma once
#include <Arduino.h>
#include <Ticker.h>
#include <Preferences.h>

#define Web_API_KEY "AIzaSyAOOtP5Va7zCa9lH62H20piGYCvdACt9DE"
#define DATABASE_URL "https://be-ca-ad623-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL "hvsuperks@gmail.com"
#define USER_PASS "SAObang!((#"
#define WIFI_SSID "Nguyễn Bố Anh Duy"
#define WIFI_PASSWORD "12345678"

extern int pin_co2, pin_fillter, pin_chiller, pin_water, pin_temp_tank;
extern float temp_set, temp_diff, temp_offset,temp_tank;
extern int fillter_pause, co2_on_1, co2_on_2, co2_off_1, co2_off_2;
extern int water_day_1, water_day_2, water_volume, water_speed;
extern String Web_API_KEY_, DATABASE_URL_, USER_EMAIL_, USER_PASS_;
extern long last_chiller, time_chiller;
extern String ssid;
extern String pass;

extern Ticker ticker, ticker_water, ticker_fillter, ticker_chiller, ticker_co2;;
extern Preferences eeprom;
extern int h;
extern int m;
extern int s;
extern int thu;
extern int ngay;
extern int thang;
extern String json_scan_wifi;

extern bool chiller_manual, co2_manual, water_manual, wifi_state;
void loadConfig();
void loadSetting();
void loadpin();
void temp_read();
void Scan_wifi();