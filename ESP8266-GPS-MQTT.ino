#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

static const int RXPin = D6, TXPin = D7;
static const uint32_t GPSBaud = 9600;

// Assign a Uniquej ID to the HMC5883 Compass Sensor
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the NEO-6m GPS module
SoftwareSerial ss(RXPin, TXPin);
///////////////////////////////// set wifi ////////////////////////////////////////////////
const char* ssid = "CEIOT-WIFI";
const char* password = NULL;
///////////////////////////////// set mqtt_server ////////////////////////////////////////////////
const char* mqtt_server = "172.16.20.12"; //IP Address or Domain ของ MQTT Server
const char mqtt_username[50] = "admin01"; //Username ของ MQTT Server
const char mqtt_password[50] = "Passw0rd"; //Password ของ MQTT Server
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
/////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(9600);
  ss.begin(GPSBaud);
  Serial.println(TinyGPSPlus::libraryVersion());
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  //  client.setCallback(callback);
  displaySensorDetails();
}
//////////////////////////////////////////////////// WIFI /////////////////////////////////////////////////////////////////////
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
}
//////////////////////////////////////////////////// Device /////////////////////////////////////////////////////////////////////
void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("");
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(3000);
}
//////////////////////////////////////////////////// Reconnect MQTT CALLBACK /////////////////////////////////////////////////////////////////////
void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      //, mqtt_username, mqtt_password
      Serial.println("connected");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//////////////////////////////////////////////////// location /////////////////////////////////////////////////////////////////////
void displayGpsInfo()
{
  // Prints the location if lat-lng information was recieved
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  // prints invalid if no information was recieved in regards to location.
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date:  "));
  // prints the recieved GPS module date if it was decoded in a valid response.
  if (gps.date.isValid())
  {
    String DataDate = String(gps.date.day()) + String("/") + String(gps.date.month()) + String("/") + String(gps.date.year());
    Serial.print(DataDate);
  }
  else
  {
    // prints invalid otherwise.
    Serial.print(F("INVALID"));
  }
  Serial.print(F("  Time:  "));
  // prints the recieved GPS module time if it was decoded in a valid response.
  if (gps.time.isValid())
  {
    String DataTime = String(gps.time.hour()) + String(":") + String(gps.time.minute()) + String(":") + String(gps.time.second()) + String(gps.time.centisecond());
    Serial.print(DataTime);
  }
  else
  {
    // Print invalid otherwise.
    Serial.print(F("INVALID"));
  }
  Serial.println("");
  delay(5000);
}
//////////////////////////////////////////////////// LOOP /////////////////////////////////////////////////////////////////////
void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded from the GPS Module.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayGpsInfo();
  //////////////////////////////////////////////////// MQTT /////////////////////////////////////////////////////////////////////
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid())
  {
    String local = String(WiFi.macAddress()) + "," + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    client.publish("GPS/Locate", local.c_str());
    delay(5000);
  }
}
