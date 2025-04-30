#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <DHT.h>

#define LIGHT_SENSORS 2
#define MOTION_SENSORS 1
#define DHT11_SENSORS 1

#define LDR_PIN A0
#define PIR_PIN D3
#define DHT_PIN D4

#define DHTTYPE DHT11

const uint32_t station_id = 0xaf7c1fe6; // Arbritrary number. Should make this randomly generated in the future. Could be stored in EEPROM.

const char* ssid = "*************";
const char* password = "**************";

const char* server_name = "***********************";

const uint8_t light_sensors[LIGHT_SENSORS] = {D1, D2};

uint8_t light_intensity;

DHT dht_sensor(DHT_PIN, DHTTYPE);

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

	dht_sensor.begin();

	pinMode(PIR_PIN, INPUT_PULLUP);

}

long unsigned int last_time = millis();
int pin = 0;

void loop() 
{
	// Uses multiplexing to read the value of multiple sensors from just analog input pin
	// Delay added to avoid the  WiFi connection being lost due to overuse of A0
	if (millis() > (last_time + 1000))
	{
		if (pin >= LIGHT_SENSORS + MOTION_SENSORS + DHT11_SENSORS - 1) 
		{
			pin = 0;
		}
		else
		{
			pin++;
		}
		
		if (pin < LIGHT_SENSORS)
		{
			digitalWrite(light_sensors[pin], HIGH);

			// The A0 pin returns a value from 0 to 1024 as a 10 bit number
			// Since the luminocity program shows it as a colour, only an 8 bit
			// number is required to show the light intensity is required
			light_intensity = (analogRead(LDR_PIN) / 4) - 1;

			digitalWrite(light_sensors[pin], LOW);


			String json_data = 
				"{"
					"\"station_id\":" + String(station_id) + ","
					"\"sensor_id\":" + String(pin) + ","
					"\"type\":" + "0" + ","
					"\"val\":" + String(light_intensity) +
				"}";
			
			Serial.print("LIGHT "); Serial.print(json_data); Serial.print("\n");

			if (WiFi.status() == WL_CONNECTED)
			{
				WiFiClient client;
				HTTPClient http;

				http.begin(client, server_name);

				http.addHeader("Content-Type", "application/json");

				http.POST(json_data);
			}
		}
		else if (pin < LIGHT_SENSORS + DHT11_SENSORS)
		{
			String json_data = 
				"{"
					"\"station_id\":" + String(station_id) + ","
					"\"sensor_id\":" + "78" + ","
					"\"type\":" + "1" + ","
					"\"val\":" + "[" + String(dht_sensor.readTemperature()) + ","
									 + String(dht_sensor.readHumidity()) + 
								  "]"
				"}";

			Serial.print("DHT11 "); Serial.print(json_data); Serial.print("\n");

			if (WiFi.status() == WL_CONNECTED)
			{
				WiFiClient client;
				HTTPClient http;

				http.begin(client, server_name);

				http.addHeader("Content-Type", "application/json");

				http.POST(json_data);
			}
		}
		else if (pin < LIGHT_SENSORS + DHT11_SENSORS + MOTION_SENSORS) 
		{

			String json_data = 
				"{"
					"\"station_id\":" + String(station_id) + ","
					"\"sensor_id\":" + "29" + ","
					"\"type\":" + "2" + ","
					"\"val\":" + (digitalRead(PIR_PIN) == LOW ? "true" : "false") +
				"}";

			Serial.print("MOTION "); Serial.print(json_data); Serial.print("\n");

			if (WiFi.status() == WL_CONNECTED)
			{
				WiFiClient client;
				HTTPClient http;

				http.begin(client, server_name);

				http.addHeader("Content-Type", "application/json");

				http.POST(json_data);
			}
		}
		
		last_time = millis();
	}		
}

