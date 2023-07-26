#include <WiFi.h>
#include <HTTPClient.h>

class Pump {
private:
    int _state;
    String _url;
    int _pin;
    HTTPClient _http;
    bool _shouldSendNetworkRequest(int state);
    int _sendNetworkRequest(String path);
    void _handleResponse(int httpResponseCode);


public:
    Pump(int pin, String url);
    void setActive();
    void setInactive();
    void init();
};

class Network {
private:
    String _baseurl;
public:
    Network(String baseurl);
    int sendGet(String extension);
}
//
//                                                                                                PLANT_LOOP
//                                                                                              |-->------->|
//                                                                                              |           |
//                                                                                              |           |
//---->--PUMP_WATER->-|                                                           |       |     |           |
//                    |                                                           | Fert  |     ^           |
//                                                                                |       |     |           |
//                | WATER |                       |  MIX  | -<--PER_PUMP_FERT--<- _________     |           |
//                | FLOAT |-->-PER_PUMP_WATER->-- | FLOAT |------>------PUMP_MIX--------->------|           |
//                |SWITCH |                       |SWITCH |--------------<---GRAVITY_FED----<----------------
//                _________                       _________
//


const char* ssid = "pretty fly for a wifi";
const char* password = "dont4getFreya";

String serverAddress = "http://100.120.17.84:3000";

#define PUMP_WATER 2
#define PUMP_MIX 4
#define FLOAT_SWITCH 21
#define PER_PUMP_WATER_DIR 35
#define PER_PUMP_WATER_STEP 34
#define PER_PUMP_FERT_DIR 33
#define PER_PUMP_FERT_STEP 34
Pump pump_water(PUMP_WATER, serverAddress);
Pump pump_mix(PUMP_MIX, serverAddress);
//TODO: Extract network methods into own class, use that now extracted method to call /init on server to get server time. set time locally to that time.
// remember, we'll need to calculate how much flow our PUMP_TWO creates as we'll be using a peristaltic pump, slowly rotating for each second PUMP_ONE is on.
// do we need a second bucket to act as a bason between the wall and the mixing bucket? without knowing the flow from the house is constant, we may need both pumps to be peristaltic pumps.
// if we do, we can calulate how much water is being used by our system. I doubt we'd need much flow, as the plants are slow drinkers :)
void setup() {
    pinMode(FLOAT_SWITCH, INPUT);

    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    pump_water.init();
    pump_mix.init();
}
void loop() {
    if(digitalRead(FLOAT_SWITCH) == HIGH) {
        pump_water.setActive();
    } else {
        pump_water.setInactive();
    }

    delay(200);
}


Pump::Pump(int pin, String url) {
    _pin = pin;
    _state = LOW;
    _url = url;
    pinMode(_pin, OUTPUT);
}

void Pump::setActive() {
    bool shouldSendNetworkRequest = _shouldSendNetworkRequest(HIGH);
    _state = HIGH;
    digitalWrite(_pin, _state);
    if(shouldSendNetworkRequest) {
        int response = _sendNetworkRequest("/pump/start/" + String(_pin));
        _handleResponse(response);
        _http.end();
    }
}

void Pump::setInactive() {
    bool shouldSendNetworkRequest = _shouldSendNetworkRequest(LOW);
    _state = LOW;
    digitalWrite(_pin, _state);
    if(shouldSendNetworkRequest) {
        int response = _sendNetworkRequest("/pump/stop/" + String(_pin));
        _handleResponse(response);
        _http.end();
    }
}

bool Pump::_shouldSendNetworkRequest(int state) {
    return state != _state;
}

int Pump::_sendNetworkRequest(String path) {
    _http.begin( (_url + path).c_str());
    _http.addHeader("Content-Type", "text/plain");
    return _http.GET();
}

void Pump::_handleResponse(int httpResponseCode) {
    if(httpResponseCode > 0) {
        String response = _http.getString();
        Serial.println(response);
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }
}

void Pump::init() {
    int response = _sendNetworkRequest("/pump/init/" + String(_pin));
    _handleResponse(response);
    _http.end();
}
