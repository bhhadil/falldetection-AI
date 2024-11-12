#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <MPU6050.h>

// WiFi credentials
const char* ssid = "Didi";  
const char* password = "hadil12345";  

const char* firebaseUrl = "https://firetest-46f48-default-rtdb.asia-southeast1.firebasedatabase.app/data.json";

MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // Initialize I2C with SDA on GPIO 21 and SCL on GPIO 22
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  // Initialize the gyroscope sensor
  mpu.initialize();  // Explicitly initialize the MPU6050
  if (!mpu.testConnection()) {
    Serial.println("Error connecting to MPU6050!");
    while (1);  // Hang if there's an error
  } else {
    Serial.println("MPU6050 connected successfully!");
  }
}

void loop() {
  // Read sensor data
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Display data for debugging
  Serial.print("Accel X: "); Serial.print(ax);
  Serial.print(" | Accel Y: "); Serial.print(ay);
  Serial.print(" | Accel Z: "); Serial.print(az);
  Serial.print("\nGyro X: "); Serial.print(gx);
  Serial.print(" | Gyro Y: "); Serial.print(gy);
  Serial.print(" | Gyro Z: "); Serial.print(gz);
  Serial.println();

  // Convert data to JSON format
  String jsonPayload = "{";
  jsonPayload += "\"gyroX\":" + String(gx) + ",";
  jsonPayload += "\"gyroY\":" + String(gy) + ",";
  jsonPayload += "\"gyroZ\":" + String(gz);
  jsonPayload += "}";

  // Send data to Firebase
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(firebaseUrl);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.PUT(jsonPayload);  // Use PUT to overwrite data or POST to add

    if (httpResponseCode > 0) {
      Serial.print("Response Code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Request Error: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("No Wi-Fi connection");
  }

  delay(1000);  // Delay before next transmission
}
