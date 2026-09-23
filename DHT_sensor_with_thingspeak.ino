#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// Replace with your network credentials
const char* ssid = "Xiaomi13T";
const char* password = "11122233";

// Replace with your ThingSpeak API Key
String serverName = "http://api.thingspeak.com/update?api_key=ENTER API KEY";

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Timer setup
unsigned long lastTime = 0;
const unsigned long timerDelay = 5000; // 5 seconds delay

// DHT Sensor setup
#define DHT_PIN 26           // Pin connected to the DHT sensor (change if necessary)
#define DHT_TYPE DHT11       // DHT11 or DHT22 (change if you're using a DHT11)

DHT dht(DHT_PIN, DHT_TYPE); // Initialize DHT sensor

WiFiClient client;
HTTPClient http;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi, IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the DHT sensor
  dht.begin();

  // Initial message for clarity
  Serial.println("Timer set to 5 seconds. It will take 10 seconds before publishing the first reading.");
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  delay(10);
}

void loop() {
  // Check if it's time to send a new HTTP request
  if (millis() - lastTime >= timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      // Read temperature and humidity from DHT sensor
      float temperature = dht.readTemperature();  // Get temperature in Celsius
      float humidity = dht.readHumidity();       // Get humidity percentage
display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("T:"+String(temperature));
   display.print((char)247);
  display.print("C");
    display.setTextSize(2);
  display.setCursor(0, 40);
 display.print("H:"+String(humidity)+"%"); 
  display.display();
      // Check if reading was successful
      if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }
      // Construct the server path with temperature and humidity data
      String serverPath = serverName + "&field1=" + String(temperature) + "&field2=" + String(humidity);

      // Start HTTP request
      http.begin(client, serverPath.c_str());

      // Send HTTP GET request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);  // Print the response payload
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);  // Error code if request failed
      }
      // End the HTTP request and free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    // Update lastTime to manage the delay
    lastTime = millis();
  }
}