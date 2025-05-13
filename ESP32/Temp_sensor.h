// Định nghĩa chân đọc cảm biến
#define NTC1_PIN 34
#define NTC2_PIN 35

// Thông số NTC và điện trở cầu phân áp
const float seriesResistor = 10000.0;     // điện trở kéo 10k
const float nominalResistance = 10000.0;  // giá trị NTC tại 25 độ C
const float nominalTemperature = 25.0;    // nhiệt độ danh định (C)
const float bCoefficient = 3950.0;        // hệ số B của NTC
const int adcMax = 4095;                  // độ phân giải ADC của ESP32
const float vcc = 3.3;                    // điện áp tham chiếu

// Chuyển đổi giá trị analog -> nhiệt độ theo công thức cầu phân áp
float readNTCTemperature(int analogPin) {
  int adcValue = analogRead(analogPin);  // đọc giá trị ADC
  float voltage = adcValue * vcc / adcMax;

  float resistance = (vcc * seriesResistor / voltage) - seriesResistor;

  float steinhart;
  steinhart = resistance / nominalResistance;        // (R/Ro)
  steinhart = log(steinhart);                        // ln(R/Ro)
  steinhart /= bCoefficient;                         // 1/B * ln(R/Ro)
  steinhart += 1.0 / (nominalTemperature + 273.15);  // + (1/To)
  steinhart = 1.0 / steinhart;                       // Invert
  steinhart -= 273.15;                               // Convert to °C

  return steinhart;
}


