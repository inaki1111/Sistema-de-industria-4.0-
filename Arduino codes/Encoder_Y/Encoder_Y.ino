#include <WiFi.h>
#include <PubSubClient.h>
#include <Encoder.h>

// Replace with your network credentials
const char* ssid = "Tec-IoT";
const char* password = "spotless.magnetic.bridge";

// MQTT broker information
const char* mqtt_server = "10.25.75.245";
const int mqtt_port = 1883;


WiFiClient espClient;
PubSubClient client(espClient);

const char* start_topic = "start_process";
const char* encoder_topic = "Encoder_Y";

Encoder myEncoder(2, 3); // Replace with your encoder pin numbers

bool send_data = false;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set up MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Initialize encoder
  myEncoder.write(0); // Initialize encoder position
}

void callback(char* topic, byte* payload, unsigned int length) {
  String received_message = "";
  for (int i = 0; i < length; i++) {
    received_message += (char)payload[i];
  }

  Serial.print("Received message on topic '");
  Serial.print(topic);
  Serial.print("': ");
  Serial.println(received_message);

  if (strcmp(topic, start_topic) == 0) {
    if (received_message == "send_data") {
      send_data = true;
    } else {
      send_data = false;
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(start_topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (send_data) {
    // Read the encoder position
    long encoderValue = myEncoder.read();

    // Publish the encoder value to the MQTT topic
    char message[10];
    snprintf(message, sizeof(message), "%ld", encoderValue);
    client.publish(encoder_topic, message);

    Serial.print("Encoder Value: ");
    Serial.println(encoderValue);
  }

  delay(100); // Adjust the delay as needed to control the publishing rate
}
