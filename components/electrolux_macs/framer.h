#pragma once
// Splits the MACS byte stream into frames. No ESPHome dependencies, so it has a host test (tests/framer_test.cpp).

#include <cstdint>
#include <vector>

namespace esphome {
namespace electrolux_macs {

struct MacsFrame {
  std::vector<uint8_t> bytes;  // marker, target, source, length, data..., checksum
  bool checksum_ok;
};

// XOR of every byte except the last one (the checksum itself)
inline uint8_t macs_checksum(const std::vector<uint8_t> &frame) {
  uint8_t checksum = 0;
  for (size_t i = 0; i + 1 < frame.size(); i++) checksum ^= frame[i];
  return checksum;
}

class MacsFramer {
 public:
  static const uint8_t MESSAGE_MARKER = 0xC9;
  static const uint8_t ACK_MARKER = 0x98;

  bool verify_checksum{true};

  bool receiving() const { return !this->buf_.empty(); }
  void reset() { this->buf_.clear(); }

  // Appends every frame completed by this byte to out
  void feed(uint8_t c, std::vector<MacsFrame> &out) {
    if (this->buf_.empty() && c != MESSAGE_MARKER && c != ACK_MARKER) return;
    this->buf_.push_back(c);
    if (this->buf_[0] == ACK_MARKER) {
      if (this->buf_.size() == 3) this->buf_.clear();
      return;
    }
    if (this->buf_.size() < 5 || this->buf_.size() != this->buf_[3] + 5u) return;
    std::vector<uint8_t> frame;
    frame.swap(this->buf_);
    bool ok = !this->verify_checksum || macs_checksum(frame) == frame.back();
    out.push_back({frame, ok});
    if (ok) return;
    // EW8W261B, machine off: the first frame after the door moves is cut short and sent again right away.
    // The cut frame swallows the start of the next one, so without this the resent frame is lost too.
    // Parse again from the next marker inside the bad frame.
    // ponytail: a stray 0xC9 in a corrupted frame can start a false frame; only the 8-bit checksum guards it
    for (size_t i = 1; i < frame.size(); i++) {
      if (frame[i] != MESSAGE_MARKER) continue;
      for (size_t j = i; j < frame.size(); j++) this->feed(frame[j], out);
      break;
    }
  }

 protected:
  std::vector<uint8_t> buf_;
};

}  // namespace electrolux_macs
}  // namespace esphome
