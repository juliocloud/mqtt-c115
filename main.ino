#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""
#define BLYNK_PRINT Serial

#define fechado 0
#define aberto 1

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Stepper.h>
#include <PubSubClient.h> // <-- NEW: MQTT Library

bool state = fechado;

// 2. WIFI CREDENTIALS
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "";
char pass[] = "";

// --- NEW: MQTT SETTINGS ---
const char* mqtt_server = "192.168.0.109"; // <-- IMPORTANT: Change this!
WiFiClient espClient;
PubSubClient client(espClient);
// --- END OF NEW MQTT SETTINGS ---

// 3. HARDWARE SETTINGS
#define DHTPIN 4
#define DHTTYPE DHT11

const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 19, 5, 18, 21);

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// --- NEW: Function to reconnect to MQTT ---
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("esp32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
// --- END OF NEW FUNCTION ---

void myTimerEvent() {
  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // B. Send to Blynk (Virtual Pin V0)
  Blynk.virtualWrite(V0, t);
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");

  // --- NEW: Publish to MQTT ---
  if (client.connected()) {
    char tempString[8];
    dtostrf(t, 4, 2, tempString); // Convert float to string
    client.publish("esp32/temperature", tempString);
    Serial.print("Published to MQTT topic 'esp32/temperature': ");
    Serial.println(tempString);
  }
  // --- END OF NEW MQTT PUBLISH ---

  // C. Motor Logic
  if (t > 27 && state == fechado) {
    Serial.println("Abrindo a janela de ventilacao (temp > 27)");
    myStepper.step(stepsPerRevolution);
    state = aberto;
  } else if (t <= 27 && state == aberto) {
    Serial.println("Fechando janela de ventilacao (temp <= 27)");
    myStepper.step(-stepsPerRevolution);
    state = fechado;
  }
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  myStepper.setSpeed(10);

  Blynk.begin(auth, ssid, pass);

  // --- NEW: Setup MQTT ---
  client.setServer(mqtt_server, 1883); // Port 1883 is standard for MQTT
  // --- END OF NEW MQTT SETUP ---

  timer.setInterval(5000L, myTimerEvent); // Increased interval to 5 seconds for stability
}

void loop() {
  Blynk.run();
  timer.run();

  // --- NEW: MQTT Loop ---
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
  // --- END OF NEW MQTT LOOP ---
}
