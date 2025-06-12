#include "Input_var.h"

Preferences eeprom;
Ticker ticker,ticker_water;

float temp_set, temp_diff, temp_offset, temp_tank;
int fillter_pause, co2_on_1, co2_on_2, co2_off_1, co2_off_2;
int water_day_1,water_day_2, water_volume, water_speed;
int  pin_fillter, pin_chiller, pin_water, pin_co2, pin_temp_tank;
String Web_API_KEY_, DATABASE_URL_, USER_EMAIL_, USER_PASS_;
long last_chiller, time_chiller;
String ssid;
String pass;
String json_scan_wifi;
bool chiller_manual = 1;
bool co2_manual = 1;
bool water_manual;


//---------- Đọc các giá trị Setting trong EEPROM ----------//
void loadConfig() {
  eeprom.begin("config_wifi", true);
  ssid = eeprom.getString("ssid", "Nguyễn Bố Anh Duy");
  pass = eeprom.getString("pass", "12345678");
  Web_API_KEY_ = eeprom.getString("API_KEY",Web_API_KEY);
  DATABASE_URL_ = eeprom.getString("DATABASE_URL",DATABASE_URL);
  USER_EMAIL_ = eeprom.getString("USER_EMAIL",USER_EMAIL);
  USER_PASS_ = eeprom.getString("USER_PASS",USER_PASS);
  eeprom.end();
}
void loadSetting(){
  eeprom.begin("Setting", true);
  temp_set = eeprom.getFloat("temp_set",28);
  temp_diff = eeprom.getFloat("temp_diff",2);
  temp_offset = eeprom.getFloat("temp_offset", 0);
  co2_on_1 = eeprom.getInt("co2_on_1", 300);
  co2_on_2 = eeprom.getInt("co2_on_2", 600);
  co2_off_1 = eeprom.getInt("co2_off_1", 1020);
  co2_off_2 = eeprom.getInt("co2_off_2", 1320);
  water_volume = eeprom.getInt("water_volume",30);
  water_speed = eeprom.getInt("water_speed",10);
  water_day_1 = eeprom.getInt("water_day_1",4);
  water_day_2 = eeprom.getInt("water_day_1",0);
  eeprom.end();
}

void loadpin(){
  eeprom.begin("loadpin", true);
  pin_fillter = eeprom.getInt("pin_fillter", 4);
  pin_chiller = eeprom.getInt("pin_chiller", 5);
  pin_water = eeprom.getInt("pin_water", 16);
  pin_co2 = eeprom.getInt("pin_co2", 17);
  pin_temp_tank = eeprom.getInt("pin_co2", 34);
  eeprom.end();

  pinMode(pin_fillter,OUTPUT);
  pinMode(pin_chiller,OUTPUT);
  pinMode(pin_water,OUTPUT);
  pinMode(pin_co2,OUTPUT);
  pinMode(pin_temp_tank,INPUT);
}

void temp_read() {
  int adcValue = analogRead(pin_temp_tank);
  float voltage = adcValue * 3.3 / 4095.0;
  float resistance = (3.3 * 10000.0 / voltage) - 10000.0;
  float steinhart;
  steinhart = resistance / 10000.0;
  steinhart = log(steinhart);
  steinhart /= 3950.0;
  steinhart += 1.0 / (25 + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;
  temp_tank = steinhart - temp_offset;
}
