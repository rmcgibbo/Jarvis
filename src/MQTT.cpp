#include "MQTT.h"
#include "IO.h"
#include "secrets.h"


MQTT::MQTT(callback_function_t on_command)
  : mqtt_(&client_, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY)
  , height_(&mqtt_, AIO_USERNAME "/feeds/height")
  , commands_(&mqtt_, AIO_USERNAME "/feeds/commands")
  , on_command_(on_command)
{
}

void MQTT::setup() {
    // io.adafruit.com SHA1 fingerprint
    static const char *fingerprint PROGMEM = "59 3C 48 0A B1 8B 39 4E 0D 58 50 47 9A 13 55 60 CC A0 1D AF";

    client_.setFingerprint(fingerprint);
    mqtt_.subscribe(&commands_);
}

void MQTT::run() {
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt_.readSubscription(100))) {
    if (subscription == &commands_) {
      on_command_((char *)commands_.lastread);      
    }
  }

  // Stop if already connected.
  if (mqtt_.connected()) {
    return;
  }

  LOG("Connecting to MQTT... ");

  uint8_t retries = 3;
  // TODO move this into a timer
  int8_t ret;
  while ((ret = mqtt_.connect()) != 0) { // connect will return 0 for connected
       LOG(mqtt_.connectErrorString(ret));
       LOG("Retrying MQTT connection in 5 seconds...");
       mqtt_.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }

  LOG("MQTT Connected!");
}
