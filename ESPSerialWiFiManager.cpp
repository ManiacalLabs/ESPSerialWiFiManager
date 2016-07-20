#include "ESPSerialWiFiManager.h"

ESPSerialWiFiManager::ESPSerialWiFiManager() {}

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size)
{ _eeprom_size = eeprom_size; }

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size, int eeprom_offset)
{ _eeprom_size = eeprom_size; _eeprom_offset = eeprom_offset; }

void ESPSerialWiFiManager::set_init_ap(char const *ssid, char const *pass)
{ _set_config(String(ssid), String(pass), true); }

void _flush_serial(){
    while(Serial.available() > 0){
        Serial.read();
    }
}

uint8_t ESPSerialWiFiManager::begin(){
    EEPROM.begin(_eeprom_size);
    _read_config();

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    if(_network_config.config){
        _connect_from_config();
    }
}

void ESPSerialWiFiManager::_write_config(){
    EEPROM_writeAnything(_eeprom_offset + 1, _network_config);
}

void ESPSerialWiFiManager::_read_config(){
    if(EEPROM.read(_eeprom_offset) != CONFIGCHECK)
    {
        OL("Bad CONFIGCHECK");
        memset(_network_config.ssid, 0, sizeof(char) * SSID_MAX);
        memset(_network_config.password, 0, sizeof(char) * PASS_MAX);
        _network_config.encrypted = false;
        _write_config();
        EEPROM.write(_eeprom_offset, CONFIGCHECK);
    }

    EEPROM_readAnything(_eeprom_offset + 1, _network_config);
}

void ESPSerialWiFiManager::_set_config(String ssid, String pass, bool enc){
    memset(_network_config.ssid, 0, sizeof(char) * SSID_MAX);
    memset(_network_config.password, 0, sizeof(char) * PASS_MAX);

    memcpy(_network_config.ssid, ssid.c_str(), sizeof(char) * ssid.length());
    memcpy(_network_config.password, pass.c_str(), sizeof(char) * pass.length());
    _network_config.encrypted = enc;

    _network_config.config = true;
}

// WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
// WL_IDLE_STATUS      = 0,
// WL_NO_SSID_AVAIL    = 1,
// WL_SCAN_COMPLETED   = 2,
// WL_CONNECTED        = 3,
// WL_CONNECT_FAILED   = 4,
// WL_CONNECTION_LOST  = 5,
// WL_DISCONNECTED     = 6

bool ESPSerialWiFiManager::_wait_for_wifi(bool status){
    int c = 0;
    if(status) Serial.println("Connecting to " + WiFi.SSID());
    while ((WiFi.status() == WL_IDLE_STATUS || WiFi.status() == WL_DISCONNECTED) && c < WIFI_WAIT_TIMEOUT) {
        delay(500);
        if(status) Serial.print(".");
        c++;
    }

    OL("");
    int ws = WiFi.status();
    if (ws == WL_CONNECTED) {
        OL("Connection Sucessful");
        return true;
    }
    else if (status && ws == WL_CONNECT_FAILED){
        OL("Failed Connecting to AP");
    }
    else{
        OL("Timed Out Connecting to AP");
    }
    OL("");
    return false;
}

bool ESPSerialWiFiManager::_wait_for_wifi(){
    return _wait_for_wifi(false);
}

void ESPSerialWiFiManager::_disp_network_details(){
    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    O("IP Address: ");
    OL(ip);

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    O("MAC address: ");
    Serial.print(mac[5],HEX);
    Serial.print(":");
    Serial.print(mac[4],HEX);
    Serial.print(":");
    Serial.print(mac[3],HEX);
    Serial.print(":");
    Serial.print(mac[2],HEX);
    Serial.print(":");
    Serial.print(mac[1],HEX);
    Serial.print(":");
    Serial.println(mac[0],HEX);

    // print your subnet mask:
    IPAddress subnet = WiFi.subnetMask();
    O("NetMask: ");
    OL(subnet);

    // print your gateway address:
    IPAddress gateway = WiFi.gatewayIP();
    O("Gateway: ");
    OL(gateway);
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
                Serial.write(tmp);
                Serial.flush();
                count++;
            }
            else{
                _flush_serial();
                OL("");
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

        if(opt < 1 || opt >= menu_size)
            OL("Invalid Menu Option!");
        else
            return opt;
    }
}

bool ESPSerialWiFiManager::_connect_from_config(){
    if(_network_config.encrypted){
        WiFi.begin(_network_config.ssid, _network_config.password);
    }
    else{
        WiFi.begin(_network_config.ssid);
    }

    bool res = _wait_for_wifi(true);
    if(res) _disp_network_details();
    return res;
}

bool ESPSerialWiFiManager::_connect_enc(String ssid, String pass){
    WiFi.begin(ssid.c_str(), pass.c_str());
    if(_wait_for_wifi(true)){
        OL("Saving config...");
        _set_config(ssid, pass, true);
        _write_config();
        return true;
    }
    else{
        return false;
    }
}

bool ESPSerialWiFiManager::_connect_enc(String ssid){
    O("Connect to "); O(ssid); OL(":");

    String pass = _prompt("Password");

    return _connect_enc(ssid, pass);
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

        if(opt_s[0] == 'q' || opt_s[0] == 'Q'){ return; } //exit scan
        else if(opt_s[0] == 's' || opt_s[0] == 'S'){ continue; }
        else{
            opt = opt_s.toInt();
            if(opt < 1 || opt >= n + 2)
                OL("Invalid Menu Option!");
            else{
                    opt = opt - 1;
                    switch (WiFi.encryptionType(opt)) {
                        case ENC_TYPE_WEP:
                        case ENC_TYPE_TKIP:
                        case ENC_TYPE_CCMP:
                        case ENC_TYPE_AUTO:
                            if(_connect_enc(WiFi.SSID(opt))){
                                _disp_network_details();
                                return;
                            }
                            else{
                                opt_s = _prompt("Scan Again? y/n");
                                if(opt_s[0] == 'y' || opt_s[0] == 'Y'){ continue; }
                                else{ return; }
                            }
                            break;
                        case ENC_TYPE_NONE:
                            Serial.println("None");
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
    static const uint8_t _main_menu_size = 4;
    static String _main_menu[_main_menu_size] = {
        "Scan",
        "Manual Config",
        "WPS Connect",
        "Quit"
    };

    static int i;
    OL("\n");
    OL("ESP Serial WiFi Manager");
    OL("=======================");

    while(true){
        OL("\nMain Menu");
        i = _print_menu(_main_menu, _main_menu_size);
        switch(i){
            case 1:
                _scan_for_networks();
                break;
            case 2:
                break;
            case 3:
                break;
            case 4: //Quit
                return;
        }
        delay(1);
    }
}
