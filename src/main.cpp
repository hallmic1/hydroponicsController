#include <Arduino.h>
#include <WiFi.h>
#include "../lib/Pump/Pump.h"
#include "../lib/Switch/Switch.h"
#include "../lib/PerPump/PerPump.h"
#include "../lib/Network/Network.h"
#include "mdns.h"
//
//                                                                                                PLANT_LOOP
//                                                                                              |-->------->|
//                                                                                              |           |
//                                                                                              |           |
//->-WATER_SOLENOID->-|                                                           |  MIX  |     |           |
//                    |                                                           | FERT  |     ^           |
//                                                                                |SWITCH |     |           |
//                | WATER |                       |  MIX  | -<--PER_PUMP_FERT--<- _________     |           |
//                | FLOAT |-->-PER_PUMP_WATER->-- | FLOAT |------>------PUMP_MIX--------->------|           |
//                |SWITCH |                       |SWITCH |--------------<---GRAVITY_FED----<----------------
//                _________                       _________
//
// state as follows: { pumps: { pump_water, pump_mix, per_pump_water, per_pump_fert }, float_switches: { float_switch_water, float_switch_mix, float_switch_fert } }


// NETWORK CONNECTION
const char* ssid = "pretty fly for a wifi";
const char* password = "dont4getFreya";


// SERVER CONNECTION
String serverAddress = "http://192.168.4.62:3001/";


// HTTP SERVER
WiFiServer server(80);
String serverHeader;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;
const long fifteenMinutes = 1000 * 60 * 15;
long mixPumpTimer = 0;
boolean mixPumpOn = true;
int base_speed = 150;
int fert_multiplier = 757;

int tick = 0;
#define WATER_SOLENOID 2
#define PUMP_MIX 4
#define WATER_FLOAT_MAX_SWITCH 33
#define WATER_FLOAT_MIN_SWITCH 25
#define MIX_FLOAT_MAX_SWITCH 35
#define MIX_FLOAT_MIN_SWITCH 32
#define PER_PUMP_WATER_DIR 26
#define PER_PUMP_WATER_STEP 27 // looks like we need 757 ml of water to 1 ml of fertilizer.
#define PER_PUMP_FERT_DIR 12
#define PER_PUMP_FERT_STEP 14
#define SWITCHES_CONNECTED 34

Network network(serverAddress);

Pump water_solenoid(WATER_SOLENOID, serverAddress, "Water_Solenoid", network);
Pump pump_mix(PUMP_MIX, serverAddress, "Mix_Pump", network);
PerPump per_pump_water(PER_PUMP_WATER_DIR, PER_PUMP_WATER_STEP, serverAddress, "Per_Pump_Water");
PerPump per_pump_fert(PER_PUMP_FERT_DIR, PER_PUMP_FERT_STEP, serverAddress, "Per_Pump_Fert");
Switch water_float_max_switch(WATER_FLOAT_MAX_SWITCH, serverAddress, "Water_Float_Max_Switch");
Switch water_float_min_switch(WATER_FLOAT_MIN_SWITCH, serverAddress, "Water_Float_Min_Switch");
Switch mix_float_max_switch(MIX_FLOAT_MAX_SWITCH, serverAddress, "Mix_Float_Max_Switch");
Switch mix_float_min_switch(MIX_FLOAT_MIN_SWITCH, serverAddress, "Mix_Float_Min_Switch");
Switch switches_connected(SWITCHES_CONNECTED, serverAddress, "Switches_Connected", HIGH);
//TODO:
// remember, we'll need to calculate how much flow our PUMP_TWO creates as we'll be using a peristaltic pump, slowly rotating for each second PUMP_ONE is on.
// we can calculate how much water is being used by our system. I doubt we'd need much flow, as the plants are slow drinkers :)
// create a switch class that can be used for the float switches
// create a peristaltic pump class that can be used for the peristaltic pumps ( a stepper motor controller)
// each should have a status, and report that status to the server. Now may be a good time to move network requests into its own class.
void sendStatus(WiFiClient client);
void start_mdns_service();
void initialize_with_network();
bool is_pump_on();
void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);
    initialize_with_network();
    start_mdns_service();
    server.begin();
}

__attribute__((unused)) void loop() {
    if(WiFiClass::status() != WL_CONNECTED) {
        Serial.print("WiFi connection lost, with error code: " + WiFiClass::status());
        while(true) {
            ESP.restart();
        }
    } else {
        if(switches_connected.isActive()) {
            water_solenoid.setState(!water_float_max_switch.isActive() && !water_float_min_switch.isActive());
            water_float_min_switch.isActive();
            pump_mix.setState(mix_float_min_switch.isActive() && is_pump_on());
            per_pump_water.setState(!mix_float_max_switch.isActive());
            per_pump_water.run(base_speed, tick);
            per_pump_fert.setState(per_pump_water.isActive());
            per_pump_fert.run(base_speed * fert_multiplier, tick);
        }
        WiFiClient client = server.available();
        if (client) {
            currentTime = millis();
            previousTime = currentTime;
            String currentLine = "";
            while (client.connected() && currentTime - previousTime <= timeoutTime) {
                currentTime = millis();
                if (client.available()) {
                    int c = client.read();
                    serverHeader += c;
                    if (c == '\n') {
                        if (currentLine.length() == 0) {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-type:application/json");
                            client.println("Access-Control-Allow-Origin: *" );
                            client.println("Connection: close");
                            client.println();
                            if(serverHeader.indexOf(String("/health").toInt()) >= 0) {
                                Serial.println("Health check");
                                client.println(R"({"status": "OK"})");
                            } else {
                                sendStatus(client);
                            }
                            break;
                        } else {
                            currentLine = "";
                        }
                    } else if (c != '\r') {
                        currentLine += c;
                    }
                }
            }
            String request = serverHeader.substring(serverHeader.indexOf("GET"), serverHeader.indexOf("Host"));
            request.replace(" HTTP/1.1", "");
            request.trim();
            Serial.println(request);
            serverHeader = "";
        }
    }
    tick = tick + 1;
}
bool is_pump_on() {
    if(mixPumpTimer > fifteenMinutes) {
        mixPumpOn = !mixPumpOn;
        mixPumpTimer = 0;
    }
    mixPumpTimer = mixPumpTimer + 1;
    return mixPumpOn;
}
void initialize_with_network() {
    Serial.println("Connecting to WiFi network: " + String(ssid) + " with password: " + String(password));
    while(WiFiClass::status() != WL_CONNECTED) {
        if(WiFiClass::status() == WL_CONNECT_FAILED) {
            Serial.println("Failed to connect to WiFi network");
            ESP.restart();
        }
        Serial.print(WiFiClass::status());
        delay(500);
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    int response = network.sendRequest("init");
    if(response == 200) {
        water_solenoid.init();
        pump_mix.init();
        water_float_max_switch.init();
        water_float_min_switch.init();
        mix_float_max_switch.init();
        mix_float_min_switch.init();
        per_pump_water.init();
        per_pump_fert.init();
    } else {
        Serial.print("Error on sending Request: ");
        Serial.println(response);
    }
}

void start_mdns_service()
{
    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return;
    }

    //set hostname
    mdns_hostname_set("espGarden");
    //set default instance
    mdns_instance_name_set("espGarden");
}



void sendStatus(WiFiClient client) {
    client.println("{");
    client.println("\"pumps\": {");
    client.println("\"Water_Solenoid\": " + String(water_solenoid.getState()));
    client.println(", \"Mix_Pump\": " + String(pump_mix.getState()));
    client.println("},");
    client.println("\"peristaltic_pumps\": {");
    client.println("\"Per_Pump_Water\": {");
    client.println("\"direction\": " + String(per_pump_water.getState().direction) + ",");
    client.println("\"on\": " + String(per_pump_water.getState().on));
    client.println("},");
    client.println("\"Per_Pump_Fert\": {");
    client.println("\"direction\": " + String(per_pump_fert.getState().direction) + ",");
    client.println("\"on\": " + String(per_pump_fert.getState().on));
    client.println("}");
    client.println("},");
    client.println("\"float_switches\": {");
    client.println(
            "\"Water_Float_Max_Switch\": " + String(water_float_max_switch.getState()) + ",");
    client.println(
            "\"Water_Float_Min_Switch\": " + String(water_float_min_switch.getState()) + ",");
    client.println(
            "\"Mix_Float_Max_Switch\": " + String(mix_float_max_switch.getState()) + ",");
    client.println(
            "\"Mix_Float_Min_Switch\": " + String(mix_float_min_switch.getState()));
    client.println("}");
    client.println("}");
    client.println();
}

