#include "electrolux_dryer_macs.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace electrolux_dryer_macs {

static const char *const TAG = "electrolux_dryer_macs";

static const uint8_t MACS_ID_DRYER_CONTROLLER = 0x22;
static const uint8_t MACS_ID_DRYER_FRONT_PANEL = 0x2A;

static const uint8_t MACS_MESSAGE_MARKER = 0xC9;
static const uint8_t MACS_ACK_MARKER = 0x98;

static const uint8_t MACS_MESSAGE_TYPE_DRYER_PROGRAM_SET = 0x50;
static const uint8_t MACS_MESSAGE_TYPE_DRYER_PROGRAM = 0x55;
static const uint8_t MACS_MESSAGE_TYPE_DRYER_STATE_SET = 0x51;
static const uint8_t MACS_MESSAGE_TYPE_DRYER_STATE = 0x52;
static const uint8_t MACS_MESSAGE_TYPE_DRYER_REMAINING_TIME = 0x56;
static const uint8_t MACS_MESSAGE_TYPE_DRYER_HEARTBEAT = 0x5F;

static const uint8_t MACS_DRYER_STATE_STANDBY = 0x0B;
static const uint8_t MACS_DRYER_STATE_IDLE = 0x01;
static const uint8_t MACS_DRYER_STATE_DRYING = 0x02;
static const uint8_t MACS_DRYER_STATE_FINISHED = 0x03;
static const uint8_t MACS_DRYER_STATE_PAUSED = 0x04;

static const uint8_t MACS_DRYER_DRYING_PHASE_IDLE = 0x0B;
static const uint8_t MACS_DRYER_DRYING_PHASE_HEATING = 0x01;
static const uint8_t MACS_DRYER_DRYING_PHASE_COOLING = 0x02;
static const uint8_t MACS_DRYER_DRYING_PHASE_ANTI_CREASE = 0x04;


std::string print_vector_hex(std::vector<uint8_t> bytes) {
  std::string res;
  size_t len = bytes.size();
  char buf[5] = {0};
  for (size_t i = 0; i < len; i++) {
    if (i > 0) res += ' ';
    sprintf(buf, "%02X", bytes[i]);
    res += buf;
  }
  return res;
}


void ElectroluxDryerMacsComponent::setup() {}

void ElectroluxDryerMacsComponent::loop() {
  const uint32_t now = App.get_loop_component_start_time();
  
  if (this->receiving_ && (now - this->last_transmission_ >= this->receive_timeout_)) {
    ESP_LOGW(TAG, "Last transmission too long ago. Reset RX index.");
    this->data_.clear();
    this->receiving_ = false;
  }
  
  if (available()) this->last_transmission_ = now;
  
  while (available()) {
    uint8_t c;
    read_byte(&c);
    if (!this->receiving_) {
      if (c != MACS_MESSAGE_MARKER && c != MACS_ACK_MARKER)
        continue;
      this->receiving_ = true;
    }
    this->data_.push_back(c);
    
    switch (this->data_[0]) {
      case MACS_ACK_MARKER:
        if (this->data_.size() == 3) {
          this->data_.clear();
          this->receiving_ = false;
        }
        break;
      case MACS_MESSAGE_MARKER:
        if (this->data_.size() == 4)
          this->data_count_ = c;
        if ((this->data_.size() > 4) and (data_.size() == this->data_count_ + 5)) {
          this->decode_data_(this->data_);
          this->data_.clear();
          this->receiving_ = false;
        }
        break;
    }
    
  }
}

void ElectroluxDryerMacsComponent::decode_data_(std::vector<uint8_t> frame) {  
  uint8_t marker_ = frame[0];
  uint8_t target_ = frame[1];
  uint8_t source_ = frame[2];
  uint8_t length_ = frame[3];
  
  if (length_ == 0) return;
  if (5 + length_ < frame.size()) return;
  std::vector<uint8_t> data(&frame[4], &frame[4 + length_]);
  uint8_t checksum_ = frame[4 + length_]; // Last byte
  
  // TODO: Check checksum

  ESP_LOGD(TAG, "Received MSG from %X, to %X, data: %s", source_, target_, print_vector_hex(data).c_str());

  uint8_t msg_type_ = data[0];
  uint16_t tmp_ = 0;
  switch (msg_type_) {
  
    case MACS_MESSAGE_TYPE_DRYER_REMAINING_TIME:
#ifdef USE_SENSOR
      tmp_ = encode_uint16(data[3], data[4]);
      if (this->remaining_time_sensor_) {
        if (tmp_ == 65535) this->remaining_time_sensor_->publish_state(NAN);
        else this->remaining_time_sensor_->publish_state((float) tmp_);
      }
#endif
      break;
      
    case MACS_MESSAGE_TYPE_DRYER_STATE:
      switch (data[2]) {
        case MACS_DRYER_STATE_STANDBY:
#ifdef USE_BINARY_SENSOR
          if (this->powered_on_binary_sensor_) this->powered_on_binary_sensor_->publish_state(0);
          if (this->running_binary_sensor_) this->running_binary_sensor_->publish_state(0);
          if (this->delicate_mode_binary_sensor_) this->delicate_mode_binary_sensor_->publish_state(0);
          if (this->heating_binary_sensor_) this->heating_binary_sensor_->publish_state(0);
#endif
#ifdef USE_SENSOR
          if (this->remaining_time_sensor_) this->remaining_time_sensor_->publish_state(NAN);
          if (this->program_dryness_level_sensor_) this->program_dryness_level_sensor_->publish_state(NAN);
          if (this->start_delay_time_sensor_) this->start_delay_time_sensor_->publish_state(NAN);
          if (this->selected_program_number_sensor_) this->selected_program_number_sensor_->publish_state(NAN);
#endif
          break;
        case MACS_DRYER_STATE_IDLE:
        case MACS_DRYER_STATE_PAUSED:
        case MACS_DRYER_STATE_FINISHED:
#ifdef USE_BINARY_SENSOR
          if (this->powered_on_binary_sensor_) this->powered_on_binary_sensor_->publish_state(1);
          if (this->running_binary_sensor_) this->running_binary_sensor_->publish_state(0);
          if (this->heating_binary_sensor_) this->heating_binary_sensor_->publish_state(0);
#endif
          break;
        case MACS_DRYER_STATE_DRYING:
#ifdef USE_BINARY_SENSOR
          if (this->powered_on_binary_sensor_) this->powered_on_binary_sensor_->publish_state(1);
          if (this->running_binary_sensor_) this->running_binary_sensor_->publish_state(1);
          if (this->heating_binary_sensor_) this->heating_binary_sensor_->publish_state(data[3] == MACS_DRYER_DRYING_PHASE_HEATING);
#endif
          break;
        default:
          break;
      }
      break;
      
    case MACS_MESSAGE_TYPE_DRYER_PROGRAM_SET:
#ifdef USE_SENSOR
      if (this->program_dryness_level_sensor_) this->program_dryness_level_sensor_->publish_state((float) (data[2] & 0x0F));
      if (this->start_delay_time_sensor_) this->start_delay_time_sensor_->publish_state((float) encode_uint16(0, data[7])*30);
      if (this->selected_program_number_sensor_) this->selected_program_number_sensor_->publish_state((float) data[9]);
#endif
#ifdef USE_BINARY_SENSOR
      if (this->delicate_mode_binary_sensor_) this->delicate_mode_binary_sensor_->publish_state((data[4] & 0x02) != 0);
#endif
      break;
      
    default:
      break;
  }
}

void ElectroluxDryerMacsComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Electrolux Dryer MACS");
  ESP_LOGCONFIG(TAG, "  Receive timeout: %d", this->receive_timeout_);
  check_uart_settings(9600, 1, esphome::uart::UART_CONFIG_PARITY_EVEN, 8);
}

}  // namespace electrolux_dryer_macs
}  // namespace esphome

