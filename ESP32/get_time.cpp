#include "get_time.h"

struct tm timeinfo;
int h, m, s, thu, ngay, thang;

void get_Time() {
  if (getLocalTime(&timeinfo)) {
    h = timeinfo.tm_hour;
    m = timeinfo.tm_min;
    s = timeinfo.tm_sec;
    thu = timeinfo.tm_wday;       // 0 = Chủ Nhật, 1 = Thứ 2, ..., 6 = Thứ 7
    ngay = timeinfo.tm_mday;      // 1 - 31
    thang = timeinfo.tm_mon + 1;  // 0 - 11 → cộng 1 thành 1 - 12
  }
}

//---------- Đồng bộ thời gian ----------//
void updateTime() {
    configTime(7 * 3600, 0, "time1.nimt.gov.vn"	,"ntp.vnn.vn","pool.ntp.org" 		);  // GMT+7
    int retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 20) {
    Serial.print(".");
    delay(100);  // mỗi lần chờ 100ms
    retry++;
    }
    if (retry >= 20) {
        Serial.println("\n⛔️ Lỗi: Không thể đồng bộ thời gian!");
        ESP.restart();
    } else {
        Serial.println("\n✅ Đồng bộ thời gian thành công:");
        Serial.printf("Giờ: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        h = timeinfo.tm_hour;
        m = timeinfo.tm_min;
        s = timeinfo.tm_sec;
        thu = timeinfo.tm_wday;       // 0 = Chủ Nhật, 1 = Thứ 2, ..., 6 = Thứ 7
        ngay = timeinfo.tm_mday;      // 1 - 31
        thang = timeinfo.tm_mon + 1;  // 0 - 11 → cộng 1 thành 1 - 12
    }

}