#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TridentTD_LineNotify.h>
#include "DHT.h"
//เวลา
#include "time.h"

#define LINE_TOKEN "kbsprXYO2C4Se3iBBuPeyaReQuJkcXEwhuFOa6QOmmX"
BlynkTimer timer;
int pinValue;
#define BLYNK_PRINT Serial
#define Pump A18
#define DHTPIN 4
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
float h;
float t;
#define moisSensor A0
#define LED A19
#define ON LOW
#define OFF HIGH
const byte ldr_pin = A7;
const int led_pin = A13;
int lightValue;
const int threshold = 2000;

WidgetLCD switch1(V1);

void myTimerEvent() {
  Blynk.virtualWrite(V2, analogRead(moisSensor));
  Blynk.virtualWrite(V3, h);
  Blynk.virtualWrite(V4, t);
  Blynk.virtualWrite(V5, lightValue);

}



char auth[] = "rwVU-f49W0PcTt9QsNkjmqZDNarYTrbK";
char WIFI_SSID[] = "Cake";
char WIFI_PASSWORD[] = "21248877";

int temp;
int value;
//เวลา
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;
bool flag = true;

char timeHour[10];
String timenow;
String morning;
String evening;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  strftime(timeHour, 10, "%R", &timeinfo);
}


void setup() {

  Serial.begin(115200);
  dht.begin();
  Serial.println(LINE.getVersion());
  LINE.setToken(LINE_TOKEN);
  pinMode(ldr_pin, INPUT);
  pinMode(led_pin, OUTPUT);

  pinMode(LED, OUTPUT);
  pinMode(Pump, OUTPUT);
  digitalWrite(Pump, OFF);
  pinMode(moisSensor, INPUT);

  //https://www.praphas.com/forum/index.php?topic=341.0https://www.praphas.com/forum/index.php?topic=341.0https://www.praphas.com/forum/index.php?topic=341.0
  timer.setInterval(1000L, myTimerEvent);

  //analogReadResolution(12);

  Blynk.begin(auth, WIFI_SSID, WIFI_PASSWORD, "blynk.iot-cm.com", 8080);
  //connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  Serial.println("CONNECTED");
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //disconnect WiFi as it's no longer needed
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  delay(2000);

}


void loop() {
  Blynk.run();
  timer.run();
  //วัดอุณหภูมิและความชื้นในอากาศ
  float newh = dht.readHumidity();
  float newt = dht.readTemperature();
  h = newh;
  t = newt;
  Serial.print(F("ความชื้นในอากาศ : "));
  Serial.print(h);
  Serial.print(F("% อุณหภูมิ : "));
  Serial.println(t);

  //วัดค่าความชื้นในดิน
  value = analogRead(moisSensor);
  Serial.print("ความชื้นในดิน = ");
  Serial.println(value);
  if (pinValue == 1) {
    if (value >= 4095) {
      digitalWrite(Pump, ON);
      Serial.println("Pump ON");
    }
    else {
      LINE.notify("น้องมอสได้รับน้ำเพียงพอแล้วจ้า");
      digitalWrite(Pump, OFF);
      Serial.println("Pump OFF");
      switch1.clear();
      pinValue = 0;
      Serial.print("V1 Slider value is: ");
      Serial.println(pinValue);
    }
  }
  else {
    digitalWrite(Pump, OFF);
    Serial.println("Pump OFF");
  }

  //Line notify
  printLocalTime();
  timenow = timeHour;
  morning = "13:23";
  evening = "13:25";

  //เช็คค่าตอนเช้า
  Serial.println(flag);
  if (timenow == morning) {
    while (flag == true) {
      LINE.notify("สวัสดีตอนเช้า");
      if (value >= 4095) {
        LINE.notifySticker("ได้เวลารดน้ำแล้ว", 789, 10874);
      } else {
        LINE.notify("เช้านี้ยังไม่ต้องรดน้ำ น้องมอสได้รับน้ำเพียงพอแล้วจ้า");
      }
      Serial.println("LINE notify success");
      delay(500);
      flag = false;
    }
  }



  //เช็คค่าตอนเย็น
  if (timenow == evening) {
    while (flag == false) {
      LINE.notify("สวัสดีตอนเย็น");
      if (value >= 4095) {
        LINE.notifySticker("ได้เวลารดน้ำแล้ว", 789, 10874);
      } else {
        LINE.notify("เย็นนี้ไม่ต้องรดน้ำแล้ว น้องมอสได้รับน้ำเพียงพอแล้วจ้า");
      }
      Serial.println("LINE notify success.");
      delay(500);
      flag = true;
    }
  }

  delay(2000);

  //วัดค่าความเข้มแสง
  lightValue = analogRead(ldr_pin);
  Serial.print("ความเข้มแสง = ");
  Serial.println(lightValue);
  if (lightValue > threshold ) {
    digitalWrite(led_pin, LOW);
    Serial.println("LED ON");
  }
  else {
    digitalWrite(led_pin, HIGH);
    Serial.println("LED OFF");
  }
}

BLYNK_WRITE(V1) {
  pinValue = param.asInt();
  Serial.print("V1 Slider value is: ");
  Serial.println(pinValue);
  if (pinValue == 1) {
    if (value >= 4095) {
      digitalWrite(Pump, ON);
      Serial.println("Pump ON ...");
    } else {
      LINE.notify("ไม่ต้องรดน้ำ น้องมอสได้รับน้ำเพียงพอแล้วจ้า");
      digitalWrite(Pump, OFF);
      Serial.println("Pump OFF");
      switch1.clear();
      pinValue = 0;
    }
  }
  else {
    digitalWrite(Pump, OFF);
    switch1.clear();
  }
}
