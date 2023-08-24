//
// Created by mhall on 7/30/2023.
//

#include "Switch.h"
#include <Arduino.h>

Switch::Switch(int pin, String url, String name, int state = LOW) {
    _name = name;
    _pin = pin;
    _state = state;
    _url = url;
    pinMode(_pin, INPUT_PULLUP);
}

bool Switch::_shouldSendNetworkRequest(int state) {
    return state != _state;
}

bool Switch::isActive() {
    if(_shouldSendNetworkRequest(digitalRead(_pin))) {

        int response = _sendNetworkRequest("switch/" + _name + "/" + getState());
        _handleResponse(response);
        _http.end();
    }
    _state = digitalRead(_pin);
    return _state;
}

void Switch::_handleResponse(int httpResponseCode) {
    if(httpResponseCode > 0) {
        String response = _http.getString();
        Serial.println(response);
    } else {
        Serial.print("Error on sending Request: ");
        Serial.println(httpResponseCode);
    }
}

int Switch::_sendNetworkRequest(String path) {
    Serial.println("Sending request to: " + _url + path);
    _http.begin( (_url + path).c_str());
    _http.addHeader("Content-Type", "text/plain");
    return _http.GET();
}

int Switch::getState() {
    return _state;
}

void Switch::init() {
    _state = digitalRead(_pin);
    int response = _sendNetworkRequest("switch/init/" + _name + "/" + getState());
    _handleResponse(response);
    _http.end();
}