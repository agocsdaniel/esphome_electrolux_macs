#pragma once
// Pure decoding helpers, free of ESPHome dependencies so they can be tested on the host
// (see tests/decode_test.cpp).
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

namespace esphome {
namespace electrolux_washing_machine_macs {

// Program set frame: [7] bits 0x1B hold the Time Manager level; level 1 is [7] = 0 plus [5] & 0x20.
inline float time_manager_level(uint8_t b5, uint8_t b7) {
  switch (b7 & 0x1B) {
    case 0x01: return 5;
    case 0x02: return 4;
    case 0x08: return 3;
    case 0x10: return 2;
    case 0x00: return (b5 & 0x20) ? 1 : NAN;
    default: return NAN;
  }
}

// EW8W261B: the panel sends default Time Manager bits even on programs without it, and several
// programs default to other [7] values (Sportswear 0x10, OneGo 4h 0x08), so the level only means
// something on the programs that offer it: 1-3 use [5]/[7] as above, FreshScent (6) uses [4]
// 0x80/0x40/0x20 for 3/2/1 bars.
inline float time_manager_level_ew8w(uint8_t program, uint8_t b4, uint8_t b5, uint8_t b7) {
  switch (program) {
    case 1:
    case 2:
    case 3: return time_manager_level(b5, b7);
    case 6:
      switch (b4 & 0xE0) {
        case 0x80: return 3;
        case 0x40: return 2;
        case 0x20: return 1;
        default: return NAN;
      }
    default: return NAN;
  }
}

// EW8W261B programs 13 (OneGo 4h 5kg) and 14 (OneGo 1h 1kg) wash and dry with fixed drying.
inline bool fixed_drying_ew8w(uint8_t program) { return program == 13 || program == 14; }

inline std::string hex_code(const char *prefix, uint8_t v) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%s 0x%02X", prefix, v);
  return buf;
}

// Washer-dryers, program set frame: [8] 0x80 = drying on, 0x40 = auto dry (otherwise timed),
// bits 0-1 = dryness level; [9] = timed drying minutes.
// fixed_by_program: the program sets the drying itself (EW8W261B OneGo), [8]/[9] don't describe it.
inline std::string drying_mode_name(uint8_t b8, uint8_t b9, bool fixed_by_program = false) {
  if (!(b8 & 0x80)) return "Off";
  if (fixed_by_program) return "Fixed by program";
  if (!(b8 & 0x40)) return "Timed " + std::to_string(b9) + " min";
  switch (b8 & 0x03) {
    case 0: return "Auto: extra dry";
    case 1: return "Auto: cupboard dry";
    case 2: return "Auto: iron dry";
    default: return hex_code("Auto: unknown", b8);
  }
}

// State frame [5]. EWX14 (author's code): locked when bits 0-1 are clear.
// EW8W261B: bit 0x01 is the lock, 0x02 is set briefly while locking or unlocking.
inline bool door_locked(bool ew8w, uint8_t b5) { return ew8w ? (b5 & 0x01) != 0 : (b5 & 0x03) == 0; }

// State frame: codes from EWX FINDINGS.md, plus 0x05 (drying) seen on EW8W261B. Anything not listed is reported raw instead of guessed.
inline std::string phase_name(uint8_t state, uint8_t phase) {
  switch (state) {
    case 0x01: return "Idle";
    case 0x02:
      switch (phase) {
        case 0x00: return "Running";
        case 0x01: return "Pre-wash";
        case 0x02: return "Wash";
        case 0x03: return "Rinse";
        case 0x04: return "Spin";
        case 0x05: return "Drying";
        case 0x08: return "Anti-crease";  // EW8W261B: after the spin, remaining time 0, door still locked
        default: return hex_code("Unknown phase", phase);
      }
    case 0x03: return "Finished";
    case 0x04: return "Paused";
    case 0x06: return "Waiting for door unlock";
    case 0x08: return "Delayed start";
    case 0x09: return "Error";
    case 0x0B: return "Off";
    default: return hex_code("Unknown state", state);
  }
}

}  // namespace electrolux_washing_machine_macs
}  // namespace esphome
