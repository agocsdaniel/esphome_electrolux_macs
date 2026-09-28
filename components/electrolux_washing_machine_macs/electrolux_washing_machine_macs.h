#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include "esphome/core/defines.h"
#include "esphome/components/electrolux_macs/electrolux_macs.h"
#include "esphome/core/preferences.h"
#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif
#include "cycle.h"

#include <vector>

namespace esphome {
namespace electrolux_washing_machine_macs {

static const uint8_t MACS_WM_WASH_PHASE_IDLE = 0x0B;
static const uint8_t MACS_WM_WASH_PHASE_HEATING = 0x01;
static const uint8_t MACS_WM_WASH_PHASE_COOLING = 0x02;
static const uint8_t MACS_WM_WASH_PHASE_ANTI_CREASE = 0x04;

class ElectroluxWashingMachineMacsComponent : public esphome::electrolux_macs::ElectroluxMacsComponent {
  public:
  ElectroluxWashingMachineMacsComponent(uart::UARTComponent *uart) : esphome::electrolux_macs::ElectroluxMacsComponent(uart) {}
  void setup() override;
  void dump_config() override;
#ifdef USE_TIME
  void set_time(time::RealTimeClock *time) { this->time_ = time; }
#endif
  void set_motor_drum_ratio(float motor_drum_ratio) { this->motor_drum_ratio_ = motor_drum_ratio;}
  void set_ew8w(bool ew8w) { this->ew8w_ = ew8w; }

#ifdef USE_SENSOR
  SUB_SENSOR(remaining_time)
  SUB_SENSOR(start_delay_time)
  SUB_SENSOR(selected_program_number)
  SUB_SENSOR(time_manager)
  SUB_SENSOR(wash_temperature)
  SUB_SENSOR(spin_speed)
  SUB_SENSOR(target_drum_speed)
  SUB_SENSOR(current_drum_speed)
  SUB_SENSOR(current_water_temperature)
  SUB_SENSOR(total_working_hours)
  SUB_SENSOR(total_cycles)
  SUB_SENSOR(sub_phase)
  SUB_SENSOR(laundry_load)
  SUB_SENSOR(program_progress)
  SUB_SENSOR(elapsed_time)
  SUB_SENSOR(start_delay_progress)
#endif

#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(active_alarm)
  SUB_TEXT_SENSOR(phase)
  SUB_TEXT_SENSOR(drying_mode)
  SUB_TEXT_SENSOR(program_start)
  SUB_TEXT_SENSOR(estimated_end)
  SUB_TEXT_SENSOR(wash_start)
#endif

#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(powered_on)
  SUB_BINARY_SENSOR(running)
  SUB_BINARY_SENSOR(door_locked)
  SUB_BINARY_SENSOR(pre_wash_enabled)
  SUB_BINARY_SENSOR(stain_wash)
  SUB_BINARY_SENSOR(extra_rinse)
  SUB_BINARY_SENSOR(soft_plus)
  SUB_BINARY_SENSOR(easy_iron)
  SUB_BINARY_SENSOR(anti_crease)
  SUB_BINARY_SENSOR(washing_enabled)
  SUB_BINARY_SENSOR(drain_pump)
  SUB_BINARY_SENSOR(water_in_drum)
  SUB_BINARY_SENSOR(drum_turning)
  SUB_BINARY_SENSOR(door_open)
#endif

  protected:
  float motor_drum_ratio_{1};
  bool ew8w_{false};
  // Program tracking (progress, elapsed, start, estimated end); needs time_id for the clock
  CycleTracker cycle_{};
  ESPPreferenceObject cycle_pref_;
  float remaining_min_{NAN};
  float remaining_delay_min_{NAN};
  bool cycle_shown_{false};
#ifdef USE_TIME
  time::RealTimeClock *time_{nullptr};
  std::string iso_time_(int64_t epoch);
#endif
  void on_cycle_state_(uint8_t state);
  void publish_cycle_();
  void decode_data_(uint8_t target, uint8_t source, std::vector<uint8_t> data) override;
  void decode_inverter_(uint8_t target, uint8_t source, std::vector<uint8_t> data);
  void decode_ui_(uint8_t target, uint8_t source, std::vector<uint8_t> data);
  
};
}  // namespace electrolux_washing_machine_macs
}  // namespace esphome

