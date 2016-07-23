#include "ESPSerialWiFiManager.h"

//helper to ensure serial read line is cleared
void _flush_serial(){
    while(Serial.available() > 0){
        Serial.read();
    }
}

ESPSerialWiFiManager::ESPSerialWiFiManager() {}

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size)
{ _eeprom_size = eeprom_size; }

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size, int eeprom_offset)
{ _eeprom_size = eeprom_size; _eeprom_offset = eeprom_offset; }


uint8_t ESPSerialWiFiManager::begin(){
    EEPROM.begin(_eeprom_size);
    _read_config();

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    if(_network_config.config){
        _connect_from_config();
    }

    return WiFi.status();
}

uint8_t ESPSerialWiFiManager::begin(String ssid, String pass){
    _set_config(ssid, pass, pass.length() > 0);
    return begin();
}

uint8_t ESPSerialWiFiManager::begin(String ssid){
    return begin(ssid, "");
}

uint8_t ESPSerialWiFiManager::status(){ return WiFi.status(); }

void ESPSerialWiFiManager::_write_config(){
    EWA(_eeprom_offset + 1, _network_config);
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

    ERA(_eeprom_offset + 1, _network_config);
}

void ESPSerialWiFiManager::_reset_config(){
    memset(_network_config.ssid, 0, sizeof(char) * SSID_MAX);
    memset(_network_config.password, 0, sizeof(char) * PASS_MAX);
    _network_config.encrypted = false;
    _network_config.config = false;
}

void ESPSerialWiFiManager::_set_config(String ssid, String pass, bool enc){
    _reset_config();

    memcpy(_network_config.ssid, ssid.c_str(), sizeof(char) * ssid.length());
    memcpy(_network_config.password, pass.c_str(), sizeof(char) * pass.length());
    _network_config.encrypted = enc;

    _network_config.config = true;
}

void ESPSerialWiFiManager::_save_config(String ssid, String pass, bool enc){
    _set_config(ssid, pass, true);
    _write_config();
    OL("Choose commit config for changes to persist reboot.\n");
}


bool ESPSerialWiFiManager::_connect_from_config(){
    _disconnect();
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

bool ESPSerialWiFiManager::_connect_noenc(String ssid){
    return _connect(ssid, "");
}

bool ESPSerialWiFiManager::_connect(String ssid, String pass){
    _disconnect();
    if(pass.length() > 0)
        WiFi.begin(ssid.c_str(), pass.c_str());
    else
        WiFi.begin(ssid.c_str());
    if(_wait_for_wifi(true)){
        _save_config(ssid, pass, pass.length() > 0);
        return true;
    }
    else{
        return false;
    }
}

bool ESPSerialWiFiManager::_connect_enc(String ssid){
    O("Connect to "); O(ssid); OL(":");

    String pass = _prompt("Password", '*');

    return _connect(ssid, pass);
}

bool ESPSerialWiFiManager::_connect_manual(){
    OL("Manual WiFi Config:");
    String ssid = _prompt("Enter SSID (Case Sensitive)");
    String enc = _prompt("Encrypted Network? y/n");
    String pass = "";
    if(CHAROPT(enc[0],'y')){
        pass = _prompt("Password", '*');
    }

    _disconnect();
    return _connect(ssid, pass);
}

bool ESPSerialWiFiManager::_connect_wps(){
    _disconnect();
    OL("Push the WPS button on your access point now.");
    _prompt("Press Enter when complete");
    OL("Attempting WPS connection. May take some time...");
    if(WiFi.beginWPSConfig()){
        String ssid = WiFi.SSID();
        if(ssid.length() > 0){
            OL("\nSuccess! Connected to network " + ssid);

            _save_config(ssid, WiFi.psk(), true);
            return true;
        }
        else{
            return false;
        }
    }
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

        if(CHAROPT(opt_s[0], 'q')){ return; } //exit scan
        else if(CHAROPT(opt_s[0], 's')){ continue; }
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
                            break;
                        case ENC_TYPE_NONE:
                            if(_connect_noenc(WiFi.SSID(opt))){
                                _disp_network_details();
                                return;
                            }
                            break;

                    opt_s = _prompt("Scan Again? y/n");
                    if(CHAROPT(opt_s[0], 'y')){ continue; }
                    else{ return; }
                  }
            }
        }
    }
}

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

void ESPSerialWiFiManager::_disconnect(){
    if(WiFi.status() == WL_CONNECTED){
        OL("Disconnecting from " + WiFi.SSID() + "...");
        WiFi.disconnect();
        while(WiFi.status() == WL_CONNECTED){
            delay(100);
        }
    }
}

void ESPSerialWiFiManager::_disp_network_details(){
    OL("Current Network Details:");
    OL("SSID: " + WiFi.SSID());

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

String ESPSerialWiFiManager::_prompt(String prompt, char mask, int timeout){
    static char cmd[PROMPT_INPUT_SIZE];
    static int count;
    static char tmp;
    memset(cmd, 0, PROMPT_INPUT_SIZE);
    count = 0;
    if(timeout > 0){
        O("\nTimeout in "); O(timeout); OL("s...");
    }

    int start = millis();

    O(prompt.c_str());
    O("> ");

    while(true){
        if(Serial.available() > 0){
            tmp = Serial.read();
            if(tmp != '\n' && tmp != '\r'){
                cmd[count] = tmp;
                if(mask==' ')
                    Serial.write(tmp);
                else
                    Serial.write(mask);
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
        if(timeout > 0 && (millis()-start) > (timeout * 1000)){
            return "-1";
        }
    }
}

String ESPSerialWiFiManager::_prompt(String prompt, char mask) { return _prompt(prompt, mask, 0); }
String ESPSerialWiFiManager::_prompt(String prompt) { return _prompt(prompt, ' ', 0); }

int ESPSerialWiFiManager::_prompt_int(String prompt){
    return _prompt_int(prompt, 0);
}
int ESPSerialWiFiManager::_prompt_int(String prompt, int timeout){
    String res = _prompt(prompt, ' ', timeout);
    int res_i = res.toInt();
    // if(res_i == 0 && res[0] != 0)
    //     res_i = NULL;
    //TODO: Menu System doesn't need 0, I think?
    return res_i;
}

int ESPSerialWiFiManager::_print_menu(String * menu_list, int menu_size){
    return _print_menu(menu_list, menu_size, 0);
}
int ESPSerialWiFiManager::_print_menu(String * menu_list, int menu_size, int timeout){
    int i, opt;
    while(true){
        for(i=0; i<menu_size; i++){
            O(i+1); O(": "); OL(menu_list[i]);
        }
        opt = _prompt_int("", timeout);

        if(timeout == 0 && (opt < 1 || opt > menu_size))
            OL("Invalid Menu Option!");
        else
            return opt;
    }
}

void ESPSerialWiFiManager::run_menu(){ return run_menu(0); }
void ESPSerialWiFiManager::run_menu(int timeout){
    bool first_run = true;
    static const uint8_t _main_menu_size = 6;
    static String _main_menu[_main_menu_size] = {
        F("Scan"),
        F("Manual Connect"),
        F("WPS Connect"),
        F("Disconnect"),
        F("Commit Config"),
        F("Quit")
        //"Disconnect"
    };

    static int i;
    OL("\n");
    OL("ESP Serial WiFi Manager");
    OL("=======================");

    while(true){
        OL("\nMain Menu");
        i = _print_menu(_main_menu, _main_menu_size, first_run ? timeout : 0);
        if(i == -1) return; //timeout ocurred, exit
        first_run = false;
        switch(i){
            case 1:
                _scan_for_networks();
                break;
            case 2:
                if(_connect_manual())
                    _disp_network_details();
                break;
            case 3: //WPS Connect
                _connect_wps();
                break;
            case 4:
                if(WiFi.status() == WL_CONNECTED){
                    _disconnect();
                    OL("Complete");
                }
                else{
                    OL("Not currently connected...");
                }
                _reset_config();
                _write_config();
                break;
            case 5:
                OL("Commiting config to EEPROM... ");
                EEPROM.commit();
                OL("Complete. Changes will now persist through reboot.");
                break;
            case 6: //quit, nothing to do, just exit
                return;
        }
        delay(1);
    }
}
