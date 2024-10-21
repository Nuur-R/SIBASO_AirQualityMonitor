#define BLYNK_TEMPLATE_ID "TMPL6sqUA8AkR"  // Pastikan ID ini benar
#define BLYNK_TEMPLATE_NAME "AirQuality"   // Pastikan nama ini sesuai dengan template di Blynk
#define BLYNK_AUTH_TOKEN "xK5yKSukZ-_6lSqne9ks2i6bWXF0Ef6k"  // Auth Token Anda

#include <WiFiManager.h>        // Library untuk konfigurasi WiFi
#include <PubSubClient.h>       // Library MQTT
#include <DHT.h>                // Library DHT
#include <BlynkSimpleEsp32.h>   // Library Blynk
#include <Adafruit_GFX.h>       // Library GFX untuk OLED
#include <Adafruit_SSD1306.h>   // Library OLED

// Konfigurasi DHT
#define DHTPIN 4                // Pin untuk sensor DHT
#define DHTTYPE DHT11           // Tipe sensor yang digunakan
DHT dht(DHTPIN, DHTTYPE); 

WiFiManager wifiManager;

// Variabel MQTT
char mqttServer[40] = "broker.emqx.io";
char mqttPort[6] = "1883";
char mqttUser[40] = "";
char mqttPassword[40] = "";
char mqttTopicPub[40] = "StudyClub/NuurR/Publis";
char mqttTopicSub[40] = "StudyClub/NuurR/Subscribe";

// Konfigurasi OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1 // Tidak menggunakan reset fisik
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Variabel untuk Blynk dan UUID
String deviceID = "AirQuality-123"; // UUID unik untuk device
float temperature, humidity;

// Fungsi Setup
void setup() {
  Serial.begin(115200);
  
  // Inisialisasi DHT
  dht.begin();

  // Konfigurasi WiFiManager
  wifiManager.autoConnect("Mqtt-Nuur");

  // Konfigurasi Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  // Konfigurasi MQTT
  mqttClient.setServer(mqttServer, atoi(mqttPort));
  mqttClient.setCallback(callback);

  // Inisialisasi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Alamat I2C default untuk OLED 128x64
    Serial.println(F("Gagal menginisialisasi OLED!"));
    for (;;);  // Berhenti jika tidak ada OLED
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("Air Quality Monitor"));
  display.display();
  delay(2000); // Tampilkan pesan awal selama 2 detik
  
  Serial.println("Setup selesai, siap mengirim data ke Blynk, MQTT, dan OLED.");
}

// Fungsi utama loop
void loop() {
  // Jalankan Blynk dan MQTT
  Blynk.run();
  mqttClient.loop();

  // Jika tidak terhubung ke broker MQTT, coba terhubung
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  // Baca data dari sensor DHT
  readDHT();

  // Kirim data ke Blynk
  Blynk.virtualWrite(V0, temperature); // Mengirim temperature ke Virtual Pin V0
  Blynk.virtualWrite(V1, humidity);    // Mengirim humidity ke Virtual Pin V1

  // Kirim data ke MQTT dalam format JSON
  publishDataToMQTT();

  // Tampilkan data di OLED
  displayDataOnOLED();

  delay(5000); // Tunggu 5 detik sebelum mengirim data lagi
}

// Fungsi untuk membaca data DHT
void readDHT() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Validasi hasil pembacaan
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Gagal membaca dari sensor DHT");
  } else {
    Serial.print("Suhu: ");
    Serial.print(temperature);
    Serial.print(" °C, Kelembaban: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}

// Fungsi untuk mengirim data ke MQTT
void publishDataToMQTT() {
  String payload = "{\"deviceID\":\"" + deviceID + "\",";
  payload += "\"data\":{\"temperature\":" + String(temperature) + ",";
  payload += "\"humidity\":" + String(humidity) + "}}";

  mqttClient.publish(mqttTopicPub, payload.c_str());
  Serial.println("Data dikirim ke MQTT: " + payload);
}

// Fungsi untuk menampilkan data di OLED
void displayDataOnOLED() {
  display.clearDisplay();
  display.setCursor(0,0);
  
  display.println("Air Quality Monitor");
  display.setTextSize(1);
  display.print("Suhu: ");
  display.print(temperature);
  display.println(" C");
  
  display.print("Kelembaban: ");
  display.print(humidity);
  display.println(" %");
  
  display.display(); // Update tampilan OLED
}

// Fungsi Callback untuk menerima pesan dari MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan diterima di topik: ");
  Serial.println(topic);

  Serial.print("Isi pesan: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Fungsi untuk reconnect ke MQTT
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Mencoba terhubung ke MQTT...");

    if (mqttClient.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Terhubung ke MQTT!");
      mqttClient.subscribe(mqttTopicSub);
    } else {
      Serial.print("Gagal terhubung, coba lagi dalam 2 detik...");
      delay(2000);
    }
  }
}
