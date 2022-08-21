#define _GLIBCXX_USE_C99 1

#include <WebServer.h>
#include <WiFi.h>
#include <uri/UriBraces.h>

const char *ssid = "MindSharkFire";
const char *password = "stayoutofmindshark";

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

int pins[] = {22, 21, 17, 16, 25};
bool states[] = {LOW, LOW, LOW, LOW, LOW};
int outputs = 5;

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < outputs; i++) {
    pinMode(pins[i], OUTPUT);
  }
  Serial.println("Set Pin Modes Complete");
  initWifi();
  Serial.println("Access Point Config Complete");
  delay(100);

  server.on("/", handle_shutoff);
  server.on(UriBraces("/{}/on/"), handle_On);
  server.on(UriBraces("/{}/off/"), handle_Off);
  server.onNotFound(handle_NotFound);
  Serial.println("Handler Set Complete");

  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  for (int i = 0; i < outputs; i++) {
    if (states[i]) {
      digitalWrite(pins[i], HIGH);
    } else {
      digitalWrite(pins[i], LOW);
    }
  }
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  // Configures static IP address
  if (!WiFi.config(local_ip, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  Serial.println(WiFi.localIP());
}

void handle_shutoff() { UpdateState("all", false); }

void handle_On() {
  String path_param = server.pathArg(0);
  UpdateState(path_param, HIGH);
}

void handle_Off() {
  String path_param = server.pathArg(0);
  UpdateState(path_param, LOW);
}

void handle_NotFound() { server.send(404, "text/plain", "Not found"); }

void UpdateState(String path_param, bool state) {
  if (path_param == "all") {
    for (int i; i < outputs; i++) {
      states[i] = state;
    }
    server.send(200, "text/plain", "State Set for All");
  } else {
    try {
      int output_index = path_param.toInt();
      if (output_index < outputs) {
        states[output_index] = state;
        server.send(200, "text/plain", "State Set: '" + path_param + "'");
      } else {
        server.send(416, "text/plain",
                    "Index out of Range: '" + path_param + "'");
      }
    } catch (std::invalid_argument const &e) {
      server.send(416, "text/plain", "Bad input: std::invalid_argument thrown");
    } catch (std::out_of_range const &e) {

      server.send(416, "text/plain",
                  "Integer overflow: std::out_of_range thrown");
    }
  }
}
