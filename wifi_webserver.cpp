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
    <style>
      #joystick-container {
        position: relative;
        width: 200px;
        height: 200px;
        background-color: #ddd;
        border-radius: 50%;
        margin: 20px auto;
        touch-action: none;
      }
      #joystick-handle {
        position: absolute;
        width: 50px;
        height: 50px;
        background-color: #888;
        border-radius: 50%;
        transform: translate(75px, 75px); /* Center handle */
      }
    </style>
  </head>
  <body>
    <h1>Control Your Car</h1>
    <p>Slide the joystick to control the car!</p>
    <p id="payload_viewer"></p>
    <button id="direction_button" type="button">Reverse Direction</button>
    <div id="joystick-container">
      <div id="joystick-handle"></div>
    </div>

    <script>
      // Function to convert JSON to URL-encoded string
      function encodeParams(data) {
        return Object.keys(data)
          .map((key) => encodeURIComponent(key) + "=" + encodeURIComponent(data[key]))
          .join("&");
      }

      let can_send = true;
      let direction = 0;

      // Send POST request with joystick-based direction
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

      // Joystick functionality
      const joystickContainer = document.getElementById("joystick-container");
      const joystickHandle = document.getElementById("joystick-handle");
      const containerSize = joystickContainer.offsetWidth;
      const handleSize = joystickHandle.offsetWidth;

      let containerCenter = containerSize / 2;
      let maxMovement = containerCenter - handleSize / 2;

      joystickContainer.addEventListener("pointerdown", startJoystick);
      document.addEventListener("pointermove", moveJoystick); // Allow dragging outside container
      document.addEventListener("pointerup", endJoystick);

      let active = false; // Track if dragging is active

      function startJoystick(event) {
        event.preventDefault();
        active = true; // Start drag
      }

      function moveJoystick(event) {
        if (!active) return; // Do nothing if not dragging

        const rect = joystickContainer.getBoundingClientRect();
        const x = event.clientX - rect.left - containerCenter;
        const y = event.clientY - rect.top - containerCenter;

        const distance = Math.sqrt(x * x + y * y);
        const clampedX = distance > maxMovement ? (x / distance) * maxMovement : x;
        const clampedY = distance > maxMovement ? (y / distance) * maxMovement : y;

        // Update joystick handle position (inside container limits)
        joystickHandle.style.transform = `translate(${containerCenter + clampedX - handleSize / 2}px, ${
          containerCenter + clampedY - handleSize / 2
        }px)`;

        const x_norm01 = Math.min(Math.max(0, (event.clientX - rect.left) / (rect.right - rect.left)), 1)*2-1;
        const y_norm01 = Math.min(Math.max(0, (event.clientY - rect.bottom) / (rect.top - rect.bottom)), 1)*2-1;

        console.log(x_norm01, y_norm01);

        // Calculate motor speeds based on joystick position
        const w_speed = Math.abs(y_norm01);
        let w_l = x_norm01 > 0 ? 1-x_norm01 : 1;
        let w_r = x_norm01 > 0 ? 1 : 1 + x_norm01;

        const payload = {
          left_direction: clampedY > 0 ? 1 : 0,
          right_direction: clampedY > 0 ? 1 : 0,
          left_speed: w_speed*w_l*255,
          right_speed: w_speed*w_r*255,
        };

        document.getElementById("payload_viewer").innerHTML = `<pre>${JSON.stringify(payload, null, 2)}</pre>`;
        if (can_send) {
          sendpayload(payload);
        }
      }

      function endJoystick(event) {
        active = false; // End drag
        joystickHandle.style.transform = `translate(${containerCenter - handleSize / 2}px, ${
          containerCenter - handleSize / 2
        }px)`; // Reset handle position

        const payload = {
          left_direction: direction,
          right_direction: direction,
          left_speed: 0,
          right_speed: 0,
        };

        document.getElementById("payload_viewer").innerHTML = `<pre>${JSON.stringify(payload, null, 2)}</pre>`;
        if (can_send) {
          sendpayload(payload);
        }
      }

      // Reverse direction button functionality
      document.getElementById("direction_button").addEventListener("click", () => {
        direction = 1 - direction;
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
