# RPi,MQTT,Flask import
import time 
import RPi.GPIO as GPIO 
import paho.mqtt.client as mqtt
from flask import Flask, render_template, request

# Configuration: 
LED_PIN        = 24
SENS_PIN       = 25
SENS2_PIN      = 23
#initialize GPIO status variables
fan_status = 0
pipe1_status = 0
pipe2_status = 0
# Initialize GPIO for LED and button. 
GPIO.setmode(GPIO.BCM) 
GPIO.setwarnings(False) 
GPIO.setup(LED_PIN, GPIO.OUT)
GPIO.setup(SENS_PIN, GPIO.OUT)
GPIO.setup(SENS2_PIN, GPIO.OUT)  
GPIO.output(LED_PIN, GPIO.HIGH)
GPIO.output(SENS_PIN, GPIO.HIGH)  
GPIO.output(SENS2_PIN, GPIO.HIGH) 

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/esp8266/temperature")
    client.subscribe("/esp8266/moisture")
    client.subscribe("/esp8266/moisture2")

# The callback for when a PUBLISH message is received from the ESP8266.
def on_message(client, userdata, message):
    print("Received message '" + str(message.payload) + "' on topic '"
        + message.topic + "' with QoS " + str(message.qos))
    if message.topic == "/esp8266/temperature":
        #print("temperature update")
        if message.payload == b'ON': 
           GPIO.output(LED_PIN, GPIO.LOW)
        if message.payload == b'OFF': 
           GPIO.output(LED_PIN, GPIO.HIGH)
	    
    if message.topic == "/esp8266/moisture":
        if message.payload == b'PIPE1_ON':
            GPIO.output(SENS_PIN, GPIO.LOW)
        if message.payload == b'PIPE1_OFF':
            GPIO.output(SENS_PIN, GPIO.HIGH)
    
    if message.topic == "/esp8266/moisture2":
        if message.payload == b'PIPE2_ON':
            GPIO.output(SENS2_PIN, GPIO.LOW)
        if message.payload == b'PIPE2_OFF':
            GPIO.output(SENS2_PIN, GPIO.HIGH)
    
mqttclient=mqtt.Client()
mqttclient.on_connect = on_connect
mqttclient.on_message = on_message
mqttclient.connect("localhost",1883,60)
mqttclient.loop_start()
   
@app.route("/")
def index():
	# Read Sensors Status
	fan_status = GPIO.input(LED_PIN)
	pipe1_status = GPIO.input(SENS_PIN)
	pipe2_status = GPIO.input(SENS2_PIN)
	templateData = {
              'title' : 'GPIO output Status!',
              'LED_PIN'  : fan_status,
              'SENS_PIN'  : pipe1_status,
              'SENS2_PIN'  : pipe2_status,
        }
	return render_template('index.html', **templateData)

@app.route("/<deviceName>/<action>")
def action(deviceName, action):
	if deviceName == 'LED_PIN':
		actuator = LED_PIN
	if deviceName == 'SENS_PIN':
		actuator = SENS_PIN
	if deviceName == 'SENS2_PIN':
		actuator = SENS2_PIN
   
	if action == "on":
		GPIO.output(actuator, GPIO.LOW)
	if action == "off":
		GPIO.output(actuator, GPIO.HIGH)
		     
	fan_status   = GPIO.input(LED_PIN)
	pipe1_status = GPIO.input(SENS_PIN)
	pipe2_status = GPIO.input(SENS2_PIN)
   
	templateData = {
              'LED_PIN'  : fan_status,
              'SENS_PIN'  : pipe1_status,
              'SENS2_PIN'  : pipe2_status,
	}
	return render_template('index.html', **templateData)
    
    
if __name__ == "__main__":
   app.run(host='0.0.0.0', port=80, debug=True)

