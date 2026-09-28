// g++ -std=c++17 -I components tests/framer_test.cpp -o /tmp/framer_test && /tmp/framer_test
// Byte streams rebuilt from EW8W261B captures with the machine off, door closed and opened.
#include <cassert>
#include <cstdio>
#include "electrolux_macs/framer.h"
using namespace esphome::electrolux_macs;

static std::vector<uint8_t> with_checksum(std::vector<uint8_t> f) {
  f.push_back(0);
  f.back() = macs_checksum(f);
  return f;
}

static std::vector<MacsFrame> run(const std::vector<uint8_t> &stream) {
  MacsFramer framer;
  std::vector<MacsFrame> out;
  for (uint8_t c : stream) framer.feed(c, out);
  assert(!framer.receiving());
  return out;
}

int main() {
  const std::vector<uint8_t> cut = {0xC9, 0x2A, 0x21, 0x09, 0x52, 0x00, 0x0B, 0x00, 0x00};
  const auto state_closed = with_checksum({0xC9, 0x2A, 0x21, 0x09, 0x52, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x40, 0x2C});
  const auto heartbeat = with_checksum({0xC9, 0x00, 0x2B, 0x03, 0x10, 0x00, 0x20});

  // Door closed, cut frame followed by the resent state frame. Logged as
  // "data: 52 00 0B 00 00 C9 2A 21 09" + "Checksum error (52 != 59)", and the close was lost
  std::vector<uint8_t> stream = cut;
  stream.insert(stream.end(), state_closed.begin(), state_closed.end());
  auto frames = run(stream);
  assert(frames.size() == 2);
  assert(!frames[0].checksum_ok);
  assert(frames[0].bytes.back() == 0x52 && macs_checksum(frames[0].bytes) == 0x59);  // same error as the log
  assert(frames[1].checksum_ok && frames[1].bytes == state_closed);

  // Cut frame followed by the heartbeat of 2B, then the resent state frame: nothing lost
  stream = cut;
  stream.insert(stream.end(), heartbeat.begin(), heartbeat.end());
  stream.insert(stream.end(), state_closed.begin(), state_closed.end());
  frames = run(stream);
  assert(frames.size() == 3);
  assert(!frames[0].checksum_ok);
  assert(frames[1].checksum_ok && frames[1].bytes == heartbeat);
  assert(frames[2].checksum_ok && frames[2].bytes == state_closed);

  // Acks are skipped, clean frames pass untouched
  stream = {0x98, 0x2A, 0x21};
  stream.insert(stream.end(), state_closed.begin(), state_closed.end());
  frames = run(stream);
  assert(frames.size() == 1 && frames[0].checksum_ok);

  // Checksum check off: the bad frame is passed on as is, no resync
  MacsFramer framer;
  framer.verify_checksum = false;
  std::vector<MacsFrame> out;
  for (uint8_t c : cut) framer.feed(c, out);
  for (uint8_t c : state_closed) framer.feed(c, out);
  assert(out.size() == 1 && out[0].checksum_ok);

  puts("framer ok");
  return 0;
}
