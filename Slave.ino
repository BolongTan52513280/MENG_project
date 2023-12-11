// This example code is in the Public Domain (or CC0 licensed, at your option.)
// By Evandro Copercini - 2018
//
// This example creates a bridge between Serial and Classical Bluetooth (SPP)
// and also demonstrate that SerialBT have the same functionalities of a normal Serial
// Note: Pairing is authenticated automatically by this device

#include "BluetoothSerial.h"
#include "esp_sleep.h"
#define SLEEP_DURATION 5e6 // 5 second in microseconds
#include "math.h"
#include <DHT.h>
#define DHTPIN 26 // The GPIO pin connected toß the DHT22 data pin
#define DHTTYPE DHT22 // Specify the sensor type

String device_name = "ESP32-BT-Slave";
String master_name = "ESP32-BT-Master";
unsigned long timeout = SLEEP_DURATION/1000 * 2 - 2000;
unsigned long start_time;
unsigned long current_time;
int state = 1;
int motorPin = 23; // Define the motor pin 
int buzzerPin = 19; // Define the buzzer pin 
bool temp_alarm = false;

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
  #error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  SerialBT.begin(device_name); //Bluetooth device name
  //SerialBT.deleteAllBondedDevices(); // Uncomment this to delete paired devices; Must be called after begin
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  dht.begin();
  pinMode(motorPin, OUTPUT); // Set the motor pin as an OUTPUT
  pinMode(buzzerPin, OUTPUT); // Set the buzzer pin as an OUTPUT
}

void loop() {
  // Temp
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Read temperature in Celsius
  // float temperatureF = dht.readTemperature(true);  // Read temperature in Fahrenheit

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temperature: ");
  Serial.print(temperature);
  delay(500);  // Wait for 0.5 seconds between measurements

  if (temperature > 24) {
    temp_alarm = true;
    Serial.println("high");
    digitalWrite(motorPin, HIGH);
    tone(buzzerPin, 1000); // Parameters: (pin, frequency in Hz)
    delay(100);
    digitalWrite(motorPin, LOW);
    noTone(buzzerPin); // Turn off the buzzer
    delay(100);
  }
  else {
    temp_alarm = false;
    digitalWrite(motorPin, LOW);
    noTone(buzzerPin); // Turn off the buzzer
  }

  // BLE
  if (!SerialBT.connected()){
    if (state == 1){
      start_time = millis();
      state = 0;
    }

    current_time = millis();
    if ((current_time - start_time) > timeout){
      Serial.println("Disconnected!");
      SerialBT.write(100);
      digitalWrite(motorPin, HIGH);
      tone(buzzerPin, 1000); // Parameters: (pin, frequency in Hz)
      delay(500);
      digitalWrite(motorPin, LOW);
      noTone(buzzerPin); // Turn off the buzzer
      delay(500);
  
    }
  }
  if (SerialBT.connected()) {
    Serial.println("Connected!");
    SerialBT.write(99);
    digitalWrite(motorPin, LOW);
    noTone(buzzerPin); // Turn off the buzzer
    state = 1;
    delay(1000);
    if (!temp_alarm) {
      // Go to deep sleep for 1 second 
      Serial.println("Going to sleep for 5 second..."); 
      esp_sleep_enable_timer_wakeup(SLEEP_DURATION); 
      esp_deep_sleep_start();
    }
  
    
  }

  delay(20);
}