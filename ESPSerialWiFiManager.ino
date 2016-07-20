#include <ESP8266WiFi.h>
#include "ESPSerialWiFiManager.h"

ESPSerialWiFiManager eswp = ESPSerialWiFiManager();

void setup(){
    Serial.begin(115200);

    // eswp.set_init_ap("SSID", "Password");
    eswp.begin();
}

void loop(){
    eswp.run_menu();
    Serial.println("Loop...");
    delay(1000);
}
