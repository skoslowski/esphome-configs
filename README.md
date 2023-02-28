# ESPHome Configs

Various configs for [ESPHome](https://esphome.io/) deployed as

* Smart-Plug & temperature sensor
* Wall switch
* Ceiling fan control

## Hardware

* [Sonoff Basic R2](https://sonoff.tech/product/wifi-diy-smart-switches/basicr2)
* [Sonoff Mini](https://sonoff.tech/product/wifi-diy-smart-switches/sonoff-mini)
* [Sonoff IFan03](https://sonoff.tech/product/wifi-diy-smart-switches/ifan03)

## Getting started

```shell
pipenv install
pipenv run esphome CONFIG.yaml compile
```
Details see [Getting Started with ESPHome](https://esphome.io/guides/getting_started_command_line.html)

## OTA tips

OpenWRT WiFi settings

* set "Operating frequency", "Mode" to "Legacy"
* enable "Allow legacy 802.11b rates"
* deactivate "Disassociate On Low Acknowledgement"
* (disable wifi on repeaters)

## From legacy firmware

python3 ~/code/hass/sonoff-mqtt-switch/espota.py -i 192.168.11.100 -a ... -f outside_light/.pioenvs/outside_light/firmware.bin
