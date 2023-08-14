//
// Created by mhall on 8/12/2023.
//

#include "Network.h"

Network::Network(String url) {
    _url = url;
}

int Network::sendRequest(String url) {
    HTTPClient http;
    Serial.println("Sending request to: " + _url + url);
    http.begin(_url + url);
    int httpResponseCode = http.GET();
    _handleResponse(httpResponseCode);
    http.end();
    return httpResponseCode;
}

void Network::_handleResponse(int httpResponseCode) {
    if(httpResponseCode > 0) {
        String response = _http.getString();
        Serial.println(response);
    } else {
        Serial.print("Error on sending Request: ");
        Serial.println(httpResponseCode);
    }
}

Network::Network(Network const &network) {
    _url = network._url;
}
