#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include "esphome/core/defines.h"

#include <vector>

namespace esphome {
namespace electrolux_dryer_macs {

class ElectroluxDryerMacsComponent : public Component, public uart::UARTDevice {
 public:
  ElectroluxDryerMacsComponent(uart::UARTComponent *uart) : uart::UARTDevice(uart) {}

#ifdef USE_SENSOR
  SUB_SENSOR(remaining_time)
  SUB_SENSOR(start_delay_time)
  SUB_SENSOR(selected_program_number)
  SUB_SENSOR(program_dryness_level)
#endif

#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(last_raw_message)
#endif

#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(powered_on)
  SUB_BINARY_SENSOR(running)
  SUB_BINARY_SENSOR(delicate_mode)
#endif

  void setup() override;
  void loop() override;
  void dump_config() override;
  
  // float get_setup_priority() const override { return esphome::setup_priority::HARDWARE_LATE; }

  void set_crc_check(bool crc_check) { this->crc_check_ = crc_check; }
  void set_receive_timeout(uint32_t receive_timeout) { this->receive_timeout_ = receive_timeout; }

 protected:
  void decode_data_(std::vector<uint8_t> data);
  
  std::vector<uint8_t> data_;
  bool receiving_{false};
  uint8_t data_count_{0};
  uint32_t last_transmission_{0};
  
  uint32_t receive_timeout_{200};
  bool crc_check_{false};

};
}  // namespace electrolux_dryer_macs
}  // namespace esphome

