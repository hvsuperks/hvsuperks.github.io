#include <IPAddress.h>
//#include <FirebaseJson.h>
#include <Arduino.h> 
#include <Firebase_ESP_Client.h>
#include "firebase.h"
#include "Input_var.h"
#include <Ticker.h>
void streamCallback(FirebaseStream data);
void streamTimeoutCallback(bool timeout);

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData stream_setting, stream_relay, stream_pin;

FirebaseJsonData result;
FirebaseData fbdo_del;  


enum VarType {
    TYPE_FLOAT,
    TYPE_INT
};
struct ParamEntry {
    const char* key;
    VarType type;
    union {
        float* fptr;
        int* iptr;
    } variable;
};
ParamEntry params[] = { // Khai báo mảng
    { "/temp_set",     TYPE_FLOAT, { .fptr = &temp_set } },
    { "/temp_diff",    TYPE_FLOAT, { .fptr = &temp_diff } },
    { "/temp_offset",  TYPE_FLOAT, { .fptr = &temp_offset } },
    { "/co2_on_1",     TYPE_INT,   { .iptr = &co2_on_1 } },
    { "/co2_on_2",     TYPE_INT,   { .iptr = &co2_on_2 } },
    { "/co2_off_1",    TYPE_INT,   { .iptr = &co2_off_1 } },
    { "/co2_off_2",    TYPE_INT,   { .iptr = &co2_off_2 } },
    { "/water_day_1",  TYPE_INT,   { .iptr = &water_day_1 } },
    { "/water_day_2",  TYPE_INT,   { .iptr = &water_day_2 } },
    { "/water_volume", TYPE_INT,   { .iptr = &water_volume } },
    { "/water_speed",  TYPE_INT,   { .iptr = &water_speed } },
};

void upload_temp(){
    char i[15];  // hh:mm:ss + null terminator
    sprintf(i, "%02d/%02d %02d:%02d:%02d", thang, ngay, h, m, s);
    FirebaseJson jsons;
    jsons.set("Time",i);
    jsons.set("Temp_tank", temp_tank);
    jsons.set("relay_co2", digitalRead(pin_co2)? "OFF":"ON");
    jsons.set("relay_fillter", digitalRead(pin_fillter)? "OFF":"ON");
    jsons.set("relay_chiller", digitalRead(pin_chiller)? "OFF":"ON");
    jsons.set("relay_water", digitalRead(pin_water)? "OFF":"ON");
    jsons.set("pin_fillter", pin_fillter);
    jsons.set("pin_chiller", pin_chiller);
    jsons.set("pin_co2", pin_co2);
    jsons.set("pin_water", pin_water);

    Firebase.RTDB.setJSON(&fbdo, "Status", &jsons);
    delay(500);
    jsons.clear();
}

void relayChiller(bool tt){
    if(digitalRead(pin_fillter) && !tt) return;
    digitalWrite(pin_chiller, tt);
    chiller_manual = 0;
    ticker_chiller.once(300,[](){chiller_manual = 1;});
    Firebase.RTDB.deleteNode(&fbdo_del, "/control/relay_chiller");
}

void relayCo2(bool tt){
    if(digitalRead(pin_fillter) && !tt) return;
    digitalWrite(pin_co2, tt) ;
    co2_manual = 0;
    ticker_co2.once(600,[](){co2_manual = 1;});
    Firebase.RTDB.deleteNode(&fbdo_del, "/control/relay_co2");
}

void relayFillter(bool tt){
    digitalWrite(pin_fillter, tt);
    Serial.println("trạng thái Fillter : " + String(digitalRead(pin_fillter)));
    Firebase.RTDB.deleteNode(&fbdo_del, "/control/relay_fillter");
}

void relayWater(bool tt){
    digitalWrite(pin_water, tt);
    water_manual = 0;
    Serial.println("đến đây rồi - relayWater ----------------");
    Serial.println("water_volume : " + String(water_volume));
    Serial.println("water_speed : " + String(water_speed));
    ticker_water.once(water_volume/water_speed*60,[](){
        digitalWrite(pin_water, 1);
        water_manual = 1;
    });
    Firebase.RTDB.deleteNode(&fbdo_del, "/control/relay_water");
}

void setup_firebase(){
    config.api_key = Web_API_KEY_.c_str();
    config.database_url = DATABASE_URL_.c_str();
    auth.user.email = USER_EMAIL_.c_str();
    auth.user.password = USER_PASS_.c_str();

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    char i[15];  // hh:mm:ss + null terminator
    sprintf(i, "%02d/%02d %02d:%02d:%02d", thang, ngay, h, m, s);
    Serial.println("Start time : " + String(i));
    if (Firebase.RTDB.setString(&fbdo, "Start", i)) {
        Serial.println("Data sent successfully");
    } else {
        Serial.println("Failed to send data");
        Serial.println(fbdo.errorReason());
    }
    // Đăng ký lắng nghe đường dẫn /test/data
    if (!Firebase.RTDB.beginStream(&stream_setting, "/Setting")) {
        Serial.printf("Stream bắt đầu thất bại: %s\n", stream_setting.errorReason().c_str());
    } else {
        Serial.println("Stream bắt đầu thành công");
    }

    if (!Firebase.RTDB.beginStream(&stream_relay, "/control")) {
        Serial.printf("Stream bắt đầu thất bại: %s\n", stream_relay.errorReason().c_str());
    } else {
        Serial.println("Stream bắt đầu thành công");
    }

    if (!Firebase.RTDB.beginStream(&stream_pin, "/Pin_set")) {
        Serial.printf("Stream bắt đầu thất bại: %s\n", stream_pin.errorReason().c_str());
    } else {
        Serial.println("Stream bắt đầu thành công");
    }
}

void firebase_loop()
{
    Firebase.RTDB.readStream(&stream_setting);
    Firebase.RTDB.readStream(&stream_relay);
    Firebase.RTDB.readStream(&stream_pin);
    if (stream_setting.streamTimeout()) {
        Serial.println("stream_setting timeout, đang kết nối lại...");
    }
    if (stream_relay.streamTimeout()) {
        Serial.println("stream_relay timeout, đang kết nối lại...");
    }
    if (stream_pin.streamTimeout()) {
        Serial.println("stream_relay timeout, đang kết nối lại...");
    }

    if (stream_setting.streamAvailable()) {
        Serial.println("Có dữ liệu thay đổi:");
        Serial.print(String("Đường dẫn: "));
        Serial.println(stream_setting.dataPath());
        Serial.print("Giá trị mới: ");
        if (stream_setting.dataPath() == "/") {
            FirebaseJson *json = stream_setting.to<FirebaseJson*>();
            eeprom.begin("Setting",false);
            for (const auto& p : params) {
                String i = p.key;
                i = i.substring(1);
                json->get(result, i);
                Serial.print(i);
                Serial.print(" : ");
                if(p.type == TYPE_FLOAT){
                    *p.variable.fptr = result.to<float>();
                    eeprom.putFloat(i.c_str(), *p.variable.fptr);
                    Serial.println(*p.variable.fptr);
                } else if(p.type == TYPE_INT) {
                    *p.variable.iptr = result.to<int>();
                    Serial.println(*p.variable.iptr);
                    eeprom.putFloat(i.c_str(), *p.variable.iptr);
                }
            }
            json->clear();
            eeprom.end();
        } else {
            for (const auto& p : params) {
                if(stream_setting.dataPath() == p.key){
                    Serial.print(p.key);
                    Serial.print(" : ");
                    String i = p.key;
                    i = i.substring(1);
                    eeprom.begin("Setting",false);
                    if(p.type == TYPE_FLOAT){
                        *p.variable.fptr = stream_setting.floatData();
                        Serial.println(*p.variable.fptr);
                        eeprom.putFloat(i.c_str(), *p.variable.fptr);
                    } else if(p.type == TYPE_INT) {
                        *p.variable.iptr = stream_setting.intData();
                        Serial.println(*p.variable.iptr);
                        eeprom.putInt(i.c_str(), *p.variable.iptr);
                    }
                    eeprom.end();
                    return;
                }
            }
        }
    }
    
    
    if (stream_relay.streamAvailable()) {
        Serial.println("stream_relay:");
        Serial.print(String("Đường dẫn: "));
        Serial.println("type : " + String(stream_relay.dataType()));
        String i = stream_relay.dataPath();
        String i_del = "/control" + i;
        struct relay{
            const char* key;
            int* var;
        };
        relay list[]
        {
            {"relay_chiller" , &pin_chiller},
            {"relay_co2", &pin_co2},
            {"relay_fillter", &pin_fillter},
            {"relay_water", &pin_water}
        };

        if (stream_relay.dataType() != "json"){
            bool tt = stream_relay.boolData();
            Serial.println(i);
            Serial.println("Giá trị mới: " + String(tt));
            Serial.println("type : " + String(stream_relay.dataType()));
            if(i == "/relay_chiiler"){
                relayChiller(tt);
            }else if(i == "/relay_co2"){
                relayCo2(tt);
            }else if(i == "/relay_fillter"){
                relayFillter(tt);
            }else if(i == "/relay_water"){
                relayWater(tt);
            }
            
        }else if (i == "/") {
            FirebaseJson *json = stream_relay.to<FirebaseJson*>();
            for (const auto p: list) {
                bool data;
                FirebaseJsonData result;
                if(json->get(result, p.key)){
                    if(p.key == "relay_chiller"){
                        relayChiller(result.to<bool>());
                    }else if(p.key == "relay_co2"){
                        relayCo2(result.to<bool>());
                    }else if(p.key == "relay_fillter"){
                        relayFillter(result.to<bool>());
                    }else if(p.key == "relay_water"){
                        relayWater(result.to<bool>());
                    }
                }
                result.clear();
            }
            json->clear();
        }
        
    }
        
    if (stream_pin.streamAvailable()) {
        Serial.println("Có dữ liệu thay đổi:");
        Serial.print(String("Đường dẫn: "));
        Serial.println(stream_pin.dataPath());
        Serial.print("Giá trị mới: " + String(stream_pin.intData()));
        String i = stream_pin.dataPath();
        i = i.substring(1);
        eeprom.begin("loadpin",false);
        eeprom.putInt(i.c_str(), stream_pin.intData());
        eeprom.end();
        loadpin();
    }
    delay(100);
}


