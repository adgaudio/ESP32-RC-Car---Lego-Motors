#include "wifi_webserver.h"
#include "lego_motor.h"
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>


#ifndef SSID_NAME
#define SSID_NAME "Esp32Car"
#endif

namespace WiFi_WebServer {
WebServer server(80);

// HTML content
const char *htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>ESP32 Car Control</title>
  </head>
  <body>
    <h1>Control Your Car</h1>
    <p>Use your phone's tilt to control the car!</p>
    <p id="payload_viewer"></p>
    <button id="direction_button" type="button">Reverse Direction</button>
  </body>

    <script>


// Function to convert JSON to URL-encoded string
function encodeParams(data) {
  return Object.keys(data)
    .map(key => encodeURIComponent(key) + "=" + encodeURIComponent(data[key]))
    .join("&");
}

// Send POST request with accelerometer-based direction
let can_send = true;
function sendpayload(payload) {
  can_send = false;
  fetch('/control_motors', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: encodeParams(payload)
  }).then(response => {
  can_send = true;
// document.getElementById("paHyload_viewer").innerHTML = response.text();
    });
  console.log(payload);
  document.getElementById("payload_viewer").innerHTML = `<pre> ${JSON.stringify(payload,null,2)} </pre>`;
}

if (typeof DeviceMotionEvent.requestPermission === 'function') {
    DeviceMotionEvent.requestPermission()
        .then(permissionState => {
            if (permissionState === 'granted') {
                window.addEventListener('devicemotion', handleMotionEvent);
            } else {
                console.warn("Motion access denied");
            }
        })
        .catch(console.error);
} else {
    // handle regular non iOS 13+ devices
    window.addEventListener('deviceorientation', handleMotionEvent);
}

let direction = 0;
function handleMotionEvent(event) {
      // direction:  get relative speed of left and right motor when turning L or R
      let g = event.gamma; // Left/right tilt
      if (g < -30) { g = -30; }
      else if (g > 30) { g = 30; }
      let w_r, w_l;  // direction weights, in [0,1]
      if (g > 0) {
        w_r = 1 - g/30;
        w_l = 1;
      } else {
        w_l = 1 - g/-30;
        w_r = 1;
      }
      // speed: get overall throttle value, using [-45,0] degrees as stop,max_speed limits
      let b = event.beta; // Forward/backward tilt
      if (b < 0) { b = 0; }
      else if (b > 45) { b = 45; }
      w_speed = b / 45;

  //     sendpayload({
  //     gamma: event.gamma, beta: event.beta,
  //   w_speed, w_l, w_r, direction
  // });
  if (can_send) {
      sendpayload({
        left_direction: direction,
        right_direction: direction,
        left_speed: parseInt(255 * w_l * w_speed),
        right_speed: parseInt(255 * w_r * w_speed),
      });
  }
  }

document.getElementById("direction_button").addEventListener("click", () => {
  direction = 1-direction;
});
    </script>
</html>
)rawliteral";

void controlMotorsRequest(LegoDCMotor &left_motor, LegoDCMotor &right_motor) {
  // if (server.hasArg("left_direction") && server.hasArg("left_speed")) {
    bool left_direction = server.arg("left_direction").toInt();
    uint8_t left_speed = server.arg("left_speed").toInt();
    left_motor.move(left_speed, left_direction);
  // }
  // if (server.hasArg("right_direction") && server.hasArg("right_speed")) {
    bool right_direction = server.arg("right_direction").toInt();
    uint8_t right_speed = server.arg("right_speed").toInt();
    right_motor.move(right_speed, right_direction);
  // }
  server.send(200, "text/plain", "OK - " + (String) right_speed + " " + (String)right_direction + " " + (String)left_speed + " " +(String) left_direction);
}

void init(LegoDCMotor &left_motor, LegoDCMotor &right_motor) {
  // Establish the ESP32 as a WiFi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID_NAME, "12341234");

  // Define HTTP routes
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", htmlPage); });
  server.on("/control_motors", HTTP_POST, [&left_motor, &right_motor]() {
    controlMotorsRequest(left_motor, right_motor);
  });

  // Start the server
  server.begin();
}

void handleClient() {
  server.handleClient();
}
} // namespace WiFi_WebServer
