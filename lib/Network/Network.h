//
// Created by mhall on 8/12/2023.
//
#include "Arduino.h"
#include <HTTPClient.h>

#ifndef UNTITLED4_NETWORK_H
#define UNTITLED4_NETWORK_H


class Network {
    private:
    String _url;
        HTTPClient _http;
        void _handleResponse(int httpResponseCode);
    public:
        Network(String url);
        int sendRequest(String url);

    Network(Network const &network);
};


#endif //UNTITLED4_NETWORK_H
