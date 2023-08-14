//
// Created by mhall on 7/29/2023.
//
#include "Arduino.h"
#include "../Network/Network.h"
#ifndef ARDUINO_PLANT_CONTROLLER_PUMP_H
#define ARDUINO_PLANT_CONTROLLER_PUMP_H
enum status {
    UNINITIALIZED,
    INITIALIZED,
    ACTIVE,
    INACTIVE,
};
class Pump {
private:

    int _state;
    String _name;
    String _url;

    int _pin;
    Network _network;
    bool _shouldSendNetworkRequest(int state);
    void setActive();
    void setInactive();

public:
    Pump(int pin, String url, String name, Network network);
    void setState(int state);
    void init();
    int getState();
};
#endif //ARDUINO_PLANT_CONTROLLER_PUMP_H
