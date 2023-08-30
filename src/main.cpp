#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define LIGHT_SENSORS_SIZE 3

#define WIFI_CONNECTED D4
#define LDR_PIN A0

const uint32_t station_id = 0xaf7c1fe6;

const char* ssid = "vodafone041107";
const char* password = "rpJtaXrLx9cLF6pG";

const char* server_name = "http://192.168.1.90:5000/update_sensor_value";

const uint8_t light_sensors[LIGHT_SENSORS_SIZE] = {D1, D2, D3};

uint8_t light_intensity;

void setup() 
{
	Serial.begin(9600);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	Serial.print("Connecting to "); Serial.println(ssid);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		Serial.print(".");
	}
	
	Serial.println("Connection established!");

	for (auto &&pin : light_sensors)
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}
	
	pinMode(LDR_PIN, INPUT);
}

long unsigned int last_time = millis();
int pin = 0;

void loop() 
{
	// Uses multiplexing to read the value of multiple sensors from just analog input pin
	// Delay added to avoid the  WiFi connection being lost due to overuse of A0
	if (millis() > (last_time + 1000))
	{
		// Loops through all of the pins every second
		if (pin >= LIGHT_SENSORS_SIZE - 1)
		{
			pin = 0;
		}
		else
		{
			pin++;
		}
				
		
		digitalWrite(light_sensors[pin], HIGH);

		// The A0 pin returns a value from 0 to 1024 as a 10 bit number
		// Since the luminocity program shows it as a colour, only an 8 bit
		// number is required to show the light intensity is required
		light_intensity = (analogRead(LDR_PIN) / 4) - 1;

		digitalWrite(light_sensors[pin], LOW);

		Serial.print("Sensor "); Serial.print(pin); Serial.print(": ");
		Serial.println(light_intensity);

		String json_data = 
			"{"
				"\"station_id\":" + String(station_id) + ","
				"\"sensor_id\":" + String(pin) + ","
				"\"type\":" + "0" + ","
				"\"val\":" + String(light_intensity) + ","
			"}";
		
		Serial.println(json_data);

		if (WiFi.status() == WL_CONNECTED)
		{
			WiFiClient client;
			HTTPClient http;

			http.begin(client, server_name);

			http.addHeader("Content-Type", "application/json");

			http.POST(
				"{"
					"\"station_id\":" + String(station_id) + ","
					"\"sensor_id\":" + String(pin) + ","
					"\"type\":" + "0" + ","
					"\"val\":" + String(light_intensity) +
				"}"
			);
		}

		last_time = millis();
	}		

}

