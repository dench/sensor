/**
 * Получить текущее время на сервере данных
 * 
 * @return bool
 */
bool getTime()
{
  return sendToServer("");
}

/**
 * Отправка данных на сервер
 * 
 * @return bool
 */
bool sendToServer(String motion)
{
  String host = readConfig("io_host");

  #if defined SERIAL
  Serial.println("Connecting to " + host);
  #endif
  if (!client.connect(host.c_str(), 80)) {
    log("Connection failed");
    return false;
  }
  String url = "/";
  url += "?id=1";
  if (motion.length()) {
    url += "&motion=";
    url += motion;
    Serial.println(motion);
  }

  #if defined DEBUG
    url += "&debug=";
    if (!debug.length()) {
      log("Up");
    }
    url += escapeUrl(debug);
  #endif
  
  client.print("GET " + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" + 
             "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      log("Timeout");
      client.stop();
      return false;
    }
  }
  String response = "";
  while(client.available()){
    response += client.readStringUntil('\n') + '\n';
  }
  response.trim();

  Serial.println(response);
  
  String response_json = response.substring(response.indexOf('{'), response.lastIndexOf('}')+1);

  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& json = jsonBuffer.parseObject(response_json);

  #if defined DEBUG
    debug = "";
  #endif

  if (json["time"]) {
    timer = json["time"];
    #if defined SERIAL
    Serial.println("Time " + String(timer));
    #endif
    if (json["fs"]) {
      Dir dir = SPIFFS.openDir("/");
      while (dir.next()) {
        log(dir.fileName());
      }
    }
    if (json["read"]) {
      const char* filename = json["read"]["filename"];
      #if defined SERIAL
      Serial.println(filename);
      #endif
      log(readFile(String(filename)));
    }
    if (json["remove"]) {
      const char* filename = json["remove"]["filename"];
      if (SPIFFS.remove(filename)) {
        log("File removed");
      }
    }
    if (json["write"]) {
      if (writeFile(json["write"]["filename"], json["write"]["data"])) {
        log("File saved");
      }
    }
    if (json["reset"]) {
      ESP.reset();
    }
    return true;
  } else {
    log("Invalid response");
    return false;
  }
}

/**
 * Инициализация пинов
 */
void initIO()
{
  // Подключение сенсора
  pinMode(PIN, INPUT);

  // Подключение индикатора
  pinMode(LED, OUTPUT);
}

void onLed(int led)
{
  digitalWrite(led, ON);
}

void offLed(int led)
{
  digitalWrite(led, OFF);
}

void toggleLed(int led)
{
  digitalWrite(led, !digitalRead(led));
}

/**
 * Мигание светодиодом
 */
void flip()
{
  toggleLed(LED);
}

/**
 * Сервер для настройки
 */
void initServer()
{
  log("Init server");
  server.on("/", handleHome);
  server.on("/style.css", handleCss);
  server.on("/script.js", handleScript);
}

void beginServer()
{
  log("Begin server");
  server.begin();
}

void stopServer()
{
  log("Stop server");
  server.stop();
}

/**
 * Конфигурация к Wi-Fi
 */
void wifiConfig()
{
  String ssid = readConfig("my_ssid");
  String password = readConfig("my_password");

  log("SSID " + ssid);

  WiFi.begin(ssid.c_str(), password.c_str());
  
  if (readConfig("dhcp") != "1") {
    log("Config IP");
    IPAddress local_ip;
    IPAddress gateway;
    IPAddress subnet;
    local_ip.fromString(readConfig("local_ip"));
    gateway.fromString(readConfig("gateway"));
    subnet.fromString(readConfig("subnet"));
    WiFi.config(local_ip, gateway, subnet);
  }
}

/**
 * Открытие точки доступа
 */
void openAP()
{
  String ssid = readConfig("ap_ssid");
  String password = readConfig("ap_password");
  
  log("Open AP " + ssid);
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid.c_str(), password.c_str());

  timer_ap.once(POINT_TIME, closeAP);

  // Старт сервера
  beginServer();
}

/**
 * Закрытие точки доступа
 */
void closeAP()
{
  log("Close AP");
  WiFi.mode(WIFI_STA);
}

/**
 * Чтение файла
 * 
 * @param key String - путь к файлу
 * @return String
 */
String readFile(String filename)
{
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    log("Failed open file " + filename);
    return "";
  }
  String text = "";
  while(file.available()) {
    text += file.readStringUntil('\n') + '\n';
  }
  text.trim();
  return text;
}

/**
 * Запись в файла
 * 
 * @param filename String - путь к файлу
 * @param data String - данные
 * @return bool
 */
bool writeFile(String filename, String data)
{
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    log("Failed open file " + filename);
    return false;
  }

  file.println(data);
  
  return true;
}

/**
 * Чтение конфигурации
 * 
 * @param key String - название конфигурации
 * @return String
 */
String readConfig(String key)
{
  return readFile("/config/" + key + ".cfg");
}

/**
 * Запись конфигурации
 * 
 * @param key String - название конфигурации
 * @param value String - значение конфигурации
 * @return bool
 */
bool writeConfig(String key, String value)
{
  return writeFile("/config/" + key + ".cfg", value);
}

/**
 * Сохранение строки в логи
 */
void log(String str) {
  #if defined SERIAL
  Serial.println(str);
  #endif
  #if defined DEBUG
    if (debug.length()) { debug += "|"; }
    str = String(millis()) + "@ " + str;
    debug += str;
  #endif
}

/**
 * Кодирование строки в url для передачи GET
 * 
 * @param url String
 * @return String
 */
String escapeUrl(String url)
{
  url.replace(" ","+");
  url.replace("!","%21");
  url.replace("#","%23");
  url.replace("$","%24");
  url.replace("&","%26");
  url.replace("'","%27");
  url.replace("(","%28");
  url.replace(")","%29");
  url.replace("*","%2A");
  url.replace("+","%2B");
  url.replace(",","%2C");
  url.replace("/","%2F");
  url.replace(":","%3A");
  url.replace(";","%3B");
  url.replace("=","%3D");
  url.replace("?","%3F");
  url.replace("@","%40");
  url.replace("[","%5B");
  url.replace("]","%5D");
  return url;
}
