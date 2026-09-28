#include "electrolux_macs.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace electrolux_macs {

static const char *const TAG = "electrolux_macs";

std::string ElectroluxMacsComponent::print_vector_hex(std::vector<uint8_t> bytes) {
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


void ElectroluxMacsComponent::setup() {}

void ElectroluxMacsComponent::loop() {
  const uint32_t now = App.get_loop_component_start_time();
  
  if (this->framer_.receiving() && (now - this->last_transmission_ >= this->receive_timeout_)) {
    ESP_LOGW(TAG, "Last transmission too long ago. Reset RX index.");
    this->framer_.reset();
  }
  
  if (available()) this->last_transmission_ = now;
  
  std::vector<MacsFrame> frames;
  while (available()) {
    uint8_t c;
    read_byte(&c);
    this->framer_.feed(c, frames);
  }
  for (auto &frame : frames) this->process_data_(frame);
}

void ElectroluxMacsComponent::process_data_(const MacsFrame &frame) {
	const std::vector<uint8_t> &bytes = frame.bytes;
	uint8_t target_ = bytes[1];
	uint8_t source_ = bytes[2];
	uint8_t length_ = bytes[3];

	if (length_ == 0) return;
	std::vector<uint8_t> data(&bytes[4], &bytes[4 + length_]);

	ESP_LOGD(TAG, "Received MSG from %X, to %X, data: %s", source_, target_, print_vector_hex(data).c_str());

	if (!frame.checksum_ok) {
		ESP_LOGW(TAG, "Checksum error (%X != %X), skipping 1 frame", bytes.back(), macs_checksum(bytes));
		return;
	}
	
	this->decode_data_(target_, source_, data);
}

void ElectroluxMacsComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "  Receive timeout: %d", this->receive_timeout_);
  ESP_LOGCONFIG(TAG, "  Verify checksum: %s", this->framer_.verify_checksum ? "true" : "false");
  check_uart_settings(9600, 1, esphome::uart::UART_CONFIG_PARITY_EVEN, 8);
}

}  // namespace electrolux_macs
}  // namespace esphome

