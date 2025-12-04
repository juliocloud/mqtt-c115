#define BLYNK_TEMPLATE_ID "BLYNK TEMPLATE ID AQUI"
#define BLYNK_TEMPLATE_NAME "C115 PROJETO"
#define BLYNK_AUTH_TOKEN "AUTH_TOKEN_ENV_AQUI"
#define BLYNK_PRINT Serial
// --- START OF FILE ---

#define fechado 0
#define aberto 1

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Stepper.h>

bool state = fechado;

// 2. WIFI CREDENTIALS
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "WLL-Inatel";
char pass[] = "inatelsemfio";

// 3. HARDWARE SETTINGS
#define DHTPIN 4          // DHT Sensor on GPIO 4
#define DHTTYPE DHT11     // Using DHT11 sensor

// Stepper Motor Settings (28BYJ-48)
const int stepsPerRevolution = 2048;  // Total steps for 360 degrees

// *** UPDATED WIRING HERE ***
// IN1 -> D19
// IN3 -> D5
// IN2 -> D18
// IN4 -> D21
// Note: The Stepper library requires pins in order 1-3-2-4 for this specific motor
Stepper myStepper(stepsPerRevolution, 19, 5, 18, 21); 

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// This function runs every 5 seconds
void myTimerEvent()
{
  // A. Read Temperature
  float t = dht.readTemperature();

  // Check if read failed
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // B. Send to Blynk (Virtual Pin V0)
  Blynk.virtualWrite(V0, t);
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");

  // C. Motor Logic
  if (t > 25 && state == fechado) {
    Serial.println("Abrindo a janela de ventilacao (temp > 25)");
    // Step positive for Clockwise
    myStepper.step(stepsPerRevolution); 
    state = aberto;
  } 
  else if(t <= 25 && state == aberto) {
    Serial.println("Fechando janela de ventilacao (temp <= 25)");
    // Step negative for Counter-Clockwise
    myStepper.step(-stepsPerRevolution);
    state = fechado;
  }
}

void setup()
{
  Serial.begin(115200);

  // Initialize Hardware
  dht.begin();
  myStepper.setSpeed(10); // Set motor speed (RPM)

  // Connect to Cloud
  Blynk.begin(auth, ssid, pass);

  // Setup Timer for 5 seconds (5000ms)
  timer.setInterval(2000L, myTimerEvent);
}

void loop()
{
  Blynk.run();
  timer.run();
}
// --- END OF FILE ---
