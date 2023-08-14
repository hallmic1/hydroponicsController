//
// Created by mhall on 7/29/2023.
//
#include "Pump.h"
#include <Arduino.h>

Pump::Pump(int pin, String url, String name, Network network) : _network(network) {
    _name = name;
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
        _network.sendRequest("pump/start/" + _name);
    }
}

void Pump::setInactive() {
    bool shouldSendNetworkRequest = _shouldSendNetworkRequest(LOW);
    _state = LOW;
    digitalWrite(_pin, _state);
    if(shouldSendNetworkRequest) {
        _network.sendRequest("pump/stop/" + _name);
    }
}

void Pump::setState(int state) {
    if(state == HIGH) {
        setActive();
    } else {
        setInactive();
    }
}

bool Pump::_shouldSendNetworkRequest(int state) {
    return state != _state;
}


int Pump::getState() {
    return _state;
}

void Pump::init() {
    _network.sendRequest("pump/init/" + _name);
}