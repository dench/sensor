void handleHome()
{
  log("Open setting");

  timer_ap.once(POINT_TIME, closeAP);

  String html = readFile("/www/index.tpl");

  if (server.hasArg("my_ssid"))
  {
    log("Submit");

    log("dhcp " + server.arg("dhcp"));
    writeConfig("dhcp", server.arg("dhcp"));

    saveParam("my_ssid");
    saveParam("my_password");
    saveParam("io_host");
    saveParam("local_ip");
    saveParam("gateway");
    saveParam("subnet");
    
    html.replace("{content}", "<div id=\"ok\">Settings saved!</div>");

    server.send(200, "text/html", html);
    wifiConfig();
  }
  else
  {
    String networks = "";
    int n = WiFi.scanNetworks();
    if (n == 0) {
      networks += "no networks found";
    } else {
      for (int i = 0; i < n; ++i) {
        networks += "<option";
        if (WiFi.SSID(i) == WiFi.SSID()) {
          networks += " selected=\"selected\"";
        }
        networks = networks+">" + WiFi.SSID(i) + "</option>";
      }
    }
  
    html.replace("{content}", readFile("/www/home.tpl"));
  
    String dhcp_checked = "";
    if (readConfig("dhcp") == "1") {
      dhcp_checked = " checked=\"checked\"";
    }
    
    html.replace("{networks}", networks);
    html.replace("{dhcp_checked}", dhcp_checked);
    html.replace("{my_password}", readConfig("my_password"));
    html.replace("{io_host}", readConfig("io_host"));
    html.replace("{local_ip}", readConfig("local_ip"));
    html.replace("{gateway}", readConfig("gateway"));
    html.replace("{subnet}", readConfig("subnet"));
    
    server.send(200, "text/html", html);
  }
}

void saveParam(String name)
{
  if (server.hasArg(name)) {
    log(name + " " + server.arg(name));
    String value = server.arg(name);
    value.trim();
    writeConfig(name, value);
  }
}


