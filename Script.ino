void handleScript()
{
  server.send(200, "text/javascript", readFile("/www/script.js"));
}

