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

static bool invertLogic = false;
int pins[] = {22, 21, 17, 16, 25};
bool states[] = {LOW, LOW, LOW, LOW, LOW};
bool dirty[] = {false, false, false, false, false};
int outputs = 5;

static int taskCore = 0;
TaskHandle_t Task0;
// TaskHandle_t Task1;
// SemaphoreHandle_t stateSemaphore0 = xSemaphoreCreateMutex();
// SemaphoreHandle_t stateSemaphore1 = xSemaphoreCreateMutex();
// SemaphoreHandle_t stateSemaphore2 = xSemaphoreCreateMutex();
// SemaphoreHandle_t stateSemaphore3 = xSemaphoreCreateMutex();
// SemaphoreHandle_t stateSemaphore4 = xSemaphoreCreateMutex();

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < outputs; i++) {
    pinMode(pins[i], OUTPUT);
  }
  Serial.println("Set Pin Modes Complete");
  initWifi();
  Serial.println("Access Point Config Complete");
  delay(100);

  Serial.println("Creating Tasks");
  xTaskCreatePinnedToCore(serverTask, /* Function to implement the task */
                          "server",   /* Name of the task */
                          10000,      /* Stack size in words */
                          NULL,       /* Task input parameter */
                          0,          /* Priority of the task */
                          &Task0,     /* Task handle. */
                          0);         /* Core where the task should */
  Serial.println("Tasks created...");
}

void serverTask(void *pvParameters) {
  // Set Handlers
  server.on("/", handle_shutoff);
  server.on(UriBraces("/{}/on/"), handle_On);
  server.on(UriBraces("/{}/off/"), handle_Off);
  server.onNotFound(handle_NotFound);
  Serial.println("Handler Set Complete");

  server.begin();
  Serial.println("HTTP server started");

  while (true) {
    server.handleClient();
  }
}

void loop() {
  // for (int i = 0; i < outputs; i++) {
  //   if (xSemaphoreTake(getSemaphore(i), (TickType_t)10) == pdTRUE) {
  //     if (dirty[i]) {
  //       Serial.println("Pin");
  //       Serial.println(pins[i]);
  //       Serial.println("Pin Dirty");
  //       Serial.println(dirty[i]);
  //       if (states[i]) {
  //         digitalWrite(pins[i], High());
  //       } else {
  //         digitalWrite(pins[i], Low());
  //       }
  //       // debug output pin state:
  //       Serial.println("Pin");
  //       Serial.println(pins[i]);
  //       Serial.println("Pin State");
  //       Serial.println(digitalReadOutputPin(pins[i]));
  //       dirty[i] = false;
  //     }
  //     xSemaphoreGive(getSemaphore(i));
  //   }
  // }
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
      if (state) {
        digitalWrite(pins[i], High());
      } else {
        digitalWrite(pins[i], Low());
      }

      // Serial.println("Pin");
      // Serial.println(pins[i]);
      // Serial.println("Pin State");
      // Serial.println(state);
      // while (xSemaphoreTake(getSemaphore(i), (TickType_t)10) != pdTRUE) {
      //   delay(50);
      // }
      // if (state != states[i]) {
      //   states[i] = state;
      //   dirty[i] = true;
      // }
      // xSemaphoreGive(getSemaphore(i));
    }
    server.send(200, "text/plain", "State Set for All");
  } else {
    try {
      int output_index = path_param.toInt();
      if (output_index == 0) {
        if (path_param != "0") {
          server.send(416, "text/plain",
                      "Bad input: std::invalid_argument thrown");
          return;
        }
      }
      if (output_index < outputs) {
        if (state) {
          digitalWrite(pins[output_index], High());
        } else {
          digitalWrite(pins[output_index], Low());
        }

        // Serial.println("Pin");
        // Serial.println(pins[output_index]);
        // Serial.println("Pin State");
        // Serial.println(state);
        // Wait for Semaphore
        // while (xSemaphoreTake(getSemaphore(output_index), (TickType_t)10)
        // !=
        //        pdTRUE) {
        //   delay(50);
        // }

        // if (state != states[output_index]) {
        //   states[output_index] = state;
        //   dirty[output_index] = true;
        // }

        // xSemaphoreGive(getSemaphore(output_index));
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

// SemaphoreHandle_t getSemaphore(int id) {
//   switch (id) {
//   case 0:
//     return stateSemaphore0;
//   case 1:
//     return stateSemaphore1;
//   case 2:
//     return stateSemaphore2;
//   case 3:
//     return stateSemaphore3;
//   case 4:
//     return stateSemaphore4;
//   }
// }

bool High() {
  if (!invertLogic) {
    return HIGH;
  } else {
    return LOW;
  }
}

bool Low() {
  if (invertLogic) {
    return HIGH;
  } else {
    return LOW;
  }
}

int digitalReadOutputPin(uint8_t pin) {
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN)
    return LOW;

  return (*portOutputRegister(port) & bit) ? HIGH : LOW;
}