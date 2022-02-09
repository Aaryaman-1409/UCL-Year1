
#include <WiFi.h> // WiFi control for ESP32
#include <ThingsBoard.h> // ThingsBoard SDK
#include <ArduinoHttpClient.h>
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks
 
#define EAP_IDENTITY "ucl email"
#define EAP_PASSWORD "ucl password"
 
 
// See https://thingsboard.io/docs/getting-started-guides/helloworld/
#define TOKEN "f4OgbkAbACiC52othUuA"
#define THINGSBOARD_SERVER "demo.thingsboard.io"
WiFiClient espClient; // Initialize ThingsBoard client
ThingsBoard tb(espClient); // Initialize ThingsBoard instance
int status = WL_IDLE_STATUS; // the Wifi radio's status
static uint16_t messageCounter = 0; // count values sent
 
const char* ssid = "eduroam";
 
PubSubClient client(espClient);
 
void connectWifi() {
Serial.println();
Serial.print(F("Connecting to "));
Serial.println(ssid);
 
WiFi.disconnect(true); //disconnect from wifi to set new wifi connection
WiFi.mode(WIFI_STA); //init wifi mode
esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
esp_wifi_sta_wpa2_ent_enable();
 
WiFi.begin(ssid); //connect to wifi
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(F("."));
}
Serial.println("");
Serial.println(F("WiFi is connected!"));
Serial.println(F("IP address set: "));
Serial.println(WiFi.localIP()); //print LAN IP
}
void setup() {
Serial.begin(115200);
connectWifi();
}
RPC_Response processTemperatureChange(const RPC_Data &data)
{
  Serial.println("Received the set temperature RPC method");
 
  // Process data
 
  float example_temperature = data["temperature"];
 
  Serial.print("Example temperature: ");
  Serial.println(example_temperature);
 
  // Just an response example
  return RPC_Response("example_response", 42);
}
void loop() {
// Reconnect to WiFi, if needed
if (WiFi.status() != WL_CONNECTED) {
connectWifi();
return;
}
 
// Reconnect to ThingsBoard, if needed
if (!tb.connected()) {
  // Connect to the ThingsBoard
  Serial.print("Connecting to: ");
  Serial.print(THINGSBOARD_SERVER);
  Serial.print(" with token ");
  Serial.println(TOKEN);
  if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
    Serial.println("Failed to connect");
    return;
  }
} 
  
int pH=0;
int rpm=0;
int temp=0;
 
 
// Uploads new telemetry to ThingsBoard using MQTT.
// See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
// for more details
tb.sendTelemetryInt("pH", pH);
tb.sendTelemetryInt("RPM", rpm);
tb.sendTelemetryInt("temperature", temp );
 
 
 
// Process messages
tb.loop();
 
 
}
