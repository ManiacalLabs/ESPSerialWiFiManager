#include <ESP8266WiFi.h>
#include <ESPSerialWiFiManager.h>

ESPSerialWiFiManager esp = ESPSerialWiFiManager();

void setup(){
    Serial.begin(115200);

    esp.begin();
}

String host = "ip.jsontest.com";

void loop(){
    if(esp.status() != WL_CONNECTED)
        esp.run_menu(10);
    else{
        //http://ip.jsontest.com/
        WiFiClient client;
        if(!client.connect(host.c_str(), 80)){
            Serial.println("Unable to connect to site!");
            return;
        }

        // We now create a URI for the request
        String url = "/";
        Serial.print("Requesting URL: ");
        Serial.println(host + url);

        // This will send the request to the server
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Connection: close\r\n\r\n");
        delay(500);

        // Read all the lines of the reply from server and print them to Serial
        while(client.available()){
          String line = client.readStringUntil('\r');
          Serial.print(line);
        }

        Serial.println();
        Serial.println("closing connection");
    }

    delay(5000);
}
