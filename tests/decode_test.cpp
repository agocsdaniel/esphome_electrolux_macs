// g++ -std=c++17 -I components tests/decode_test.cpp -o /tmp/decode_test && /tmp/decode_test
// Expected values come from captures on an Electrolux EW8W261B, checked against its display.
#include <cassert>
#include <cstdio>
#include "electrolux_washing_machine_macs/decode.h"
#include "electrolux_washing_machine_macs/cycle.h"
using namespace esphome::electrolux_washing_machine_macs;

int main() {
  // Time Manager: 5..1 bars
  assert(time_manager_level(0x00, 0x01) == 5);
  assert(time_manager_level(0x00, 0x02) == 4);
  assert(time_manager_level(0x00, 0x08) == 3);
  assert(time_manager_level(0x00, 0x10) == 2);
  assert(time_manager_level(0x20, 0x00) == 1);
  assert(time_manager_level(0x00, 0x81) == 5);  // pre-wash bit (0x80) does not affect the level
  assert(std::isnan(time_manager_level(0x00, 0x00)));
  // EW8W261B program sweep: Time Manager only on Cotton, Cotton Eco, Synthetics and FreshScent
  assert(time_manager_level_ew8w(2, 0x00, 0x00, 0x41) == 5);  // Cotton Eco
  assert(std::isnan(time_manager_level_ew8w(4, 0x00, 0x00, 0x01)));   // Delicate
  assert(std::isnan(time_manager_level_ew8w(10, 0x00, 0x00, 0x10)));  // Sportswear
  assert(std::isnan(time_manager_level_ew8w(13, 0x00, 0x00, 0x08)));  // OneGo 4h
  assert(time_manager_level_ew8w(6, 0x80, 0x00, 0x00) == 3);  // FreshScent default
  assert(time_manager_level_ew8w(6, 0x20, 0x00, 0x00) == 1);
  // Drying
  assert(drying_mode_name(0x00, 0x0A) == "Off");
  assert(drying_mode_name(0x01, 0x0A) == "Off");
  assert(drying_mode_name(0xC1, 0x0A) == "Auto: cupboard dry");
  assert(drying_mode_name(0xC0, 0x0A) == "Auto: extra dry");
  assert(drying_mode_name(0xC2, 0x0A) == "Auto: iron dry");
  assert(drying_mode_name(0x80, 0x0F) == "Timed 15 min");
  // OneGo 1h sends 0x91 and OneGo 4h 0x81 (which would read as timed), so it goes by program number
  assert(drying_mode_name(0x91, 0x0A, fixed_drying_ew8w(14)) == "Fixed by program");
  assert(drying_mode_name(0x81, 0x0A, fixed_drying_ew8w(13)) == "Fixed by program");
  assert(drying_mode_name(0x81, 0x0A, fixed_drying_ew8w(1)) == "Timed 10 min");
  // Door lock, EW8W261B sequence at pause: 01 -> 03 -> 00, and 25 while washing
  assert(door_locked(true, 0x25));
  assert(door_locked(true, 0x01));
  assert(!door_locked(true, 0x00));
  assert(door_locked(false, 0x00));  // EWX14 behaviour unchanged
  // Phases: unknown codes are reported raw
  assert(phase_name(0x02, 0x03) == "Rinse");
  assert(phase_name(0x02, 0x05) == "Drying");
  assert(phase_name(0x02, 0x07) == "Unknown phase 0x07");
  assert(phase_name(0x02, 0x08) == "Anti-crease");
  assert(phase_name(0x0C, 0x00) == "Unknown state 0x0C");
  // Program tracking: delayed start not counted, pauses subtracted, reset at the end
  CycleTracker c;
  assert(c.on_state(0x08, 1000) && !c.active());  // delayed start: program clock not started
  assert(c.on_state(0x02, 4600) && c.start == 4600);
  assert(!c.on_state(0x02, 4700));
  assert(c.on_state(0x04, 5200));   // pause after 10 min
  assert(c.elapsed_min(5800) == 10);  // still paused
  assert(c.on_state(0x02, 5800));   // resume after a 10 min pause
  assert(c.elapsed_min(6400) == 20);
  assert(cycle_progress(20, 60) == 25);
  assert(cycle_progress(20, 0) == 99);
  assert(std::isnan(cycle_progress(20, NAN)));
  assert(c.on_state(0x03, 6400) && !c.active());
  assert(!c.on_state(0x0B, 6500));
  // Delayed start (EW8W261B capture 2026-09-24: 0x08 while waiting, then running)
  CycleTracker d;
  assert(d.on_state(0x08, 1000) && d.delayed() && !d.active());
  assert(!d.on_state(0x08, 1060));
  assert(d.delay_waited_min(1600) == 10);
  assert(cycle_progress(d.delay_waited_min(1600), 50) == (float) (100.0 * 10 / 60));
  assert(d.on_state(0x02, 4600) && d.active() && !d.delayed() && d.elapsed_min(4600) == 0);
  CycleTracker e;  // delay cancelled
  assert(e.on_state(0x08, 1000) && e.on_state(0x01, 1200) && !e.delayed());
  assert(utc_offset(3 * 3600) == "+03:00");
  assert(utc_offset(-(5 * 3600 + 30 * 60)) == "-05:30");
  assert(utc_offset(0) == "+00:00");
  puts("ok");
}
