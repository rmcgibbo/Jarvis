Jarvis Desk Controller
======================

This is code for building a jarvis desk controller that talks wifi.
Based on https://github.com/phord/Jarvis, which figured out the pins.

Notes
-----
I'm using PlatformIO Core, version 5.0.4. Build with `pio run`, upload to device
with `pio run --target upload`.

Wiring:
![IMG_1563](https://user-images.githubusercontent.com/641278/118867800-782efe80-b8b1-11eb-8139-678afb8e69c5.JPG)

secrets: You need to supply your own secrets.h file. You can't use mine, because I don't want randos on the internet
to change the height of my desk.

```
#define WLAN_SSID       "your ssid"
#define WLAN_PASS       "your password"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883
#define AIO_USERNAME    "your username for adafruit io"
#define AIO_KEY         "your key for adafruit io"
```

Raising and lowering the desk with `curl`:
```
$ curl -F "value=55"  -H "X-AIO-Key: $YOUR_AIO_KEY" https://io.adafruit.com/api/v2/$YOUR_AIO_USERNAME/feeds/commands/data  # low
$ curl -F "value=165" -H "X-AIO-Key: $YOUR_AIO_KEY" https://io.adafruit.com/api/v2/$YOUR_AIO_USERNAME/feeds/commands/data  # high
```
