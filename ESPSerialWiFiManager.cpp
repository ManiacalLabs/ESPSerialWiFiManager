#include "ESPSerialWiFiManager.h"

ESPSerialWiFiManager::ESPSerialWiFiManager() {}

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size)
{ _eeprom_size = eeprom_size; }

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size, int eeprom_offset)
{ _eeprom_size = eeprom_size; _eeprom_offset = eeprom_offset; }

void ESPSerialWiFiManager::set_init_ap(char const *ssid, char const *pass)
{ _ssid = String(ssid); _pass = String(pass); }

void _flush_serial(){
    while(Serial.available() > 0){
        Serial.read();
    }
}

uint8_t ESPSerialWiFiManager::begin(){
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    // WiFi.begin(_ssid.c_str(), _pass.c_str());
    // if(_wait_for_wifi())
    //     Serial.println("WiFi Connected!");
    // else
    //     Serial.println("Unable to connect to WiFi!");
}

bool ESPSerialWiFiManager::_wait_for_wifi(bool status){
    int c = 0;
    if(status) Serial.println("Connecting to " + _ssid);
    while ( c < 20 ) {
        if (WiFi.status() == WL_CONNECTED) { return true; }
        delay(500);
        if(status) Serial.print(".");
        c++;
    }
    return false;
}

bool ESPSerialWiFiManager::_wait_for_wifi(){
    return _wait_for_wifi(false);
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

int ESPSerialWiFiManager::_print_menu(String * menu_list, int menu_size){
    int i, opt;
    while(true){
        for(i=0; i<menu_size; i++){
            O(i+1); O(": "); OL(menu_list[i]);
        }
        opt = _prompt_int("");
        OL(opt);

        if(opt < 1 || opt >= menu_size)
            OL("Invalid Menu Option!");
        else
            return opt;
    }
}

bool ESPSerialWiFiManager::_connect_wpa(String ssid, String pass){
    _ssid = ssid;
    WiFi.begin(ssid.c_str(), pass.c_str());
    return _wait_for_wifi(true);
}

bool ESPSerialWiFiManager::_connect_wpa(String ssid){
    O("Connect to "); O(ssid); OL(":");

    String pass = _prompt("Password");
    OL(pass);

    return _connect_wpa(ssid, pass);
}

void ESPSerialWiFiManager::_scan_for_networks(){
    int i, opt;
    String opt_s;

    while(true){
        OL("Starting network scan...");
        int n = WiFi.scanNetworks();

        if(n == 0){
            OL("\nNo Avaliable Networks!\nChoose Option Below.");
        }
        else{
            O(n); OL(" networks found:");
            for(i=0; i < n; i++){
                O(i + 1);O(": ");O(WiFi.SSID(i));
                O(" (");O(WiFi.RSSI(i));O(")");
                OL((WiFi.encryptionType(i) == ENC_TYPE_NONE)?"":"*");
            }
        }

        OL("");
        OL("s: Scan Again");
        OL("q: Quit Network Scan");

        opt_s  = _prompt("");
        OL(opt_s);

        if(opt_s[0] == 'q' || opt_s[0] == 'Q'){ return; } //exit scan
        else{
            opt = opt_s.toInt();
            if(opt < 1 || opt >= n + 2)
                OL("Invalid Menu Option!");
            else{
                    opt = opt - 1;
                    switch (WiFi.encryptionType(opt - 1)) {
                        case ENC_TYPE_WEP:
                            Serial.println("WEP");
                            break;
                        case ENC_TYPE_TKIP:
                        case ENC_TYPE_CCMP:
                            _connect_wpa(WiFi.SSID(opt - 1));
                            break;
                        case ENC_TYPE_NONE:
                            Serial.println("None");
                            break;
                        case ENC_TYPE_AUTO:
                            _connect_wpa(WiFi.SSID(opt - 1));
                            break;
                  }
            }
        }
    }
}

// void printEncryptionType(int thisType) {
//   // read the encryption type and print out the name:
//   switch (thisType) {
//     case ENC_TYPE_WEP:
//       Serial.println("WEP");
//       break;
//     case ENC_TYPE_TKIP:
//       Serial.println("WPA");
//       break;
//     case ENC_TYPE_CCMP:
//       Serial.println("WPA2");
//       break;
//     case ENC_TYPE_NONE:
//       Serial.println("None");
//       break;
//     case ENC_TYPE_AUTO:
//       Serial.println("Auto");
//       break;
//   }
// }

void ESPSerialWiFiManager::run_menu(){
    static const uint8_t _main_menu_size = 3;
    static String _main_menu[_main_menu_size] = {
        "Scan",
        "Manual Config",
        "WPS Connect"
    };

    static int i;
    OL("\n");
    OL("ESP Serial WiFi Manager");
    OL("=======================");

    while(true){
        i = _print_menu(_main_menu, _main_menu_size);
        switch(i){
            case 1:
                _scan_for_networks();
                break;
            case 2:
                break;
            case 3:
                break;
        }
        delay(1);
    }
    OL(_prompt("Enter choice"));

}
