#include "electrolux_washing_machine_macs.h"
#include "decode.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

using namespace esphome::electrolux_macs;
namespace esphome {
namespace electrolux_washing_machine_macs {

static const char *const TAG = "electrolux_washing_machine_macs";

void ElectroluxWashingMachineMacsComponent::decode_ui_(uint8_t target, uint8_t source, std::vector<uint8_t> data) {  
  if (data.size() < 3) return;
  uint8_t msg_type_ = data[0];
  char alarm_buf[4] = {0};
  uint16_t tmp_ = 0;
  switch (msg_type_) {
    case MACS_MESSAGE_TYPE_TIME_CHANGE:
      if (data.size() < 5) break;
#ifdef USE_SENSOR
      switch (data[2]) {
        case MACS_TIME_CHANGE_PROGRAM_TIME:
          tmp_ = encode_uint16(data[3], data[4]);
          this->remaining_min_ = tmp_ == 65535 ? NAN : (float) tmp_;
          if (this->remaining_time_sensor_) {
            if (tmp_ == 65535) this->remaining_time_sensor_->publish_state(NAN);
            else this->remaining_time_sensor_->publish_state((float) tmp_);
          }
          this->publish_cycle_();  // keep the estimated end in step with the machine
          break;
        case MACS_TIME_CHANGE_START_DELAY:
          tmp_ = encode_uint16(data[3], data[4]);
          // 10 s units. The controller keeps counting down after a cancelled delay, even when off,
          // so this only means something while the state is delayed start (0x08).
          this->remaining_delay_min_ = tmp_ == 65535 ? NAN : tmp_ / 6.0f;
          if (this->start_delay_time_sensor_) {
            if (tmp_ == 65535) this->start_delay_time_sensor_->publish_state(NAN);
            else this->start_delay_time_sensor_->publish_state(((float) tmp_) / 6);
          }
          // the countdown frame follows the delayed start state, which may have used the stale one
          if (this->cycle_.delayed()) this->publish_cycle_();
          break;
        case MACS_TIME_CHANGE_TOTAL_WORK_HOURS:
          tmp_ = encode_uint16(data[3], data[4]);
          if (this->total_working_hours_sensor_) this->total_working_hours_sensor_->publish_state(((float) tmp_) / 10);
          break;
        case MACS_TIME_CHANGE_TOTAL_CYCLES:
          tmp_ = encode_uint16(data[3], data[4]);
          if (this->total_cycles_sensor_) this->total_cycles_sensor_->publish_state(tmp_);
          break;
        case 0x04:
          // EW8W261B: weighed laundry in grams (500 g steps, as on the display), FFFF = no reading
          tmp_ = encode_uint16(data[3], data[4]);
          if (this->laundry_load_sensor_) this->laundry_load_sensor_->publish_state(tmp_ == 0xFFFF ? NAN : tmp_ / 1000.0f);
          break;
      }
#endif
      break;
      
    case MACS_MESSAGE_TYPE_STATE:
      if (data.size() < 6) break;
#ifdef USE_TEXT_SENSOR
      if (this->phase_text_sensor_) this->phase_text_sensor_->publish_state(phase_name(data[2], data[3]));
#endif
      this->on_cycle_state_(data[2]);
      switch (data[2]) {
        case MACS_APPLIANCE_STATE_STANDBY:
#ifdef USE_BINARY_SENSOR
          if (this->powered_on_binary_sensor_) this->powered_on_binary_sensor_->publish_state(0);
          if (this->running_binary_sensor_) this->running_binary_sensor_->publish_state(0);
          if (this->door_locked_binary_sensor_) this->door_locked_binary_sensor_->publish_state(0);
          if (this->pre_wash_enabled_binary_sensor_) this->pre_wash_enabled_binary_sensor_->publish_state(NAN);
          if (this->stain_wash_binary_sensor_) this->stain_wash_binary_sensor_->publish_state(NAN);
          if (this->extra_rinse_binary_sensor_) this->extra_rinse_binary_sensor_->publish_state(NAN);
          if (this->soft_plus_binary_sensor_) this->soft_plus_binary_sensor_->publish_state(NAN);
          if (this->easy_iron_binary_sensor_) this->easy_iron_binary_sensor_->publish_state(NAN);
          if (this->anti_crease_binary_sensor_) this->anti_crease_binary_sensor_->publish_state(NAN);
          if (this->washing_enabled_binary_sensor_) this->washing_enabled_binary_sensor_->publish_state(NAN);
          if (this->drain_pump_binary_sensor_) this->drain_pump_binary_sensor_->publish_state(0);
          if (this->water_in_drum_binary_sensor_) this->water_in_drum_binary_sensor_->publish_state(0);
          if (this->drum_turning_binary_sensor_) this->drum_turning_binary_sensor_->publish_state(0);
#endif
#ifdef USE_TEXT_SENSOR
          if (this->drying_mode_text_sensor_) this->drying_mode_text_sensor_->publish_state("");
#endif
#ifdef USE_SENSOR
          if (this->remaining_time_sensor_) this->remaining_time_sensor_->publish_state(NAN);
          if (this->start_delay_time_sensor_) this->start_delay_time_sensor_->publish_state(NAN);
          if (this->selected_program_number_sensor_) this->selected_program_number_sensor_->publish_state(NAN);
          if (this->wash_temperature_sensor_) this->wash_temperature_sensor_->publish_state(NAN);
          if (this->spin_speed_sensor_) this->spin_speed_sensor_->publish_state(NAN);
          if (this->time_manager_sensor_) this->time_manager_sensor_->publish_state(NAN);
#endif
          break;
        case MACS_APPLIANCE_STATE_IDLE:
        case MACS_APPLIANCE_STATE_PAUSED:
        case MACS_APPLIANCE_STATE_FINISHED:
        case MACS_APPLIANCE_STATE_ERROR:
#ifdef USE_BINARY_SENSOR
          if (this->powered_on_binary_sensor_) this->powered_on_binary_sensor_->publish_state(1);
          if (this->running_binary_sensor_) this->running_binary_sensor_->publish_state(0);
#endif
          break;
        case MACS_APPLIANCE_STATE_RUNNING:
        case MACS_APPLIANCE_STATE_PENDING_START:
#ifdef USE_BINARY_SENSOR
          if (this->powered_on_binary_sensor_) this->powered_on_binary_sensor_->publish_state(1);
          if (this->running_binary_sensor_) this->running_binary_sensor_->publish_state(1);
#endif
          break;
        default:
          break;
      }
#ifdef USE_BINARY_SENSOR
      if (this->door_locked_binary_sensor_) this->door_locked_binary_sensor_->publish_state(door_locked(this->ew8w_, data[5]));
      // EW8W261B: [5] 0x08 = door open. While the machine is off, every open and close
      // starts with a cut frame that the framer resyncs past (see framer.h).
      if (this->door_open_binary_sensor_ && this->ew8w_) this->door_open_binary_sensor_->publish_state((data[5] & 0x08) != 0);
      // EW8W261B, checked over a full wash + dry cycle: [5] 0x04 follows the inverter speed,
      // [5] 0x20 is set from fill until drain, [7] 0x80 pulses at every drain and through the spin
      if (this->drum_turning_binary_sensor_) this->drum_turning_binary_sensor_->publish_state((data[5] & 0x04) != 0);
      if (this->water_in_drum_binary_sensor_) this->water_in_drum_binary_sensor_->publish_state((data[5] & 0x20) != 0);
      if (this->drain_pump_binary_sensor_ && data.size() >= 8)
        this->drain_pump_binary_sensor_->publish_state((data[7] & 0x80) != 0);
#endif
#ifdef USE_SENSOR
      // [4] changes within a phase (08 early in the spin, 10 near the end of drying, 20 when finished); meaning unknown
      if (this->sub_phase_sensor_) this->sub_phase_sensor_->publish_state(data[4]);
#endif
      break;
      
    case MACS_MESSAGE_TYPE_PROGRAM_SET:
      if (data.size() < 13) break;
#ifdef USE_SENSOR
      if (this->wash_temperature_sensor_) this->wash_temperature_sensor_->publish_state((float) data[2]);
      // [3] bit 0x80 is set on washer-dryers when washing is off (dry only); not part of the spin speed
      if (this->spin_speed_sensor_) this->spin_speed_sensor_->publish_state((float) (data[3] & 0x7F) * 50);
      if (this->start_delay_time_sensor_) this->start_delay_time_sensor_->publish_state((float) data[this->ew8w_ ? 11 : 9] * 30);
      if (this->selected_program_number_sensor_) this->selected_program_number_sensor_->publish_state((float) data[12]);
      if (this->time_manager_sensor_) this->time_manager_sensor_->publish_state(
          this->ew8w_ ? time_manager_level_ew8w(data[12], data[4], data[5], data[7]) : time_manager_level(data[5], data[7]));
#endif
#ifdef USE_BINARY_SENSOR
      if (this->easy_iron_binary_sensor_) this->easy_iron_binary_sensor_->publish_state((data[6] & 0x01) != 0);
      if (this->stain_wash_binary_sensor_) this->stain_wash_binary_sensor_->publish_state((data[6] & 0x04) != 0);
      if (this->extra_rinse_binary_sensor_) this->extra_rinse_binary_sensor_->publish_state((data[6] & 0x10) != 0);
      if (this->soft_plus_binary_sensor_) this->soft_plus_binary_sensor_->publish_state((data[6] & 0x20) != 0);
      if (this->pre_wash_enabled_binary_sensor_) this->pre_wash_enabled_binary_sensor_->publish_state((data[7] & 0x80) != 0);
      if (this->anti_crease_binary_sensor_) this->anti_crease_binary_sensor_->publish_state((data[5] & 0x01) != 0);
      // [4] & 0x01 and [3] & 0x80 always change together when washing is turned off; [3] is used
      if (this->washing_enabled_binary_sensor_) this->washing_enabled_binary_sensor_->publish_state((data[3] & 0x80) == 0);
#endif
#ifdef USE_TEXT_SENSOR
      if (this->drying_mode_text_sensor_) this->drying_mode_text_sensor_->publish_state(drying_mode_name(data[8], data[9], this->ew8w_ && fixed_drying_ew8w(data[12])));
#endif
      break;
      
    case MACS_MESSAGE_TYPE_ALARM:
#ifdef USE_TEXT_SENSOR
      if (this->active_alarm_text_sensor_) {
        if(data[2] != 0) {
          sprintf(alarm_buf, "E%02X", data[2] - 1);
          this->active_alarm_text_sensor_->publish_state(alarm_buf, 3);
        } else this->active_alarm_text_sensor_->publish_state(alarm_buf, 0);
      }
#endif
      break;
      
    default:
      break;
  }
}

void ElectroluxWashingMachineMacsComponent::decode_inverter_(uint8_t target, uint8_t source, std::vector<uint8_t> data) {
  if (data.empty()) return;
  int16_t tmp_ = 0;
  if(source == MACS_ID_INVERTER) {
    switch (data[0]) {
      case MACS_MESSAGE_TYPE_INVERTER_STATE:
        if (data.size() < 21) break;
  #ifdef USE_SENSOR
        tmp_ = encode_uint16(data[2], data[3]) & 0x7FFF;
        if (this->current_drum_speed_sensor_) this->current_drum_speed_sensor_->publish_state(((float) tmp_) / motor_drum_ratio_);
        
        tmp_ = encode_uint16(data[19], data[20]);
        tmp_ = (tmp_ & 0x8000) ? (uint16_t)(0u - tmp_) : tmp_;
        if (this->target_drum_speed_sensor_) this->target_drum_speed_sensor_->publish_state(((float) tmp_) / motor_drum_ratio_);
        
        if (this->current_water_temperature_sensor_) this->current_water_temperature_sensor_->publish_state((float) data[6]);
  #endif
        break;
        
      default:
        break;
    }
  } else if(target == MACS_ID_INVERTER) {
    switch (data[0]) {
      case MACS_MESSAGE_TYPE_INVERTER_STATE:
        if (data.size() < 4) break;
  #ifdef USE_SENSOR
        tmp_ = encode_uint16(data[2], data[3]);
        tmp_ = (tmp_ & 0x8000) ? (uint16_t)(0u - tmp_) : tmp_;
        if (this->target_drum_speed_sensor_) this->target_drum_speed_sensor_->publish_state(((float) tmp_) / motor_drum_ratio_);
  #endif
        break;
        
      default:
        break;
    }
  }
}

void ElectroluxWashingMachineMacsComponent::decode_data_(uint8_t target, uint8_t source, std::vector<uint8_t> data) {
  uint8_t endpoint = target;
  if(endpoint == MACS_ID_WM_CONTROLLER) endpoint = source;

  switch (endpoint) {
    case MACS_ID_FRONT_PANEL:
      decode_ui_(target, source, data);
      break;
    case MACS_ID_INVERTER:
      decode_inverter_(target, source, data);
      break;
    default:
      break;
  }
}

void ElectroluxWashingMachineMacsComponent::setup() {
  esphome::electrolux_macs::ElectroluxMacsComponent::setup();
  this->cycle_pref_ = global_preferences->make_preference<CycleTracker>(fnv1_hash("electrolux_wm_cycle"));
  if (!this->cycle_pref_.load(&this->cycle_)) this->cycle_ = CycleTracker{};
  this->set_interval("cycle", 60000, [this]() { this->publish_cycle_(); });
}

#ifdef USE_TIME
// Local time with its UTC offset, e.g. 2026-09-24T14:20:00+03:00 (a timestamp for HA, readable in the web UI).
// ESPHome applies the time zone itself, so the offset is worked out here rather than taken from strftime %z.
std::string ElectroluxWashingMachineMacsComponent::iso_time_(int64_t epoch) {
  ESPTime local = ESPTime::from_epoch_local(epoch);
  ESPTime as_utc = local;
  as_utc.recalc_timestamp_utc();
  return local.strftime("%Y-%m-%dT%H:%M:%S") + utc_offset(as_utc.timestamp - epoch);
}
#endif

void ElectroluxWashingMachineMacsComponent::on_cycle_state_(uint8_t state) {
#ifdef USE_TIME
  if (this->time_ == nullptr) return;
  ESPTime now = this->time_->now();
  if (!now.is_valid()) return;
  if (state == MACS_APPLIANCE_STATE_FINISHED && this->cycle_.active()) {
#ifdef USE_SENSOR
    if (this->elapsed_time_sensor_) this->elapsed_time_sensor_->publish_state(this->cycle_.elapsed_min(now.timestamp));
    if (this->program_progress_sensor_) this->program_progress_sensor_->publish_state(100);
#endif
#ifdef USE_TEXT_SENSOR
    if (this->estimated_end_text_sensor_) this->estimated_end_text_sensor_->publish_state(this->iso_time_(now.timestamp));
#endif
  }
  bool was_delayed = this->cycle_.delayed();
  if (this->cycle_.on_state(state, now.timestamp)) {
    this->cycle_pref_.save(&this->cycle_);
#ifdef USE_SENSOR
    // the wait is over, the program progress starts from 0
    if (was_delayed && this->cycle_.active() && this->start_delay_progress_sensor_)
      this->start_delay_progress_sensor_->publish_state(100);
#endif
    this->publish_cycle_();
  }
  // Clear the values once the machine is switched off or back to setting up a program
  if ((state == MACS_APPLIANCE_STATE_IDLE || state == MACS_APPLIANCE_STATE_STANDBY) && this->cycle_shown_) {
    this->cycle_shown_ = false;
#ifdef USE_SENSOR
    if (this->elapsed_time_sensor_) this->elapsed_time_sensor_->publish_state(NAN);
    if (this->program_progress_sensor_) this->program_progress_sensor_->publish_state(NAN);
    if (this->start_delay_progress_sensor_) this->start_delay_progress_sensor_->publish_state(NAN);
#endif
#ifdef USE_TEXT_SENSOR
    if (this->program_start_text_sensor_) this->program_start_text_sensor_->publish_state("");
    if (this->estimated_end_text_sensor_) this->estimated_end_text_sensor_->publish_state("");
    if (this->wash_start_text_sensor_) this->wash_start_text_sensor_->publish_state("");
#endif
  }
#endif
}

void ElectroluxWashingMachineMacsComponent::publish_cycle_() {
#ifdef USE_TIME
  if (this->time_ == nullptr || !(this->cycle_.active() || this->cycle_.delayed())) return;
  ESPTime now = this->time_->now();
  if (!now.is_valid()) return;
  this->cycle_shown_ = true;
  if (this->cycle_.delayed()) {
    if (std::isnan(this->remaining_delay_min_)) return;
    int64_t wash_start = now.timestamp + (int64_t) (this->remaining_delay_min_ * 60);
#ifdef USE_SENSOR
    if (this->start_delay_progress_sensor_)
      this->start_delay_progress_sensor_->publish_state(
          cycle_progress(this->cycle_.delay_waited_min(now.timestamp), this->remaining_delay_min_));
#endif
#ifdef USE_TEXT_SENSOR
    if (this->wash_start_text_sensor_) this->wash_start_text_sensor_->publish_state(this->iso_time_(wash_start));
    if (this->estimated_end_text_sensor_ && !std::isnan(this->remaining_min_))
      this->estimated_end_text_sensor_->publish_state(this->iso_time_(wash_start + (int64_t) (this->remaining_min_ * 60)));
#endif
    return;
  }
  float elapsed = this->cycle_.elapsed_min(now.timestamp);
#ifdef USE_SENSOR
  if (this->elapsed_time_sensor_) this->elapsed_time_sensor_->publish_state(elapsed);
  if (this->program_progress_sensor_) this->program_progress_sensor_->publish_state(cycle_progress(elapsed, this->remaining_min_));
#endif
#ifdef USE_TEXT_SENSOR
  if (this->program_start_text_sensor_) this->program_start_text_sensor_->publish_state(this->iso_time_(this->cycle_.start));
  if (this->estimated_end_text_sensor_ && !std::isnan(this->remaining_min_))
    this->estimated_end_text_sensor_->publish_state(this->iso_time_(now.timestamp + (int64_t) (this->remaining_min_ * 60)));
#endif
#endif
}

void ElectroluxWashingMachineMacsComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Electrolux Washing Machine MACS");
  esphome::electrolux_macs::ElectroluxMacsComponent::dump_config();
  ESP_LOGCONFIG(TAG, "  Motor to drum ratio = %f", this->motor_drum_ratio_);
  ESP_LOGCONFIG(TAG, "  Model = %s", this->ew8w_ ? "EW8W261B" : "EWX14");
}

}  // namespace electrolux_washing_machine_macs
}  // namespace esphome

