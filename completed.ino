#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
//#include <MemoryFree.h>
// Update these with values suitable for your network.
//byte mac[] = { 0x64, 0x1F, 0x7F, 0x1F, 0xAB, 0x35 };
byte mac[] = { 0x90, 0xA2, 0xDA, 0x06, 0x00, 0x30 };

//  Hue constants
 
//const char hueHubIP[] = "192.168.20.93";  // Hue hub IP

IPAddress hueHubIP(192,168,1,214);  // Hue hub IP
const char hueUsername[] = "OqQP9WDrHorWMFttPiuGhzSSV2sW0K29x8ZWVzWd";  // Hue username
const int hueHubPort = 80;

//  Hue variables
 
unsigned int hueLight = 2;  // target light (designed to be changed)
boolean hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String gotGet; // get message from lamp status
String hueCmd;  // Hue command
String message_buff; // Message received from MQTT

long offlineHue;

bool altern_on = true;
bool altern_on_tmp = true;
bool switch_on = true;

// Set the static IP address to use if the DHCP fails to assign
const IPAddress ip(192, 168, 0, 177);

EthernetClient ethClient;
EthernetClient ethClient2;
PubSubClient client;
PubSubClient client2;

const char *mqtt_server = "m15.cloudmqtt.com";
const int mqtt_port = 13274;
const char *mqtt_user = "ldwqzejh";
const char *mqtt_pass = "PkkXIJ8X5fdM";
const char *mqtt_client_name = "Maxime";

/*
int char_length(char* str)
{
  int l = 0;
  while(str[l] != '\0')
    l++;

  return l;
} */

int availableMemory() {
  int size = 2048; // Use 2048 with ATmega328
  byte *buf;

  while ((buf = (byte *) malloc(--size)) == NULL)
    ;

  free(buf);

  return size;
}

boolean GetHue()
{
  if (ethClient.connect(hueHubIP, hueHubPort))
  { 
    
    ethClient.print("GET /api/");
    ethClient.print(hueUsername);
    ethClient.print("/lights/");
    ethClient.print(hueLight);
    ethClient.println(" HTTP/1.1");
    ethClient.print("Host: ");
    ethClient.println(hueHubIP);
    ethClient.println("Content-type: application/json");
    ethClient.println("keep-alive");
    ethClient.println();
    
    while (ethClient.connected())
    {
      if (ethClient.available())
      {
        gotGet = "";/*
        Serial.print("av: ");
        Serial.print(availableMemory());
        Serial.print(" "); */
        ethClient.readStringUntil('{');
        gotGet += ethClient.readStringUntil(',');
        gotGet += ethClient.readStringUntil(',');
        gotGet += ethClient.readStringUntil(',');
        ethClient.readStringUntil('\0');
        Serial.println(availableMemory());
        
        Serial.print("gGet: ");
        Serial.println(gotGet);
        Serial.print("hueCmd: ");
        Serial.println(hueCmd);
        Serial.print("msgRec: ");
        Serial.println(message_buff);
        
        
        
        break;  // not capturing other light attributes yet
      }
    }
    
    ethClient.stop();
    return true;  // captured on,bri,hue
  }
  else
    return false;  // error reading on,bri,hue
}


boolean SetHue()
{
  if (ethClient.connect(hueHubIP, hueHubPort))
  {
    while (ethClient.connected())
    {
      ethClient.print("PUT /api/");
      ethClient.print(hueUsername);
      ethClient.print("/lights/");
      ethClient.print(hueLight);
      ethClient.println("/state HTTP/1.1");
      ethClient.println("keep-alive");
      ethClient.print("Host: ");
      ethClient.println(hueHubIP);
      ethClient.print("Content-Length: ");
      ethClient.println(hueCmd.length());
      //ethClient.println(char_length(hueCmd));
      ethClient.println("Content-Type: text/plain;charset=UTF-8");
      ethClient.println();  // blank line before body
      ethClient.println(hueCmd);  // Hue command
    }
    ethClient.stop();
    return true;  // command executed
  }
  else
  {
    Serial.println("Fail to SetHue");
    return false;  // command failed
  }
    
}

void showState()
{
  //Serial.println("\nHere's the whole GET:");
  //Serial.println(gotGet);
}


void callback(char* topic, byte* payload, unsigned int length) {
  
  /// Incoming message 
  Serial.print("Message arrived from CloudMQTT [");
  Serial.print(topic);
  hueLight = topic[5] - '0';
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message_buff += (char)payload[i];
  }
  Serial.println();
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection ... ");
    // Attempt to connect
    if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      //client.subscribe("foo/bar");
      client.subscribe("lamp/1");
      client.subscribe("lamp/2");
      client.subscribe("lamp/3");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup()
{
  Serial.begin(9600);

  client.setClient(ethClient2);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  } 
  
  // Allow the hardware to sort itself out
  delay(1000);
  offlineHue = 20000;


}

void loop()
{
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  hueCmd = "";  // initialize command

  /*
  if (switch_on)  // get light state
  {
    if( GetHue() )
    {
        if(altern_on)
        {
          hueCmd = "{\"on\":true";
          altern_on_tmp = false;     
        }
        else
        {
          hueCmd = "{\"on\":false";
          //hueCmd = "{\"on\":true";
          altern_on_tmp = true;
        }
    
        altern_on = altern_on_tmp;
        
        hueCmd += ",\"bri\":50";
        hueCmd += ",\"hue\":" + String(offlineHue) + "}";
        
        SetHue();
    }
    else
    {
      Serial.println("Fail to getHue");
    }
  }
  */
  


  if(message_buff != "") // When a message is received, stop alterning on/off
  {
    //Serial.println(message_buff);

    hueCmd = message_buff;
    //hueCmd += ",\"bri\":1";
    //hueCmd += ",\"hue\":" + String(offlineHue) + "}";
    Serial.println("Message sent to Hub: ");
    Serial.print(hueCmd);
    SetHue();
    
    message_buff = "";
    switch_on = false;
  }

      /// Sending message of the state of all lamps
    char tmp_get[50];
    
    hueLight = 1;
    if ( GetHue() ) {}
    else Serial.println("Fail to GetHue of lamp 1");
    for(int i=0; i<gotGet.length(); i++) // Copy gotGet to a char[50]
      tmp_get[i] = gotGet[i];
    tmp_get[gotGet.length()] = '\0';
    client.publish("lampstate/1", tmp_get);
    tmp_get[0] = '\0';
    
    hueLight = 2;
    if ( GetHue() ) {}
    else Serial.println("Fail to GetHue of lamp 2");
    for(int i=0; i<gotGet.length(); i++) // Copy gotGet to a char[50]
      tmp_get[i] = gotGet[i];
    tmp_get[gotGet.length()] = '\0';
    client.publish("lampstate/2", tmp_get);
    tmp_get[0] = '\0';
    
    hueLight = 3;
    if ( GetHue() ) {}
    else Serial.println("Fail to GetHue of lamp 3");
    for(int i=0; i<gotGet.length(); i++) // Copy gotGet to a char[50]
      tmp_get[i] = gotGet[i];
    tmp_get[gotGet.length()] = '\0';
    client.publish("lampstate/3", tmp_get);
    tmp_get[0] = '\0';



  delay(3000);
  
}
