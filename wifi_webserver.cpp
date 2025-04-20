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
<!doctype html>
<html>
  <head>
    <title>ESP32 Car Control</title>
  </head>
  <body>
    <h1>Control Your Car</h1>
    <p>Use your phone's tilt to control the car!</p>
    <p id="payload_viewer"></p>
    <button id="direction_button" type="button">Reverse Direction</button>
    <button id="permission_button" type="button">Enable Motion Control</button>

    <script>
// Function to convert JSON to URL-encoded string
function encodeParams(data) {
  return Object.keys(data)
    .map((key) => encodeURIComponent(key) + "=" + encodeURIComponent(data[key]))
    .join("&");
}

// Send POST request with accelerometer-based direction
let can_send = true;
function sendpayload(payload) {
  can_send = false;
  fetch("/control_motors", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: encodeParams(payload),
  }).then((response) => {
    can_send = true;
  });
}

let direction = 0;
function handleMotionEvent(event) {
  let g = event.gamma; // Left/right tilt
  g = Math.max(-30, Math.min(30, g));
  let w_r = g > 0 ? 1 - g / 30 : 1;
  let w_l = g > 0 ? 1 : 1 - g / -30;

  let b = event.beta; // Forward/backward tilt
  b = Math.max(0, Math.min(45, b));
  let w_speed = b / 45;

  const payload = {
    left_direction: direction,
    right_direction: direction,
    left_speed: Math.round(255 * w_l * w_speed),
    right_speed: Math.round(255 * w_r * w_speed),
  };
  console.log(payload);
  document.getElementById("payload_viewer").innerHTML =
    `<pre>${JSON.stringify(payload, null, 2)}  ${event.beta}  ${event.gamma}</pre>`;
  if (can_send) {
    sendpayload(payload);
  }
}

// Add click event to request motion access on iOS

function requestPermissions() {
  if (typeof DeviceMotionEvent.requestPermission === "function") {
    DeviceMotionEvent.requestPermission()
      .then((permissionState) => {
        if (permissionState === "granted") {
          document.getElementById("permission_button").disabled = true;
          window.addEventListener("devicemotion", handleMotionEvent);
        } else {
          console.warn("Motion access denied");
        }
      })
      .catch(console.error);
  } else {
    document.getElementById("permission_button").disabled = true;
    console.warn("device does not have DeviceMotionEvent API not available. Trying to fall back to a deprecated approach.");
    document.getElementById("permission_button").innerHTML = "DeviceMotionEvent API not available"
    // handle regular non iOS 13+ devices
    window.addEventListener("deviceorientation", handleMotionEvent);
  }
}

document.addEventListener('DOMContentLoaded', () => {
  // android
  requestPermissions();
  // ios
  document
    .getElementById("permission_button")
    .addEventListener("click", requestPermissions);

  document.getElementById("direction_button").addEventListener("click", () => {
    direction = 1 - direction;
  });
});

    </script>
  </body>
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
