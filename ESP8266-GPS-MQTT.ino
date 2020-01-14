#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   9600-baud serial GPS device hooked up on pins 8(rx) and 9(tx) and a HMC5883 Magnetic Compass
   connected to the SCL/SDA pins.
*/

static const int RXPin = D6, TXPin = D7;
static const uint32_t GPSBaud = 9600;

// Assign a Uniquej ID to the HMC5883 Compass Sensor
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the NEO-6m GPS module
SoftwareSerial ss(RXPin, TXPin);
///////////////////////////////// set wifi ////////////////////////////////////////////////
const char* ssid = "PetCE";
const char* password = "Pet232541";
///////////////////////////////// set mqtt_server ////////////////////////////////////////////////
const char* mqtt_server = "tailor.cloudmqtt.com"; //IP Address or Domain ของ MQTT Server
const char mqtt_username[50] = "nrwkpslr"; //Username ของ MQTT Server
const char mqtt_password[50] = "1vFxwOFUAvdM"; //Password ของ MQTT Server
const int mqtt_port = 14382;
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
  client.setCallback(callback);
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
//////////////////////////////////////////////////// MQTT CALLBACK /////////////////////////////////////////////////////////////////////
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    Serial.print("LOW");
  } else {
    Serial.print("HIGH"); // Turn the LED off by making the voltage HIGH
  }
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
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement…
      client.publish("test/gps", "This is GPS");
      // … and resubscribe
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

  Serial.print(F("  Date/Time: "));
  // prints the recieved GPS module date if it was decoded in a valid response.
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    // prints invalid otherwise.
    Serial.print(F("INVALID"));
  }
  Serial.print(F(" "));
  // prints the recieved GPS module time if it was decoded in a valid response.
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    // Print invalid otherwise.
    Serial.print(F("INVALID"));
  }
  Serial.println();
  if (mag.begin())
  {
    displayCompassInfo();
  }
}
//////////////////////////////////////////////////// X,Y,Z /////////////////////////////////////////////////////////////////////
void displayCompassInfo()
{
  /* Get a new sensor event */
  sensors_event_t event;
  mag.getEvent(&event);

  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  "); Serial.println("uT");

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);

  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.05;
  heading += declinationAngle;

  // Correct for when signs are reversed.
  if (heading < 0)
    heading += 2 * PI;

  // Check for wrap due to addition of declination.
  if (heading > 2 * PI)
    heading -= 2 * PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180 / M_PI;

  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);

  delay(3000);
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
  if (gps.location.isValid())
  {
    client.publish("test/gps", String(gps.location.lat(), 6).c_str());
    client.publish("test/gps", String(gps.location.lng(), 6).c_str());
  }
}
