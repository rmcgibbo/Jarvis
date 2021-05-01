#include <Ticker.h>
#include <cstdint>
#include "MQTT.h"
#include "secrets.h"
#include "IO.h"
#include "JarvisDesk.h"


JarvisDesk jarvis;

// flip light every second
bool ledState = false;
Ticker flipper([=]() {
    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;
}, 1000);

// subscribe to new height commands
MQTT mqtt([=](char* command) {
  auto value = atoi(command);
  if (value > 0 && value < 256) {
    jarvis.goto_height(value);
  }
});

// publish height to adafruit.io every 60 seconds
Ticker publisher([=]() {
    mqtt.publish_height(jarvis.height_);
}, 60000);


/* ---------------------------------------------------------------------------- */
/*                                          main                                */
/* ---------------------------------------------------------------------------- */
void setup() {
  io.setup(true);
  mqtt.setup();
  jarvis.begin();
  flipper.start();
  publisher.start();
}

void loop() {
  mqtt.run();
  io.run();
  jarvis.run();
  flipper.update();
  publisher.update();

  // if (Serial.available()) {
  //   int target = Serial.parseInt();
  //   LOG("GOING TO %d\n", target);
  //   jarvis.goto_height(target);
  // }
}
