#include <array>
#include <utility>
#include <vector>

#include "m5pm1.hpp"
#include "check.hpp"

TEST("M5PM1: battery voltage and power source read over I2C without writes") {
  std::array<uint8_t, 256> regs{};
  regs[0x04] = 0x01;  // PWR_SRC = USB (5VIN)
  regs[0x22] = 0x0a;  // VBAT low
  regs[0x23] = 0x10;  // VBAT high -> 0x100a = 4106 mV
  std::vector<std::pair<uint8_t, uint8_t>> writes;
  hg::M5Pm1 power(
      [&](uint8_t reg, uint8_t* out, size_t n) {
        for (size_t i = 0; i < n; ++i) out[i] = regs[reg + i];
        return true;
      },
      [&](uint8_t reg, uint8_t value) { writes.emplace_back(reg, value); return true; });
  auto p = power.read();
  CHECK(p.has_value());
  CHECK(p->battery_mv == 4106);
  CHECK(p->external_power == true);
  CHECK(p->battery_present == true);
  CHECK(!p->charging);  // not exposed by this register range
  CHECK(writes.empty());

  // Battery power source (3), and an out-of-range voltage is rejected.
  regs[0x04] = 0x03;
  regs[0x22] = 0xff;
  regs[0x23] = 0xff;  // 0xffff = 65535 mV -> rejected
  p = power.read();
  CHECK(p->external_power == false);
  CHECK(!p->battery_mv);

  // Implausibly low voltage is also rejected.
  regs[0x22] = 0x00;
  regs[0x23] = 0x00;  // 0 mV
  CHECK(!power.read()->battery_mv);
  CHECK(writes.empty());
}

TEST("M5PM1: shutdown issues the keyed SYS_CMD and fails on a read error") {
  std::array<uint8_t, 256> regs{};
  std::vector<std::pair<uint8_t, uint8_t>> writes;
  bool fail_read = false;
  hg::M5Pm1 power(
      [&](uint8_t reg, uint8_t* out, size_t n) {
        if (fail_read) return false;
        for (size_t i = 0; i < n; ++i) out[i] = regs[reg + i];
        return true;
      },
      [&](uint8_t reg, uint8_t value) { writes.emplace_back(reg, value); return true; });

  CHECK(power.power_off());
  CHECK_EQ(writes.size(), size_t(1));
  CHECK_EQ(writes[0].first, uint8_t(0x0C));
  CHECK_EQ(writes[0].second, uint8_t(0xA1));  // key 0xA0 | cmd 0x01 (shutdown)

  fail_read = true;
  CHECK(!power.read());
}
