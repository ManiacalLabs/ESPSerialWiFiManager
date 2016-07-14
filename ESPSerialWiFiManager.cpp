#include "ESPSerialWiFiManager.h"

ESPSerialWiFiManager::ESPSerialWiFiManager() {}

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size)
{ _eeprom_size = eeprom_size; }

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size, int eeprom_offset)
{ _eeprom_size = eeprom_size; _eeprom_offset = eeprom_offset; }

void ESPSerialWiFiManager::set_init_ap(char const *ssid, char const *pass)
{ _ssid = String(ssid); _pass = String(pass); }

uint8_t ESPSerialWiFiManager::begin(){
    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect();
    // delay(100);
    // WiFi.begin(_ssid.c_str(), _pass.c_str());
    // if(_wait_for_wifi())
    //     Serial.println("WiFi Connected!");
    // else
    //     Serial.println("Unable to connect to WiFi!");
}

bool ESPSerialWiFiManager::_wait_for_wifi(){
    int c = 0;
    Serial.println("Connecting to " + _ssid);
    while ( c < 20 ) {
        if (WiFi.status() == WL_CONNECTED) { return true; }
        delay(500);
        Serial.print(WiFi.status());
        Serial.print(".");
        c++;
    }
    return false;
}

void _flush_serial(){
    while(Serial.available() > 0){
        Serial.read();
    }
}

String ESPSerialWiFiManager::_prompt(String prompt){
    static char cmd[PROMPT_INPUT_SIZE];
    static int count;
    static char tmp;
    memset(cmd, 0, PROMPT_INPUT_SIZE);
    count = 0;

    O(prompt.c_str());
    O("> ");

    while(true){
        if(Serial.available() > 0){
            tmp = Serial.read();
            if(tmp != '\n' && tmp != '\r'){
                cmd[count] = tmp;
                count++;
            }
            else{
                _flush_serial();
                return String(cmd);
            }
        }
        delay(1);
    }
}

int ESPSerialWiFiManager::_prompt_int(String prompt){
    String res = _prompt(prompt);
    int res_i = res.toInt();
    // if(res_i == 0 && res[0] != 0)
    //     res_i = NULL;
    //TODO: Menu System doesn't need 0, I think?
    return res_i;
}

#define MAIN_MENU_SIZE 2
static const char * _main_menu[MAIN_MENU_SIZE] = {
    "Scan",
    "Manual Config"
};

void ESPSerialWiFiManager::run_menu(){
    static int i; //for loops
    OL("");
    OL("ESP Serial WiFi Manager");
    OL("=======================");

    while(true){
        for(i=0; i<MAIN_MENU_SIZE; i++){
            O(i+1); O(": "); OL(_main_menu[i]);
        }
        int main = _prompt_int("");
        OL(main);
        delay(1);
    }
    OL(_prompt("Enter choice"));

}
