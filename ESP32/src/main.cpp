#include <Arduino.h> 
#include <WiFi.h>
#include "config.h"
#include "get_time.h"
#include "firebase.h"
#include "Input_var.h"
#include "web_server.h"
#include "action.h"

Ticker ticker_wifi;
Ticker ticker_action;


bool wifi_state = false;
bool action_state = false;
bool temp_state = false;

void config(){
    if(wifi_setup(ssid, pass)){
        delay(1000);
        updateTime();
        delay(1000);
        Serial.printf("🕒 Thời gian: %02d:%02d:%02d - %02d/%02d/%04d\n",
            timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec,
            timeinfo.tm_mday,
            timeinfo.tm_mon + 1,
            timeinfo.tm_year + 1900);
        setup_firebase();
    }else{
        ticker_wifi.once(600, [](){wifi_state = true;});
        Wifi_AP();
    }
}

void setup()
{
    Serial.begin(115200);
    loadpin();
        digitalWrite(pin_fillter, 0);
        digitalWrite(pin_chiller, 1);
        digitalWrite(pin_co2, 0);
        digitalWrite(pin_water, 1);
    loadConfig();
    loadSetting();
    config();
    Web_sever();
    ticker_action.attach(5,[](){action_state = true;});
}

bool fillter = 1;
int last_fillter;
void loop()
{
    firebase_loop();
    server_handleClient();
    if(wifi_state){
        config();
        wifi_state = false;
    }
    if(action_state){
        get_Time();
        ac_chiller();
        ac_co2();
        ac_water();
        upload_temp();
        action_state = false;
    }
    
    if(fillter && digitalRead(pin_fillter)){
        fillter = 0;
        last_fillter = h*60 + m;
    }else if(((h*60+m) - last_fillter > fillter_pause) && digitalRead(pin_fillter)){
        fillter = 1;
        digitalWrite(pin_fillter,0);
    }
    delay(50);
}

