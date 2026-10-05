#pragma once

#include <functional>
#include <utility>

#include "hg/hal.hpp"

namespace hg {

// M5Stack M5PM1 power-management companion (the "PM1" / PY32-based PMIC found
// on the M5Stick S3), addressed at 0x6E on the internal I2C bus. Register map
// follows the M5PM1 Chip User Manual v1.9 and M5Stack's M5PM1 driver library
// (https://github.com/m5stack/M5PM1, MIT). The port owns I2C.
//
// Unlike the AXP2101, the M5PM1 exposes the battery voltage and power source
// directly as mV registers (VBAT at 0x22/0x23, PWR_SRC at 0x04) and performs
// shutdown through a keyed system command (SYS_CMD at 0x0C).
class M5Pm1 final : public Power {
 public:
  using Read = std::function<bool(uint8_t, uint8_t*, size_t)>;
  using Write = std::function<bool(uint8_t, uint8_t)>;
  M5Pm1(Read read, Write write) : read_(std::move(read)), write_(std::move(write)) {}
  std::optional<PowerStatus> read() override;
  bool power_off() override;

 private:
  static constexpr uint8_t kRegPwrSrc = 0x04;
  static constexpr uint8_t kRegSysCmd = 0x0C;
  static constexpr uint8_t kRegVbatL = 0x22;
  static constexpr uint8_t kRegVbatH = 0x23;
  static constexpr uint8_t kSysCmdShutdown = 0xA1;  // key 0xA0 | cmd 0x01

  Read read_;
  Write write_;
};

}  // namespace hg
