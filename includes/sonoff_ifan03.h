#pragma once

#include "esphome.h"

using namespace esphome;

namespace IFan03 {

/* IFan03: Remote Control Codes
 *
 * btn  message                    meaning
 *       0  1  2  3  4  5  6  7
 * ==== ========================  ===================================
 *      AA 55 01 01 00 01 01 04    Wifi long press - start wifi setup
 *      AA 55 01 01 00 01 02 05    Rf and Wifi short press
 *               ||       ||
 *  0   AA 55 01 04 00 01 00 06    Fan 0
 *  1   AA 55 01 04 00 01 01 07    Fan 1
 *  2   AA 55 01 04 00 01 02 08    Fan 2
 *  3   AA 55 01 04 00 01 03 09    Fan 3
 *  4   AA 55 01 04 00 01 04 0A    Light
 *               ||       |
 *  5   AA 55 01 06 00 01 01 09   Buzzer
 *               ||       ||
 *      AA 55 01 07 00 01 01 0A   Rf long press - forget RF codes
 *
 * https://github.com/arendst/Tasmota/blob/development/tasmota/xdrv_22_sonoff_ifan.ino
 */

class RemoteReceiver : public Component, public uart::UARTDevice {
public:
  static constexpr const char *TAG = "ifan03.remote_receiver";

  std::vector<BinarySensor *> buttons = {{
      new BinarySensor(), // button_fan_off
      new BinarySensor(), // button_fan_low
      new BinarySensor(), // button_fan_medium
      new BinarySensor(), // button_fan_high
      new BinarySensor(), // button_light
      new BinarySensor()  // button_buzzer
  }};

  RemoteReceiver(UARTComponent *parent) : UARTDevice(parent) {}

  void setup() override {
    // nothing to do here
  }

  void loop() override {

    while (available()) {
      if (_pos == 0 && peek() != 0xAA)
        continue;
      if (_pos == 1 && peek() != 0x55) {
        _pos = 0;
        continue;
      }

      _buffer[_pos++] = read();

      if (_pos >= 8) {
        _parse();
        _pos = 0;
      }
    }
  }

protected:
  void _parse() {
    ESP_LOGD(TAG, "%02X %02X %02X %02X %02X %02X %02X %02X", _buffer[0],
             _buffer[1], _buffer[2], _buffer[3], _buffer[4], _buffer[5],
             _buffer[6], _buffer[7]);

    uint8_t crc = 0;
    for (size_t i = 2; i < 7; ++i)
      crc += _buffer[i];
    if (crc != _buffer[7]) {
      ESP_LOGD(TAG, "crc failed");
      return;
    }

    _send_ack();
    _handle(_buffer[3], _buffer[6]);
  }

  void _handle(uint8_t mode, uint8_t action) {
    if (mode == 4 && action < 5) {
      buttons[action]->publish_state(true);
      buttons[action]->publish_state(false);
    } else if (mode == 6) {
      buttons[5]->publish_state(true);
      buttons[5]->publish_state(false);
    } else {
      ESP_LOGW(TAG, "button ignored: mode=%d, action=%d", mode, action);
    }
  }

  void _send_ack() {
    // Send Acknowledge - Copy first 5 bytes, reset byte 6 and store crc in byte
    // 7 AA 55 01 04 00 00 05
    uint8_t crc = 0;
    for (size_t i = 0; i < 5; ++i) {
      if (i >= 2)
        crc += _buffer[i];
      write(_buffer[i]);
    }
    write(0); // ACK (does not change crc)
    write(crc);
  }

private:
  std::array<uint8_t, 8> _buffer{};
  size_t _pos{};
};

/* IFan03 output:
 *
 *  GPIO_REL2,        // GPIO14 WIFI_O1 Relay 2 (0 = Off, 1 = On) controlling
 * the fan GPIO_REL3,        // GPIO12 WIFI_O2 Relay 3 (0 = Off, 1 = On)
 * controlling the fan GPIO_REL4,        // GPIO15 WIFI_O3 Relay 4 (0 = Off, 1 =
 * On) controlling the fan
 *
 *  Speeds:
 *      Relays 432x  0=LOW, 1=HIGH
 *             000x off
 *             001x low
 *             011x med
 *             100x high  // different than iFan02 101x
 *
 * https://github.com/arendst/Tasmota/blob/development/tasmota/xdrv_22_sonoff_ifan.ino
 */

class FanOutput : public Component, public output::FloatOutput {
public:
  static constexpr const char *TAG = "ifan03.fan_output";

  void setup() override
  {
    pinMode(14, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(15, OUTPUT);
  }

  void write_state(float state) override {
    if (state >= 5./6)  // 1.0
    {
      digitalWrite(14, LOW); // different than iFan02
      digitalWrite(12, LOW);
      digitalWrite(15, HIGH);
      ESP_LOGD(TAG, "high, relays 0 0 1");
    }
    else if (state >= 3./6)  // 0.66
    {
      digitalWrite(14, HIGH);
      digitalWrite(12, HIGH);
      digitalWrite(15, LOW);
      ESP_LOGD(TAG, "medium, relays 1 1 0");
    }
    else if (state >= 1./6)  // 0.33
    {
      digitalWrite(14, HIGH);
      digitalWrite(12, LOW);
      digitalWrite(15, LOW);
      ESP_LOGD(TAG, "low, relays 1 0 0");
    }
    else  // 0.0
    {
      digitalWrite(14, LOW);
      digitalWrite(12, LOW);
      digitalWrite(15, LOW);
      ESP_LOGD(TAG, "off, relays 0 0 0");
    }
  }
};

} // namespace IFan03