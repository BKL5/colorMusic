#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define LEDS 156

#define MIN_MIREDS 250
#define MAX_MIREDS 454

#define SMOOTHNESS 0.25
#define EXPONENT 1.25
//#define LOW_PASS_FILTER 0

#define WIFI_SSID "FLORENTA_2.4_GHZ"
#define WIFI_PASSWORD "javu3RuN"

#define MQTT_SERVER "10.254.1.11"
#define MQTT_CLIENTID "NodeMcu Nr. 1"
#define MQTT_USERNAME "nodemcu_nr_1"
#define MQTT_PASSWORD "nodemcu_nr_1"

#define OTA_HOSTNAME "nodemcu-nr-1"
#define OTA_PASSWORD "javu3RuN"

CRGB leds[LEDS];

int hue;

WiFiClient wiFiClient;
PubSubClient pubSubClient(wiFiClient);

bool bar_nr_1_lamp_nr_1_2_ws2812b_state = false, bar_nr_1_lamp_nr_1_2_state = false;
int bar_nr_1_lamp_nr_1_2_ws2812b_brightness = 255, bar_nr_1_lamp_nr_1_2_brightness = 255, bar_nr_1_lamp_nr_1_2_color_temp = 454;
const char* bar_nr_1_lamp_nr_1_2_ws2812b_effect = "rainbow";

String bar_nr_1_lamp_nr_1_2_ws2812b_effectString = "rainbow";

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, 5, GRB>(leds, LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 6, GRB>(leds, LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(bar_nr_1_lamp_nr_1_2_ws2812b_brightness);

  pubSubClient.setServer(MQTT_SERVER, 1883);
  pubSubClient.setCallback(callback);

  wifiConnect();

  mqttConnect();

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End failed");
    }
  });

  ArduinoOTA.begin();
  Serial.println("Ready");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected");

    wifiConnect();
  }

  if (!pubSubClient.connected()) {
    Serial.println("MQTT disconnected");

    mqttConnect();
  }

  pubSubClient.loop();

  FastLED.setBrightness(bar_nr_1_lamp_nr_1_2_ws2812b_brightness);

  unsigned long hue_timer;

  if (millis() - hue_timer > 15) {
    if (++hue >= 255) hue = 0;
    hue_timer = millis();
  }

  if (bar_nr_1_lamp_nr_1_2_ws2812b_state) {
    if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "rainbow") {
      fill_rainbow(leds, LEDS, hue, 1);

      FastLED.show();
    } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "purple") {
      fill_solid(leds, LEDS, CRGB::Purple);

      FastLED.show();
    } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "pink") {
      fill_solid(leds, LEDS, CRGB::Pink);

      FastLED.show();
    } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "white") {
      fill_solid(leds, LEDS, CRGB::White);

      FastLED.show();
    } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-rainbow" || bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-purple" || bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-pink" || bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-white") {
      unsigned long timer;

      if (millis() - timer > 5) {
        float level;

        for (byte i = 0; i < 100; i ++) {
          float analogReadA0 = analogRead(A0);

          if (level < analogReadA0) level = analogReadA0;

          //level = map(level, LOW_PASS_FILTER, 1023, 0, 500);

          level = constrain(level, 0, 500);

          level = pow(level, EXPONENT);

          float levelFiltered = level * SMOOTHNESS + levelFiltered * (1 - SMOOTHNESS);

          if (levelFiltered > 15) {

            float averageLevel = levelFiltered * 0.005 + averageLevel * (1 - 0.005);

            float maximumLevel = averageLevel * 1.75;

            byte length = map(levelFiltered, 0, maximumLevel, 0, LEDS / 2);

            length = constrain(length, 0, LEDS / 2);

            byte count = 0;
            for (int i = (LEDS / 2 - 1); i > ((LEDS / 2 - 1) - length); i--) {
              if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-rainbow") {
                leds[i] = ColorFromPalette(RainbowColors_p, (count * 255 / LEDS / 2) / 2 - hue);
              } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-purple") {
                leds[i] = CRGB(CRGB::Purple);
              } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-pink") {
                leds[i] = CRGB(CRGB::Pink);
              } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-white") {
                leds[i] = CRGB(CRGB::White);
              }
              count++;
            }
            count = 0;
            for (int i = (LEDS / 2); i < (LEDS / 2 + length); i++ ) {
              if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-rainbow") {
                leds[i] = ColorFromPalette(RainbowColors_p, (count * 255 / LEDS / 2) / 2 - hue);
              } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-purple") {
                leds[i] = CRGB(CRGB::Purple);
              } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-pink") {
                leds[i] = CRGB(CRGB::Pink);
              } else if (bar_nr_1_lamp_nr_1_2_ws2812b_effectString == "vu-meter-white") {
                leds[i] = CRGB(CRGB::White);
              }
            }
          }
        }

        FastLED.show();
        //FastLED.clear();

        timer = millis();
      }
    }
  } else {
    fill_solid(leds, LEDS, CRGB::Black);

    FastLED.show();
  }
  yield();
}


void wifiConnect() {
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(10);
  }

  Serial.println("Wi-Fi connected");
}

void mqttConnect() {
  while (!pubSubClient.connected()) {
    if (pubSubClient.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) {
      pubSubClient.subscribe("light/bar_nr_1_lamp_nr_1_2_ws2812b/command_topic");
      pubSubClient.subscribe("light/bar_nr_1_lamp_nr_1_2/command_topic");
      pubSubClient.subscribe("light/bar_nr_1_shelves_nr_1_2_3/command_topic");

      Serial.println("MQTT connected");

      publish();
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char json[length];

  for (int i = 0; i < length; i++) {
    json[i] = (char)payload[i];
  }
  
  Serial.println(topic);
  Serial.println(json);

  StaticJsonDocument<JSON_OBJECT_SIZE(10)> staticJsonDocument;

  deserializeJson(staticJsonDocument, json);

  if (topic == "light/bar_nr_1_lamp_nr_1_2_ws2812b/command_topic") {
    if (staticJsonDocument.containsKey("state")) {
      if (strcmp(staticJsonDocument["state"], "ON") == 0) {
        bar_nr_1_lamp_nr_1_2_ws2812b_state = true;
        
        if (staticJsonDocument.containsKey("brightness")) {
          bar_nr_1_lamp_nr_1_2_ws2812b_brightness = staticJsonDocument["brightness"];
        }
        if (staticJsonDocument.containsKey("effect")) {
          bar_nr_1_lamp_nr_1_2_ws2812b_effect = staticJsonDocument["effect"];

          bar_nr_1_lamp_nr_1_2_ws2812b_effectString = bar_nr_1_lamp_nr_1_2_ws2812b_effect;
        }
      }
      else if (strcmp(staticJsonDocument["state"], "OFF") == 0) {
        bar_nr_1_lamp_nr_1_2_ws2812b_state = false;
      }
    }
  } else if (topic == "light/bar_nr_1_lamp_nr_1_2/command_topic") {
    if (staticJsonDocument.containsKey("state")) {
      if (strcmp(staticJsonDocument["state"], "ON") == 0) {
        bar_nr_1_lamp_nr_1_2_state = true;
        
        if (staticJsonDocument.containsKey("brightness")) {
          bar_nr_1_lamp_nr_1_2_brightness = staticJsonDocument["brightness"];
        }
        if (staticJsonDocument.containsKey("color_temp")) {
          bar_nr_1_lamp_nr_1_2_color_temp = staticJsonDocument["color_temp"];
        }
      }
      else if (strcmp(staticJsonDocument["state"], "OFF") == 0) {
        bar_nr_1_lamp_nr_1_2_state = false;
      }
    }
  } else if (topic == "light/bar_nr_1_shelves_nr_1_2_3/command_topic") {

  }
  publish();
}

void publish() {
  StaticJsonDocument<JSON_OBJECT_SIZE(3)> bar_nr_1_lamp_nr_1_2_ws2812b_staticJsonDocument;

  if (bar_nr_1_lamp_nr_1_2_ws2812b_state) {
    bar_nr_1_lamp_nr_1_2_ws2812b_staticJsonDocument["state"] = "ON";
    bar_nr_1_lamp_nr_1_2_ws2812b_staticJsonDocument["brightness"] = bar_nr_1_lamp_nr_1_2_ws2812b_brightness;
    bar_nr_1_lamp_nr_1_2_ws2812b_staticJsonDocument["effect"] = bar_nr_1_lamp_nr_1_2_ws2812b_effect;
  } else {
    bar_nr_1_lamp_nr_1_2_ws2812b_staticJsonDocument["state"] = "OFF";
  }

  char bar_nr_1_lamp_nr_1_2_ws2812b_char[1024];

  serializeJson(bar_nr_1_lamp_nr_1_2_ws2812b_staticJsonDocument, bar_nr_1_lamp_nr_1_2_ws2812b_char);

  pubSubClient.publish("light/bar_nr_1_lamp_nr_1_2_ws2812b/state_topic", bar_nr_1_lamp_nr_1_2_ws2812b_char, true);

  StaticJsonDocument<JSON_OBJECT_SIZE(5)> bar_nr_1_lamp_nr_1_2_staticJsonDocument;

  if (bar_nr_1_lamp_nr_1_2_state) {
    bar_nr_1_lamp_nr_1_2_staticJsonDocument["state"] = "ON";
    bar_nr_1_lamp_nr_1_2_staticJsonDocument["min_mireds"] = MIN_MIREDS;
    bar_nr_1_lamp_nr_1_2_staticJsonDocument["max_mireds"] = MAX_MIREDS;
    bar_nr_1_lamp_nr_1_2_staticJsonDocument["brightness"] = bar_nr_1_lamp_nr_1_2_brightness;
    bar_nr_1_lamp_nr_1_2_staticJsonDocument["color_temp"] = bar_nr_1_lamp_nr_1_2_color_temp;
  } else {
    bar_nr_1_lamp_nr_1_2_staticJsonDocument["state"] = "OFF";
    bar_nr_1_lamp_nr_1_2_staticJsonDocument["min_mireds"] = MIN_MIREDS;
    bar_nr_1_lamp_nr_1_2_staticJsonDocument["max_mireds"] = MAX_MIREDS;
  }

  char bar_nr_1_lamp_nr_1_2_char[1024];

  serializeJson(bar_nr_1_lamp_nr_1_2_ws2812b_staticJsonDocument, bar_nr_1_lamp_nr_1_2_char);

  pubSubClient.publish("light/bar_nr_1_lamp_nr_1_2/state_topic", bar_nr_1_lamp_nr_1_2_char, true);

}
