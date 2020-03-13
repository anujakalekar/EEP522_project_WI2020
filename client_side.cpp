// Loading the ESP8266WiFi library, PubSubClient library and sensor library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT22   // DHT 22 sensor for temperature and humidity

// set credentials to connect to the router
const char* ssid = "shuddha_halkatpana";
const char* password = "warle70rupye";

// IP address of Raspberry pi: Here Raspberry pi is server/broker
const char* mqtt_server = "192.168.0.26"; 

// Initializes the espClient
WiFiClient espClient;
PubSubClient client(espClient);

// DHT Sensor pin
const int DHTPin = 12; //D6
// Soil moisture sensors pin
const int soil_pin_1 = 14; //D5
const int soil_pin_2 = 2; //D4

float temp_fahrenheit = 0;
float temp_celsius = 0;
float humidity = 0;
float heat_index = 0; 
int temp_last_state = 0;  //temperature last state
int humd_last_state = 0;  //humidity last state
int soil1_curr_state = 0; //soil moisture sensor 1 current state: code for this is commented as read analog values of sensor 
int soil1_last_state = 0;    //soil moisture sensor 1 last state
int soil2_curr_state = 0; //soil moisture sensor 2 current state
int soil2_last_state = 0;    //soil moisture sensor 2 last state

double val_analog = 0.0;
double volts_analog = 0.0;
int moist_val = 0;
int last_moist_val = 0;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Timers auxiliar variables
long now = millis();
long last_measure = 0;

//connect to your router
void setup_wifi() {
  delay(10);
  //start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected : ");
  Serial.println(WiFi.localIP());
}



// This functions reconnects ESP8266 to MQTT broker(Raspberry pi)
void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

// Publish the messeges to the broker
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");
    
  now = millis();
  // Publishes new temperature, humidity and soil moisture
  if (now - last_measure > 10000) {
    last_measure = now;
    humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temp_celsius = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    temp_fahrenheit = dht.readTemperature(true);

    // Check for the failed reads
    if (isnan(humidity) || isnan(temp_celsius) || isnan(temp_fahrenheit)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Computes heat index in Celsius with the help of temperature and humidity
    heat_index = dht.computeHeatIndex(temp_celsius, humidity, false);
          
    static char humidityTemp[7];
    dtostrf(humidity, 6, 2, humidityTemp);

    // Publishes Temperature and Humidity values if not in limits 
    if(temp_celsius > 29.4 || temp_celsius < 26.7) { //less than val:26.7
      if(temp_last_state == 0){
        client.publish("/esp8266/temperature","ON");
        temp_last_state = 1;
      }
    }
    else {
      if(temp_last_state =1) {
        client.publish("/esp8266/temperature","ON");
      }
      temp_last_state = 0;
    }     
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t Temperature: ");
    Serial.print(temp_celsius);
    Serial.print(" *C ");
    Serial.print(temp_fahrenheit);
    Serial.print(" *F\t Heat index: ");
    Serial.print(heat_index);
    Serial.println(" *C \n");

    //Analog soil moisture value 
    val_analog = analogRead(A0); //dBm read value from pinA0
    // convert the analog signal to voltage. A0 reads between 0 and ~3 volts, This conversion produce value
    // between 0 and 1024. 
    volts_analog = (val_analog *3.08)/1024;
    
    // get moisture compare value by converting the analog (0-1024) value to a value between 0 and 100.
    // the value of 400 was determined by using a dry moisture sensor  (in air).
    moist_val = (val_analog*100)/400;
    moist_val = 100-moist_val;
    
   //Publishes soil moisture value
   if(moist_val <=25 && last_moist_val==0){
    client.publish("/esp8266/moisture","PIPE1_ON");
    last_moist_val = 1;
   }
   else if(moist_val>25 && last_moist_val ==1){
    client.publish("/esp8266/moisture","PIPE1_OFF");
    last_moist_val = 0;
   }
   
   /*  This part is to read digital values
   // Publishes Analog soil moisture value based on sensor 1 state
    soil1_curr_state = digitalRead(soil_pin_1);
    Serial.println("Soil moisture sensor 1 state: ");
    Serial.println(soil1_curr_state);
    if(soil1_curr_state == 1 && soil1_last_state ==0) {
      client.publish("/esp8266/moisture","PIPE1_ON");
      soil1_last_state = 1;
    }
    else if(soil1_curr_state == 0 && soil1_last_state ==1) {
      client.publish("/esp8266/moisture","PIPE1_OFF");
      soil1_last_state = 0;
    }
    */
    // Publishes Digital soil moisture value based on sensor 2 state
    soil2_curr_state = digitalRead(soil_pin_2);
    Serial.println("Soil moisture sensor 2 state: ");
    Serial.println(soil2_curr_state);
    if(soil2_curr_state == 1 && soil2_last_state ==0) {
      client.publish("/esp8266/moisture2","PIPE2_ON");
      soil2_last_state = 1;
    }
    else if(soil2_curr_state == 0 && soil2_last_state ==1) {
      client.publish("/esp8266/moisture2","PIPE2_OFF");
      soil2_last_state = 0;
    }
  }
}