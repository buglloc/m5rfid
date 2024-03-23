#include <Arduino.h>
#include <M5Unified.h>
#include <Adafruit_PN532.h>


extern const uint8_t img_lock_open[] asm("_binary_assets_lock_open_qoi_start");
extern const uint8_t img_lock_close[] asm("_binary_assets_lock_close_qoi_start");

static uint8_t last_uid[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static uint8_t last_uid_len = 0;
static bool last_have_uid = false;
static uint32_t update_delay = 8;
static Adafruit_PN532 nfc(-1, -1);

bool is_have_uid()
{
  last_uid_len = 0;
  if (!nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A)) {
    return false;
  }

  bool ok = nfc.readDetectedPassiveTargetID(last_uid, &last_uid_len);
  return ok && last_uid_len > 0;
}

void draw_lock(bool have_uid)
{
  if (have_uid) {
    M5.Display.drawQoi(
      img_lock_open,  // data_pointer
      ~0u,  // data_length (~0u = auto)
      0,    // X position
      0,    // Y position
      M5.Display.width(),  // Width
      M5.Display.height(), // Height
      0,    // X offset
      0,    // Y offset
      1.0,  // X magnification(default = 1.0 , 0 = fitsize , -1 = follow the Y magni)
      1.0,  // Y magnification(default = 1.0 , 0 = fitsize , -1 = follow the X magni)
      datum_t::middle_center
    );

    M5.Display.setCursor(12, 12);
    for (uint8_t i = 0; i < last_uid_len; ++i) {
      M5.Display.printf("%02X", last_uid[i] & 0xff);
      if (i < last_uid_len-1) {
        M5.Display.print(" ");
      }
    }
    return;
  }

  M5.Display.drawQoi(
    img_lock_close,  // data_pointer
    ~0u,  // data_length (~0u = auto)
    0,    // X position
    0,    // Y position
    M5.Display.width(),  // Width
    M5.Display.height(), // Height
    0,    // X offset
    0,    // Y offset
    1.0,  // X magnification(default = 1.0 , 0 = fitsize , -1 = follow the Y magni)
    1.0,  // Y magnification(default = 1.0 , 0 = fitsize , -1 = follow the X magni)
    datum_t::middle_center
  );
}

void setup(void)
{
  auto cfg = M5.config();
  cfg.serial_baudrate = 9600;
  cfg.clear_display = true;
  M5.begin(cfg);

  /// For models with LCD : backlight control (0~255)
  M5.Display.setBrightness(128);
  draw_lock(false);

  Wire.begin(0, 26, 100000UL);
  nfc.begin();

  uint32_t version_data = nfc.getFirmwareVersion();
  if (!version_data) {
    M5_LOGE("Didn't find PN53x board");
    while (1); // halt
  }

  M5_LOGI("Found chip PN5: 0x%02X", (version_data>>24) & 0xFF);
  M5_LOGI("Firmware ver. %d.%d", (version_data>>16) & 0xFF, (version_data>>8) & 0xFF);
}

void loop(void)
{
  M5.delay(update_delay);
  M5.update();

  bool have_uid = is_have_uid();
  if (have_uid == last_have_uid) {
    return;
  }
  last_have_uid = have_uid;

  M5.Display.clear();
  draw_lock(have_uid);
  update_delay = have_uid ? 2000 : 8;
}
