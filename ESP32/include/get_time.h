#pragma once
#include <time.h>
#include <Arduino.h> 

//----- Get Time -----//
extern struct tm timeinfo;
extern int h, m, s, thu, ngay, thang;
void get_Time();
void updateTime();