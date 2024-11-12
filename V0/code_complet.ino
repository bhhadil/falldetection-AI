#include <Wire.h>
#include <MPU6050_tockn.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TensorFlowLite_ESP32.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include "esp_core_dump.h"

// Configuration WiFi et Pushbullet
const char ssid[] PROGMEM = "Didi";
const char password[] PROGMEM = "hadil1245";
const char accessToken[] PROGMEM = "o.92USPwOxho8g5UmPp5oJD12IVHAy1t2d";

// Texte pour les notifications
const char notificationUrl[] PROGMEM = "https://api.pushbullet.com/v2/pushes";
const char notificationTitle[] PROGMEM = "Alerte de Chute";
const char notificationMessage[] PROGMEM = "Chute détectée ! Veuillez vérifier.";

// Capteur MPU6050
MPU6050 mpu(Wire);
#include "fall_detectionmodel.h"

// Configuration du modèle
constexpr int tensor_arena_size = 4096; //la taille de la mémoire du tenseur
uint8_t tensor_arena[tensor_arena_size];
tflite::MicroInterpreter interpreter(tflite::GetModel(fall_detection_model), 
                                     tflite::AllOpsResolver(), 
                                     tensor_arena, 
                                     tensor_arena_size, 
                                     nullptr);
TfLiteTensor* input;
TfLiteTensor* output;

void sendPushbulletNotification(const char* message) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(notificationUrl); // Utilise la chaîne PROGMEM
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Access-Token", accessToken);
        
        String jsonPayload = "{\"type\": \"note\", \"title\": \"";
        jsonPayload += notificationTitle;
        jsonPayload += "\", \"body\": \"";
        jsonPayload += message;
        jsonPayload += "\"}";
        
        if (http.POST(jsonPayload) > 0) {
            Serial.println(F("Notification envoyée !"));
        }
        http.end(); // Libération de mémoire
    }
}
void setup() {
    Serial.begin(115200);
    esp_log_level_set("*", ESP_LOG_NONE); // Désactive la journalisation

    Wire.begin();
    mpu.begin();
    
    // Connexion au WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println(F("Connexion au WiFi..."));
    }
    Serial.println(F("Connecté au WiFi !"));

    if (interpreter.AllocateTensors() != kTfLiteOk) {
        Serial.println(F("Erreur lors de l'allocation des tenseurs"));
        while (true); // Bloquer en cas d'erreur
    } else {
        input = interpreter.input(0);
        output = interpreter.output(0);
    }
}

void loop() {
    mpu.update(); // Met à jour les données

    // Normalisation des données d'accélération et de gyroscope
    input->data.f[0] = mpu.getAccX() / 16384.0f;
    input->data.f[1] = mpu.getAccY() / 16384.0f;
    input->data.f[2] = mpu.getAccZ() / 16384.0f;
    input->data.f[3] = mpu.getGyroX() / 131.0f;
    input->data.f[4] = mpu.getGyroY() / 131.0f;
    input->data.f[5] = mpu.getGyroZ() / 131.0f;

    // Invocation du modèle
    if (interpreter.Invoke() == kTfLiteOk) {
        if (output->data.f[0] > 0.8) {
            sendPushbulletNotification(notificationMessage);
            
        }
   } else {
        Serial.println(F("Erreur lors de l'exécution du modèle"));
    }
    
    delay(200); //  le délai pour réduire le nombre d'échantillons
}
