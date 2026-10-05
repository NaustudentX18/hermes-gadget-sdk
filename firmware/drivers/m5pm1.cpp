#include "m5pm1.hpp"

namespace hg {

// Power source values (PWR_SRC[2:0], M5PM1 manual v1.9): 0 = none, 1 = 5VIN
// (USB/DC), 2 = 5VINOUT, 3 = battery. "2" is only meaningful when the port is
// wired bidirectionally; treat it as external power here since the device is
// being powered rather than running on the cell.
std::optional<PowerStatus> M5Pm1::read() {
  uint8_t src = 0;
  uint8_t vbat[2] = {};
  if (!read_(kRegPwrSrc, &src, 1)) return std::nullopt;
  if (!read_(kRegVbatL, vbat, 2)) return std::nullopt;

  const uint8_t power_src = src & 0x07;
  PowerStatus out;
  out.external_power = (power_src == 1) || (power_src == 2);
  out.battery_present = true;  // the M5PM1 has no separate presence flag
  const uint16_t mv = static_cast<uint16_t>((vbat[1] << 8) | vbat[0]);
  // VBAT is reported in mV. Reject implausible readings (below 2000 mV the cell
  // is effectively dead; above 5000 mV indicates a bad read or no cell).
  if (mv >= 2000 && mv <= 5000) out.battery_mv = mv;
  // The M5PM1 does not expose charging state directly through a status register
  // in this register range; leave `charging` unset rather than guess.
  return out;
}

bool M5Pm1::power_off() {
  // SYS_CMD: key 0xA in bits [7:4], command 0x01 (shutdown) in bits [1:0].
  return write_(kRegSysCmd, kSysCmdShutdown);
}

}  // namespace hg
