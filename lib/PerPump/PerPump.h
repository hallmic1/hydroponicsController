//
// Created by mhall on 8/2/2023.
//
#include "Arduino.h"
#include <HTTPClient.h>

#ifndef UNTITLED4_PERPUMP_H
#define UNTITLED4_PERPUMP_H
enum Direction {
    CLOCKWISE = true,
    COUNTER_CLOCKWISE = false,
};

struct Status {
    Direction direction;
    bool on;
};

class PerPump {
private:
    Status _status;
    String _name;
    String _url;
    int _pin;
    HTTPClient _http;
    bool _shouldSendNetworkRequest(bool isOn);
    int _sendNetworkRequest(String path);
    void _handleResponse(int httpResponseCode);
    void setActive();
    void setInactive();
public:
    PerPump(int direction_pin, int step_pin, String url, String name);
    void setState(int state);
    void setDirection(Direction direction);
    void init();
    void run(int speed, int tick);
    bool isActive();
    Status getState();
};
#endif //UNTITLED4_PERPUMP_H
