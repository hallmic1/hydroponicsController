//
// Created by mhall on 7/30/2023.
//
#include "Arduino.h"
#include <HTTPClient.h>

#ifndef UNTITLED4_SWITCH_H
#define UNTITLED4_SWITCH_H
class Switch {
    private:
        int _state;
        String _name;
        String _url;
        int _pin;
        HTTPClient _http;
        bool _shouldSendNetworkRequest(int state);
        int _sendNetworkRequest(String path);
        void _handleResponse(int httpResponseCode);
    public:
        Switch(int pin, String url, String name);
        bool isActive();
        void init();
        int getState();
};
#endif //UNTITLED4_SWITCH_H
