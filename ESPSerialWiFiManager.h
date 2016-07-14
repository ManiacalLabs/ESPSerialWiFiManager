#ifndef ESP_SERIAL_WIFI_H
#define ESP_SERIAL_WIFI_H

#include <ESP8266WiFi.h>
//#include <EEPROM.h>

// #define ESWM_CONNECTED 0
// #define ESWM_NOCONFIG 1

#define O(t) Serial.print(t)
#define OL(t) Serial.println(t)
#define PROMPT_INPUT_SIZE 256

class ESPSerialWiFiManager {
    public:
        ESPSerialWiFiManager();
        ESPSerialWiFiManager(int eeprom_size);
        ESPSerialWiFiManager(int eeprom_size, int eeprom_offset);

        uint8_t begin();

        void set_init_ap(char const *ssid, char const *password);

        void run_menu();

    private:

        int _eeprom_size = 512;
        int _eeprom_offset = 0;

        String _ssid = "";
        String _pass = "";

        bool _wait_for_wifi();

        String _prompt(String prompt);
        int _prompt_int(String prompt);
};

#endif ESP_SERIAL_WIFI_H
