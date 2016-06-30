#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <FS.h>

#define DEBUG true // Режим 1 - для разработки | 0 - для готового устройства
#define SERIAL false
#define POINT_TIME 300 // Время отображения точки доступа в сек
#define PIN 3 // Пин сигнала от датчика
#define LED 2 // Пин индикатора
#define ON LOW // Уровень для включения индикатора
#define OFF HIGH // Уровень для отключения индикатора
#define FS_COUNT 60 // Максимальное количество записей в data
#define FLIP_PERIOD 0.2 // Период миганий в сек

#if defined DEBUG
// Данные отладки для отправки на сервер
String debug = "";
#endif

Ticker timer_ap;
Ticker timer_in;
Ticker flipper;
ESP8266WebServer server(80);
WiFiClient client;

unsigned int timer = 0; // Метка времени
bool send = false; // Флаг для отправки данных
bool buf = false; // Буфер сработки
bool led_on = false; // Буфер сработки
bool wifi = false; // Флаг активности Wi-Fi
String data = ""; // Данные из меток сработок, которые еще не были отправлены на сервер.
int fs_count = 0; // Количество данных в data

void setup()
{
  #if defined SERIAL
  Serial.begin(115200);
  Serial.println("");
  #endif
  log("Start device");

  /**
   * Инициализация пинов
   */
  initIO();

  // TODO: Сделать мигание с таймаутом или мигать только когда есть движения
  flipper.attach(FLIP_PERIOD, flip);
  
  /**
   * Монтирование файловой системы
   */
  SPIFFS.begin();

  /**
   * Инициализация сервера внутренних настроек
   */
  initServer();

  /**
   * Открытие точки доступа
   */
  openAP();

  /**
   * Конфигурация Wi-Fi
   */
  wifiConfig();

  /**
   * Получить текущее время на сервере данных
   */
  // TODO: Сделать, чтоб при первом коннекте к серверу, добавлялось время к прошлым сработкам
  if (!getTime()) {
    if (!getTime()) {
      getTime();
    }
  }

  /**
   * Включение таймера с интервалом 60 сек
   */
  timer_in.attach(60, interval);
}

void loop()
{
  // Отлов запросов к серверу настроеек
  server.handleClient();

  if (WiFi.status() == WL_CONNECTED && !wifi) {
    flipper.detach();
    wifi = true; 
    log("Wi-Fi connected");
    log("Local IP " + WiFi.localIP().toString());
  }
  if (WiFi.status() != WL_CONNECTED && wifi) {
    flipper.attach(FLIP_PERIOD, flip);
    wifi = false;
    log("Wi-Fi disconnected");
  }

  // Детектор движения
  if (digitalRead(PIN) == HIGH) {
    buf = true;
    if (!led_on) {
      led_on = true;
      onLed(LED);
      #if defined SERIAL
      Serial.println("Led ON");
      #endif
    }
  } else {
    if (led_on) {
      led_on = false;
      offLed(LED);
      #if defined SERIAL
      Serial.println("Led OFF");
      #endif
    }
  }

  if (send) {
    send = false;
    trigger();
  }
}

/**
 * Сохранение и отправка данных на сервер
 */
void trigger() {
  if (buf) {
    if (data.length()) { data += "."; }
    data += timer;
  }
  buf = false;
  timer++;

  fs_count++;
  if (fs_count >= FS_COUNT) {
    fs_count = 0;
    if (data.length() && writeFile("/data/" + String(timer) + ".txt", data)) {
      data = "";
    }
  }
  
  if (sendToServer(data)) {
    data = "";
    fs_count = 0;
    Dir dir = SPIFFS.openDir("/data");
    while (dir.next()) {
      log("Read file");
      if (sendToServer(readFile(String(dir.fileName())))) {
        if (SPIFFS.remove(dir.fileName())) {
          log("Delete file");
        }
      }
    }
  }
}

/**
 * Интервал по таймеру 60 сек
 */
void interval()
{
  send = true;
}
