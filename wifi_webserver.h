#ifndef WIFI_WEBSERVER_H
#define WIFI_WEBSERVER_H

#include <WebServer.h>
#include "lego_motor.h"

namespace WiFi_WebServer {
void init(LegoDCMotor &left_motor, LegoDCMotor &right_motor);
void handleClient();
}

#endif  // WIFI_WEBSERVER_H
