#include "Input_var.h"
#include "action.h"

long last_mini_chiller;
Ticker ticker_chiller, ticker_co2;
void ac_chiller(){
    temp_read();
    bool tmp = temp_tank < 35 && temp_tank > 15;
    if(!tmp) { // Nếu cảm biến nhiệt độ hỏng
        digitalWrite(pin_chiller,1); // tắt Chiller
    }
    if(!chiller_manual || digitalRead(pin_fillter)) return;
    if((temp_tank > (temp_set + temp_diff) && digitalRead(pin_chiller)) && tmp){
        digitalWrite(pin_chiller,0);
        last_chiller = millis();
        last_mini_chiller = last_chiller;
        chiller_manual = 0;
        ticker_chiller.once(120,[](){chiller_manual = 1;});
    }
    if((temp_tank < (temp_set - temp_diff) || tmp) && !digitalRead(pin_chiller)){
        digitalWrite(pin_chiller, 1);
        time_chiller += last_chiller;
        chiller_manual = 0;
        ticker_chiller.once(120,[](){chiller_manual = 1;});
    }
}

void ac_co2(){
    bool relay_co2;
    int i = h*60 +m;
    if(!co2_manual || digitalRead(pin_fillter)) return;
    relay_co2 = (i > co2_on_1 && i < co2_off_1) || (i > co2_on_2 && i < co2_off_2);
    if(relay_co2 && digitalRead(pin_co2)){
        digitalWrite(pin_co2, 0);
        chiller_manual = 0;
        ticker_co2.once(60,[](){co2_manual = 1;});
    }else if((!relay_co2 && !digitalRead(pin_co2))){
        digitalWrite(pin_co2, 1);
        chiller_manual = 0;
        ticker_co2.once(60,[](){co2_manual = 1;});
    }
}

bool water_check = true;
void waterFlag(){
    water_check = false;
    ticker_water.once(10800, [](){water_check = true;});
}

void ac_water(){
    if(!water_manual) return;
    int i = h*60 + m;
    bool relay_water = (i > 22*60) && (i < 23*60+59) && (thu == 3 || thu == 0);
    if ((relay_water  && digitalRead(pin_water) && water_check) && water_manual){
        digitalWrite(pin_water,0);
        ticker_water.once(water_volume/(water_speed/60),[](){water_manual = 1;});
    }else if(!digitalRead(pin_water) && water_check && water_manual){
        digitalWrite(pin_water,1);
    }
}