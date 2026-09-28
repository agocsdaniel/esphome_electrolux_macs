#pragma once
// Program cycle tracking from the state frame, free of ESPHome dependencies so it can be tested on
// the host (see tests/decode_test.cpp). Times are unix epoch seconds.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace esphome {
namespace electrolux_washing_machine_macs {

// Kept in flash, so a reboot in the middle of a program keeps counting from the real start.
struct CycleTracker {
  int64_t start{0};         // first running state frame of the program, 0 = no program
  int64_t paused_total{0};  // seconds spent paused, finished pauses only
  int64_t pause_since{0};   // start of the current pause, 0 = not paused
  int64_t delay_start{0};   // first delayed start (0x08) frame, 0 = not waiting

  // Returns true when the tracker changed. A delayed start (0x08) is tracked apart: the program clock
  // starts at the first running (0x02) frame. Paused (0x04) and waiting for the door (0x06) stop it.
  bool on_state(uint8_t state, int64_t now) {
    switch (state) {
      case 0x08:
        if (this->start == 0 && this->delay_start == 0) {
          this->delay_start = now;
          return true;
        }
        return false;
      case 0x02:
        if (this->start == 0) {
          *this = CycleTracker{now, 0, 0, 0};
          return true;
        }
        if (this->pause_since != 0) {
          this->paused_total += now - this->pause_since;
          this->pause_since = 0;
          return true;
        }
        return false;
      case 0x04:
      case 0x06:
        if (this->start != 0 && this->pause_since == 0) {
          this->pause_since = now;
          return true;
        }
        return false;
      case 0x01:  // idle: cancelled, or a new program being set up
      case 0x03:  // finished
      case 0x0B:  // off
        if (this->start == 0 && this->delay_start == 0) return false;
        *this = CycleTracker{};
        return true;
      default:
        return false;
    }
  }

  bool active() const { return this->start != 0; }
  bool delayed() const { return this->start == 0 && this->delay_start != 0; }

  // Minutes waited so far for a delayed start.
  float delay_waited_min(int64_t now) const { return (now - this->delay_start) / 60.0f; }

  // Minutes the program has actually been running, pauses excluded.
  float elapsed_min(int64_t now) const {
    int64_t paused = this->paused_total + (this->pause_since ? now - this->pause_since : 0);
    return (now - this->start - paused) / 60.0f;
  }
};

// Share of the program (or of the start delay) done, from the time gone so far and the time the
// machine says is left. Capped at 99 until the machine moves on.
inline float cycle_progress(float elapsed_min, float remaining_min) {
  if (std::isnan(remaining_min) || elapsed_min + remaining_min <= 0) return NAN;
  return std::min(99.0f, 100.0f * elapsed_min / (elapsed_min + remaining_min));
}

// "+03:00" for an offset of 3 h east of UTC, as ISO 8601 wants it.
inline std::string utc_offset(int32_t offset_s) {
  char buf[16];
  int32_t a = std::abs(offset_s);
  snprintf(buf, sizeof(buf), "%c%02d:%02d", offset_s < 0 ? '-' : '+', (int) (a / 3600), (int) (a % 3600 / 60));
  return buf;
}

}  // namespace electrolux_washing_machine_macs
}  // namespace esphome
