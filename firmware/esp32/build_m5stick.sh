#!/bin/bash
# Build the HermesOnaStick M5Stick S3 firmware.
# Run: env -u VIRTUAL_ENV -u VIRTUAL_ENV_PROMPT bash /Volumes/AI1TB/dev/hermes-gadget-sdk/firmware/esp32/build_m5stick.sh
set -uo pipefail
unset VIRTUAL_ENV VIRTUAL_ENV_PROMPT

IDF=/Volumes/AI1TB/toolchains/esp-idf
ENVPY=$(ls -d /Users/forest/.espressif/python_env/idf5.3_py*/bin/python 2>/dev/null | head -1)
export IDF_PATH="$IDF"

# Put IDF tools on PATH (compiler, esptool, etc.)
. "$IDF/export.sh" >/dev/null 2>&1

cd "$IDF/../../dev/hermes-gadget-sdk/firmware/esp32" 2>/dev/null || cd /Volumes/AI1TB/dev/hermes-gadget-sdk/firmware/esp32

"$ENVPY" "$IDF/tools/idf.py" -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;boards/m5stick-s3/sdkconfig.defaults" set-target esp32s3 build
