#include <SoftwareSerial.h>
#include "IO.h"
#include "JarvisDesk.h"

static const int8_t HS0 = 4;  // RJ45 Pin 8
static const int8_t HS1 = 5;  // RJ45 Pin 7
static const int8_t HS2 = 13; // RJ45 Pin 6
static const int8_t HS3 = 14; // RJ45 Pin 1
static const int8_t Tx = 12;  // RJ45 Pin 2

static const int8_t FUDGE_BELOW = 2;
static const int8_t FUDGE_ABOVE = 4;

// From https://github.com/phord/Jarvis
// Pin 	Label 	Description
// 1  	HS3 	  Handset control line 3 [1]
// 2 	  Tx 	    Serial control messages from controller to handset [2]
// 3 	  GND 	  Ground
// 4 	  Rx   	  Serial control messages from handset to controller [2]
// 5 	  VCC 	  Vcc (5vdc) supply from desk controller [3]
// 6 	  HS2 	  Handset control line 2 [1]
// 7 	  HS1 	  Handset control line 1 [1]
// 8 	  HS0 	  Handset control line 0 [1]

// My board wiring:
// Huzzah 14 -> RJ45 Pin 1 (white)
// Huzzah 12 -> RJ45 Pin 2 (green)
// Huzzah 13 -> RJ45 Pin 6 (white)
// Huzzah 5  -> RJ45 Pin 7 (black)
// Huzzah 4  -> RJ45 Pin 8 (blue)



struct DeskPacket {
  enum state_t {
    SYNC1,   // waiting for 0x01
    SYNC2,   // waiting for 0x01 (second)
    HEIGHT1,  // waiting for first height arg, usually 0x01 but 0x00 if at the bottom
    HEIGHT2,  // waiting for second height arg
  } state_ = SYNC1;

  uint8_t buf_[2];

  // Process a byte sent by the desk controller. This returns
  // true when we have a complete packet, and false if we've
  // only processed a partial packet so far.
  // AFAIK, the packets sent by the desk controller containing
  // the height are sent as 4 bytes like (0x1, 0x1, (0x0|0x1), NUMBER),
  // and the height is encoded in the last two bytes.
  // Once put returns `true`, call decode() to get the height.
  bool put(uint8_t b) {
    switch (state_) {
    case SYNC1:
      if (b == 0x01) {
        state_ = SYNC2;
        return false;
      } else {
        state_ = SYNC1;
        return false;
      }
    case SYNC2:
      if (b == 0x01) {
        state_ = HEIGHT1;
        return false;
      } else{
        state_ = SYNC1;
        return false;
      }
    case HEIGHT1:
      if (b == 0x00 || b == 0x01) {
        buf_[0] = b;
        state_ = HEIGHT2;
        return false;
      } else {
        state_ = SYNC1;
        return false;
      }
    case HEIGHT2:
      buf_[1] = b;
      state_ = SYNC1;
      return true;
    default:
      LOG("PROGRAMMING ERROR");
      return false;
    }
    return false;
  }

  // decode height sent by the desk controller.
  // returns a number from 0 (lowest) to 256
  // (highest). This function can only be called
  // after put() returns true, which indicates
  // that we have enough data. Otherwise it'll
  // return uninitialized garbage.
  uint16_t decode() {
    return 256 * buf_[0] + buf_[1] - 255;
  }
};

SoftwareSerial deskSerial(Tx);
DeskPacket deskPacket;


void latch_pin(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void unlatch_pin(int pin) {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

// Read any available bytes from the desk controller,
// which seems to spend most of its time sending back
// the current height to the handset.
// Sets the instance variables `height_` and `height_changed_`.
void JarvisDesk::_read_desk_serial() {
  height_changed_ = false;
  while (deskSerial.available()) {
    auto ch = deskSerial.read();
    if (deskPacket.put(ch)) {
      int height = deskPacket.decode();
      if ((height != height_) && ((height_ == -1) || abs(height - height_) < 5)) {
        LOG("READ DESK SERIAL HEIGHT CHANGED new=%d old=%d\n", height, height_);
        height_ = height;
        height_changed_ = true;
      }
    }
  }
}

void JarvisDesk::_go_down() {
  if (moving_ != MOVE_DOWN) {
    moving_ = MOVE_DOWN;
    latch_pin(HS0);
    unlatch_pin(HS1);
  }
}

void JarvisDesk::_go_up() {
  if (moving_ != MOVE_UP) {
    moving_ = MOVE_UP;
    unlatch_pin(HS0);
    latch_pin(HS1);
  }
}

void JarvisDesk::_go_stop() {
  if (moving_ != MOVE_STOP) {
    moving_ = MOVE_STOP;
    unlatch_pin(HS0);
    unlatch_pin(HS1);
  }
}

void JarvisDesk::begin() {
  deskSerial.begin(9600);
  pinMode(Tx, INPUT);

  // Pulsing this signal triggers the desk to respond
  // with the current height, so at startup we can know
  // the current height, which is good.
  latch_pin(HS0);
  latch_pin(HS3);
  delay(30);
  unlatch_pin(HS0);
  unlatch_pin(HS3);
}


void JarvisDesk::run() {
  _read_desk_serial();

  if (height_changed_) {
    LOG("HEIGHT: %d MOVING: %d: TARGET=%d\n", height_, moving_, target_height_);
  }

  // change which direction we're moving in
  if (target_height_ > 0) {
    if (height_ < target_height_ - FUDGE_BELOW) {
      // LOG("Target = %d, Current = %d, moving up\n", target_height_, height_);
      _go_up();
    } else if (height_ > target_height_ + FUDGE_ABOVE) {
      // LOG("Target = %d, Current = %d, moving down\n", target_height_, height_);
      _go_down();
    } else {
      // LOG("Target = %d, Current = %d, stopping\n", target_height_, height_);
      _go_stop();
    }
  }
}

void JarvisDesk::goto_height(int16_t target_height) {
  if (target_height >= 0 && target_height <= 256) {
    target_height_ = target_height;
  } else {
    target_height_ = -1;
  }
}