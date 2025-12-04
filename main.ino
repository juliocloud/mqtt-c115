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
#include <PubSubClient.h>

bool state = fechado;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "";
char pass[] = "";

const char* mqtt_server = "192.168.0.109";
WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN 4
#define DHTTYPE DHT11

const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 19, 5, 18, 21);

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("esp32Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void myTimerEvent() {
  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Falha ao ler o sensor DHT");
    return;
  }

  Blynk.virtualWrite(V0, t);
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" °C");

  if (client.connected()) {
    char tempString[8];
    dtostrf(t, 4, 2, tempString);
    client.publish("esp32/temperature", tempString);
    Serial.print("Publicado no tópico MQTT 'esp32/temperature': ");
    Serial.println(tempString);
  }

  if (t > 27 && state == fechado) {
    Serial.println("Abrindo a janela de ventilacao");
    myStepper.step(stepsPerRevolution);
    state = aberto;
  } else if (t <= 27 && state == aberto) {
    Serial.println("Fechando a janela de ventilacao");
    myStepper.step(-stepsPerRevolution);
    state = fechado;
  }
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  myStepper.setSpeed(10);

  Blynk.begin(auth, ssid, pass);

  client.setServer(mqtt_server, 1883);

  timer.setInterval(5000L, myTimerEvent);
}

void loop() {
  Blynk.run();
  timer.run();

  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
}
