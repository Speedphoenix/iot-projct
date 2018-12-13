#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
constchar* ssid = "YourWiFiSSID ";
constchar* password =  "YourWiFiPassword";
constchar* mqttServer = "test.mosquitto.com";
constintmqttPort =1883;
constchar* mqttUser = "YourMQTTUsername";
constchar* mqttPassword = "YourMQTTPassword";
WiFiClient espClient;
PubSubClient client(espClient);
voidsetup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if(client.connect("ESP8266Client", mqttUser, mqttPassword )) 
{
     Serial.println("connected");
    }else{
      Serial.print("failed state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.publish("test/esp8266","Hello ESP World");
  client.subscribe("test/esp8266");// here is where you later add a wildcard
}
voidcallback(char* topic, byte* payload, unsigned intlength)
{
  Serial.print("Messageved in topic: ");
  Serial.println(topic);
  Serial.print("Message " )
  for(int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
voidloop()
{
  client.loop();
}
