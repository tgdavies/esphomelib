//
//  deep_sleep_component.cpp
//  esphomelib
//
//  Created by Otto Winter on 15.04.18.
//  Copyright © 2018 Otto Winter. All rights reserved.
//

#include <Esp.h>
#include "esphomelib/deep_sleep_component.h"
#include "esphomelib/log.h"
#include "esphomelib/helpers.h"
#include "esphomelib/ota_component.h"

#ifdef USE_DEEP_SLEEP

ESPHOMELIB_NAMESPACE_BEGIN

static const char *TAG = "deep_sleep";

void DeepSleepComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Deep Sleep...");
  if (this->sleep_duration_.has_value())
    ESP_LOGCONFIG(TAG, "  Sleep Duration: %llu ms", *this->sleep_duration_ / 1000);
  if (this->run_duration_.has_value())
    ESP_LOGCONFIG(TAG, "  Run Duration: %u ms", *this->run_duration_);
  if (this->loop_cycles_.has_value())
    ESP_LOGCONFIG(TAG, "  Loop Cycles: %u", *this->loop_cycles_);
#ifdef ARDUINO_ARCH_ESP32
  if (this->wakeup_pin_.has_value())
    ESP_LOGCONFIG(TAG, "  Wakeup Pin: %u %s", this->wakeup_pin_->get_pin(), this->wakeup_pin_->is_inverted() ? "LOW" : "HIGH");
#endif
  if (this->run_duration_.has_value())
    this->set_timeout(*this->run_duration_, [this](){ this->begin_sleep(); });
}
void DeepSleepComponent::loop() {
  if (this->loop_cycles_.has_value()) {
    if (++this->at_loop_cycle_ >= *this->loop_cycles_)
      this->begin_sleep();
  }
}
float DeepSleepComponent::get_loop_priority() const {
  return -100.0f; // run after everything else is ready
}
void DeepSleepComponent::set_sleep_duration(uint32_t time_ms) {
  this->sleep_duration_ = uint64_t(time_ms) * 1000;
}
#ifdef ARDUINO_ARCH_ESP32
void DeepSleepComponent::set_wakeup_pin(GPIOInputPin pin) {
  this->wakeup_pin_ = pin;
}
#endif
void DeepSleepComponent::set_run_cycles(uint32_t cycles) {
  this->loop_cycles_ = cycles;
}
void DeepSleepComponent::set_run_duration(uint32_t time_ms) {
  this->run_duration_ = time_ms;
}
void DeepSleepComponent::begin_sleep() {
  ESP_LOGI(TAG, "Beginning Deep Sleep");

  run_safe_shutdown_hooks("deep-sleep");

#ifdef ARDUINO_ARCH_ESP32
  if (this->sleep_duration_.has_value())
    esp_sleep_enable_timer_wakeup(*this->sleep_duration_);
  if (this->wakeup_pin_.has_value())
    esp_sleep_enable_ext0_wakeup(gpio_num_t(this->wakeup_pin_->get_pin()),
                                 !this->wakeup_pin_->is_inverted());
  esp_deep_sleep_start();
#endif

#ifdef ARDUINO_ARCH_ESP8266
  ESP.deepSleep(*this->sleep_duration_);
#endif
}

ESPHOMELIB_NAMESPACE_END

#endif //USE_DEEP_SLEEP
