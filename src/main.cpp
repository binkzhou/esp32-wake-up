#include "WiFi.h"
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);
 DynamicJsonDocument doc(1024);
WiFiUDP udp;

boolean  rev_flag =  false;


void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
  byte mac[6];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

    deserializeJson(doc, payload);
    JsonObject obj = doc.as<JsonObject>();
    String mac_str = obj[String("mac")];
    String ip_str = obj[String("ip")];
    uint16_t p = obj[String("port")];
    Serial.println(mac_str);
    Serial.println(ip_str);
    parseBytes(mac_str.c_str(), ':', mac, 6, 16);
  
    udp.beginPacket(ip_str.c_str(),p);
    for (int i = 0; i < 6; i++) {
      udp.printf("%c",0xff); 
    }
    for (int i = 0; i < 16; i++) {
      udp.printf("%c%c%c%c%c%c", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]); 
    }

    udp.endPacket();
}




void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    String con_name;
    con_name = "arduinoClient"+ String(random(10020));
    Serial.println("connect name: " + con_name);
    if (client.connect(con_name.c_str())) {
      Serial.println("connected");
      client.subscribe("wakeup");
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
  Serial.begin(115200);

  //Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  //Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("SmartConfig received.");

  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());



  client.setServer("www.fenghuazhengmao.top", 1882);
  client.setCallback(callback);

  udp.begin(9);

  delay(1500);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}