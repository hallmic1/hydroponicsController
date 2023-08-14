//
// Created by mhall on 8/2/2023.
//
#include "PerPump.h"
#include "Arduino.h"

PerPump::PerPump(int direction_pin, int step_pin, String url, String name) {
    _pin = step_pin;
    _url = url;
    _name = name;
    _status = {CLOCKWISE, false};
    pinMode(direction_pin, OUTPUT);
    pinMode(step_pin, OUTPUT);
}

void PerPump::init() {
    int response = _sendNetworkRequest("peristaltic_pump/init/" + _name + "/" + (_status.direction ? "clockwise" : "counter-clockwise"));
    _handleResponse(response);
    _http.end();
    setDirection(CLOCKWISE);
    digitalWrite(_pin, LOW);
    digitalWrite(_pin, HIGH);
    digitalWrite(_pin, LOW);
}

void PerPump::setActive() {
    bool shouldSendNetworkRequest = _shouldSendNetworkRequest(true);
    _status.on = true;
    digitalWrite(_pin, HIGH);
    if(shouldSendNetworkRequest) {
        int response = _sendNetworkRequest("peristaltic_pump/start/" + _name + "/" + (_status.direction ? "clockwise" : "counter-clockwise"));
        _handleResponse(response);
        _http.end();
    }
}

void PerPump::run(int speed, int tick) {
    if(_status.on && tick % speed == 0) {
        digitalWrite(_pin, HIGH);
        delayMicroseconds(200);
        digitalWrite(_pin, LOW);
        delayMicroseconds(200);
    }
}

void PerPump::setInactive() {
    bool shouldSendNetworkRequest = _shouldSendNetworkRequest(false);
    _status.on = false;
    digitalWrite(_pin, LOW);
    if(shouldSendNetworkRequest) {
        int response = _sendNetworkRequest("peristaltic_pump/stop/" + _name + "/" + (_status.direction ? "clockwise" : "counter-clockwise"));
        _handleResponse(response);
        _http.end();
    }
}

void PerPump::setState(int state) {
    if(state == HIGH) {
        setActive();
    } else {
        setInactive();
    }
}

bool PerPump::_shouldSendNetworkRequest(bool isOn) {
    return isOn != _status.on;
}

int PerPump::_sendNetworkRequest(String path) {
    Serial.println("Sending request to: " + _url + path);
    _http.begin( (_url + path).c_str());
    _http.addHeader("Content-Type", "text/plain");
    return _http.GET();
}

void PerPump::_handleResponse(int httpResponseCode) {
    if(httpResponseCode > 0) {
        String response = _http.getString();
        Serial.println(response);
    } else {
        Serial.print("Error on sending Request: ");
        Serial.println(httpResponseCode);
    }
}

void PerPump::setDirection(Direction direction) {
    if(direction == CLOCKWISE) {
        _status.direction = CLOCKWISE;
        digitalWrite(_pin, HIGH);
    } else {
        _status.direction = COUNTER_CLOCKWISE;
        digitalWrite(_pin, LOW);
    }
}

bool PerPump::isActive() {
    return _status.on;
}

Status PerPump::getState() {
    return _status;
}

