#include "wifi_webserver.h"
#include "lego_motor.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> // For HTTPS


#ifndef SSID_NAME
#define SSID_NAME "Esp32Car"
#endif


namespace WiFi_WebServer {
WiFiServerSecure server(443); // HTTPS uses port 443

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


// openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/C=US/ST=MD/L=Baltimore/O=GreenMount School Engineering Club/OU=Unit/CN=GMSEng"
//
const char *certPem = R"rawliteral(
-----BEGIN CERTIFICATE-----
MIID1zCCAr+gAwIBAgIUenmmpDyLN06UOMR+YKd7uderkscwDQYJKoZIhvcNAQEL
BQAwezELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAk1EMRIwEAYDVQQHDAlCYWx0aW1v
cmUxKzApBgNVBAoMIkdyZWVuTW91bnQgU2Nob29sIEVuZ2luZWVyaW5nIENsdWIx
DTALBgNVBAsMBFVuaXQxDzANBgNVBAMMBkdNU0VuZzAeFw0yNTA0MTcxODAwNDVa
Fw0yNjA0MTcxODAwNDVaMHsxCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJNRDESMBAG
A1UEBwwJQmFsdGltb3JlMSswKQYDVQQKDCJHcmVlbk1vdW50IFNjaG9vbCBFbmdp
bmVlcmluZyBDbHViMQ0wCwYDVQQLDARVbml0MQ8wDQYDVQQDDAZHTVNFbmcwggEi
MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC7lM6IWrdZci9VuDlWuywvJGCY
0tMANYbVkk6CnydqWaCivUUx0B/mZ9ESuNhNCkxn1SHeGsOb/Iw9PsTplKOmUtfL
+m8Q22MXNXRTR9JeI6vTLiM7fjavtxUggq01azaHQ+IeM3OvaZbB1WV+UvHFSR4z
//iBKpEH5fyWjYhuf1sLs+RjmMkDa/n/Yrqj5dvgxD2No7s8pxWrcBStXjBmiwG+
NYP5uM+Frv/3Er634EaRakid8r46kQOOnzn0tjymXwoXF6rSZDO/L+aCT3mI/TOK
meenPwQ0a5OXgKWBwWb/6GLixR1ObZ3OeSgzI0kGCBNiYFU8o0Wwf6a98vsBAgMB
AAGjUzBRMB0GA1UdDgQWBBQN4y3ZfP53wVeq5dSk1xia1/0/BjAfBgNVHSMEGDAW
gBQN4y3ZfP53wVeq5dSk1xia1/0/BjAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3
DQEBCwUAA4IBAQBZtcBjOBmzU/7GZHBuHuqrympHvOy3z+5rE+hCPaGkSeYAH95m
CpGdxAuXVZzhg+4p6PFfwrOp3l6XGebtikPnJzvyCvmnzGWU7bp2254nUyPjfgP5
euApmEnydv0sa28CaaKWQEbInFwrCesbvhwgqKmjHaPD/a2zBj7ermeUEayjTkCZ
twCubbtNrlmsDfVl2RDUO2dJggLJIQDUwU0VTmWP0lKEJsDdY6jRzoMsCUeNkpf7
WtHldHXGaEn18nAxuR+e2xciLDdvntWlitkT8TzFQSMj0+fn9Wenm14eTlF+akHD
3SyNhf3jvLCPirKaEzLmwXuQXgoXbai0uESG
-----END CERTIFICATE-----
)rawliteral";
const char *keyPem = R"rawliteral(
-----BEGIN PRIVATE KEY-----
MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQC7lM6IWrdZci9V
uDlWuywvJGCY0tMANYbVkk6CnydqWaCivUUx0B/mZ9ESuNhNCkxn1SHeGsOb/Iw9
PsTplKOmUtfL+m8Q22MXNXRTR9JeI6vTLiM7fjavtxUggq01azaHQ+IeM3OvaZbB
1WV+UvHFSR4z//iBKpEH5fyWjYhuf1sLs+RjmMkDa/n/Yrqj5dvgxD2No7s8pxWr
cBStXjBmiwG+NYP5uM+Frv/3Er634EaRakid8r46kQOOnzn0tjymXwoXF6rSZDO/
L+aCT3mI/TOKmeenPwQ0a5OXgKWBwWb/6GLixR1ObZ3OeSgzI0kGCBNiYFU8o0Ww
f6a98vsBAgMBAAECggEAUOCzQCVlAoLyF9KLnnKrdVQI9juIUIHDthgUE8vNcdRd
J23WBMlIx84hXiIm2OjE0swZgyslBf+Y89s5icDV4qO7ea5r7ue4zMv2cOX5tS9K
KY6uEDu2FoMo79CIIA0vAJlrSDxE9+/d2YtJ7HKU3cxN4nvOSNryl2Y5RyyVE5bF
t9SFvVyVQKK15Y2tkYUMz924b00VzF6UounGY5JAD2NWOqBwtyeubXM55baqg5N+
9YmG8xwBAlDRv5FiBP80WmzGwq5k5zwxKCB38sS3fgVXbVG0K1pz6tKLfcCs6xB3
/j66pJaxZi+hkNdpW9nxBgZAPLhqiND1udjGzwblVQKBgQD0GglMzgc2XLeYpz+L
ZZUmBHYLz80OcLMsF3j2SdKqpCoSTTRwCXpJgJabDNjIKrtut1mzUdhfgIh+Vush
lYGo8RCbP6oio/Wjm8bAdYCcZEDs4KC9BzvXBfF8hfNVZFuaqao6k8qj1103a6Uj
ZuSONyeGpVJGqyy8Z6YcVfe8UwKBgQDEuX55q5ALllDhvGoAzTVWiP8lSxe9qWea
0UllbD7fJ5cNaUd+7+xto5kQ5VQ+RAgzWsDhbeMWLsmHFuWqIUfA0eEnNhwB6TAO
fbijRWXi/5nq5rxMi0c2X+lZduRvHEVPQI5vvcssg1lHR6GU4YYH3HbWurYqmxtt
T1me60ug2wKBgAtpjDXDDCNGgm7ootfpj+ePHdW/iV67diUBk+4v1WGU+0KPyXvT
dZwqHuBw4VG6bbjnaZIwqWUNpVQCzEttqfo0Cwq3F0U3VSypA8nLtI+bQE3S0rED
vZB3/qpLuOytHHtGo2bJshem4fzNU5MsJFNh0L9Cy23yYs0MK3/3pPVxAoGANPEK
lWnTURr511YiXObc1NX3fCzSTctaQ3LRQsc3wExiPUy43fNpeDQPzFk7K6qZXmCt
Cb3N4DllKMLDud5M9hpFco0ASo9bzHqPBvl1KvrIjEveHudYmcyD+vyhCznbeTGR
Y4b7N5Z9n04qsOtka5csMCt9PMgTQH6bSsZdywMCgYBY5raXGYDqMq2Lw3vYL8pm
Pl9Sy0Ql0QUgFxWSL+oxkAe5WA9WyuisJMklSeIc8C7DAZz1XgcE6LOWDYvIKbPl
mSBTmmxeBJ5p+hqMQXcH1z4p7JtfZGCUMbSi/Z/LW8kTRi8Kr+XdVLj4aqoHji6a
6qatxfWMqz8Fb1n2Zs5gHw==
-----END PRIVATE KEY-----
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

  // Load SSL/TLS certificate
  server.loadCertificate(certPem); // Load cert.pem
  server.loadPrivateKey(keyPem);   // Load key.pem

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
