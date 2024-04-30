#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

const char* WIFI_SSID = "Familia Pillco"; // Nombre de tu red WiFi internet de reserva: Celerity_LFAJARDO_2.4
const char* WIFI_PASSWORD = "4177637pillcog"; // Contraseña de tu red WiFi clave del interntet de reserva: LUFACO8520

const char* API_KEY = "AIzaSyC7AxSjPYiERFRrvuj5NvfOyJ8Gkai721E"; // API Key de Firebase
const char* DATABASE_URL = "https://dianocheapp-default-rtdb.firebaseio.com"; // URL de tu base de datos Firebase
const char* USER_EMAIL = "esp32si@gmail.com"; // Correo electrónico de usuario de Firebase
const char* USER_PASSWORD = "321456"; // Contraseña de usuario de Firebase

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

const int LDR_PIN = 34; // Pin del sensor LDR

void setup_WIFI() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Conectado con IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setupFirebase() {
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  config.signer.tokens.legacy_token = "<database secret>";
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
}

void setup() {
  Serial.begin(115200);
  setup_WIFI();
  setupFirebase();
  pinMode(LDR_PIN, INPUT);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
    if (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0) {
      int lecturaState1 = analogRead(LDR_PIN);
      Serial.printf("Valor LDR: %d\n", lecturaState1);

      if (Firebase.RTDB.setInt(&fbdo, ("/sensor/valor"), lecturaState1)) {
        Serial.println("Datos enviados exitosamente a Firebase.");
      } else {
        Serial.print("Error al enviar datos a Firebase: ");
        Serial.println(fbdo.errorReason());
      }
      
      sendDataPrevMillis = millis();
    }
  } else {
    Serial.println("Firebase no está listo o Wi-Fi desconectado. Reconectando...");
    setup_WIFI();
    setupFirebase();
  }
  delay(500); // Esperar un segundo antes de la próxima lectura
}
