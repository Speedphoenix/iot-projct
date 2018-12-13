/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen

 */
 
/** from: https://codebender.cc/sketch:356078#HC-SR04%20Ultrasonic%20Sensor%20Example.ino
* HC-SR04 Demo
* Demonstration of the HC-SR04 Ultrasonic Sensor* Date: August 3, 2016
*
* Description:
* Connect the ultrasonic sensor to the Arduino Uno as per the
* hardware connections below. Run the sketch and open a serial
* monitor. The distance read from the sensor will be displayed
* in centimeters and inches.
*
* Hardware Connections:
* Arduino | HC-SR04
* -------------------
* 5V | VCC
* 11 | Trig
* 12 | Echo
* GND | GND
*
* License:
* Public Domain
*/

#include <SPI.h>
#include <Ethernet.h>

//distancecaptor constants

// Pins
const int TRIG_PIN = 5;
const int ECHO_PIN = 6;
// Anything over 400 cm (23200 us pulse) is "out of range"
const unsigned int MAX_DIST = 23200;
const float MAX_CM = 70;


//  Hue constants
 
const char hueHubIP[] = "192.168.20.131";  // Hue hub IP
const char hueUsername[] = "j5pXiKBVqoD5oEFgwKAfeGO1NDrkoumlCTDFqv5r";  // Hue username
const int hueHubPort = 80;
 
const int hueBriMax = 255;  // max brightness value
const int hueBriStep = 64;  // brightness step size when button 1 is pressed (256/64 = 4 steps)
 
const long hueHueMax = 65535;  // max hue value
const long hueHueStep = 6553;  // hue step size when button 2 is pressed (65536/10922 = 6 steps)
 
//  Hue variables
 
int hueBriDir;  // holds brightness step direction for each remote/light (up=1,down=-1)
long hueHueDir;  // holds hue step direction for each remote/light (up=1,down=-1)
 
const unsigned int hueLight = 2;  // target light
boolean hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String hueCmd;  // Hue command
String gotGet;

long offlineHue;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x06, 0x00, 0x30 };
//E5-FA-98-2E-96-8D
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;


boolean GetHue()
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    client.print("GET /api/");
    client.print(hueUsername);
    client.print("/lights/");
    client.print(hueLight);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(hueHubIP);
    client.println("Content-type: application/json");
    client.println("keep-alive");
    client.println();
    while (client.connected())
    {
      if (client.available())
      {
        gotGet = client.readStringUntil('\0');
        /*
        client.findUntil("\"on\":", "\0");
        hueOn = (client.readStringUntil(',') == "true");  // if light is on, set variable to true

        client.findUntil("\"bri\":", "\0");
        hueBri = client.readStringUntil(',').toInt();  // set variable to brightness value
 
        client.findUntil("\"hue\":", "\0");
        hueHue = client.readStringUntil(',').toInt();  // set variable to hue value
*/
        break;  // not capturing other light attributes yet
      }
    }
    client.stop();
    return true;  // captured on,bri,hue
  }
  else
    return false;  // error reading on,bri,hue
}
/*
 
    SetHue
    Set light state using hueCmd command
 
*/
boolean SetHue()
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    while (client.connected())
    {
      client.print("PUT /api/");
      client.print(hueUsername);
      client.print("/lights/");
      client.print(hueLight);
      client.println("/state HTTP/1.1");
      client.println("keep-alive");
      client.print("Host: ");
      client.println(hueHubIP);
      client.print("Content-Length: ");
      client.println(hueCmd.length());
      client.println("Content-Type: text/plain;charset=UTF-8");
      client.println();  // blank line before body
      client.println(hueCmd);  // Hue command
    }
    client.stop();
    return true;  // command executed
  }
  else
    return false;  // command failed
}

void showState()
{
  Serial.println("\nHere's the whole GET:");
  Serial.println(gotGet);
  /*
  Serial.print("light on: ");
  Serial.println(hueOn);
  Serial.print("brightness: ");
  Serial.println(hueBri);
  Serial.print("Hue value: ");
  Serial.println(hueHue);
  */
}

void setup() {

  // The Trigger pin will tell the sensor to range find, since the default mode is input we don’t need to define Echo pin as input
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
  offlineHue = 0;
  hueHueDir = 1;
}

void loop() {
  unsigned long t1;
  unsigned long t2;
  unsigned long pulse_width;
  float cm;
  Serial.println();
  Serial.println();
  // Hold the trigger pin high for at least 10 us
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Wait for pulse on echo pin
  while ( digitalRead(ECHO_PIN) == 0 );
  // Measure how long the echo pin was held high (pulse width)
  // Note: the micros() counter will overflow after ~70 min
  t1 = micros();
  while ( digitalRead(ECHO_PIN) == 1);
  t2 = micros();
  pulse_width = t2 - t1;
  // Calculate distance in centimeters and inches. The constants
  // are found in the datasheet, and calculated from the assumed speed//of sound in air at sea level (~340 m/s).
  cm = pulse_width / 58.0;
  // Print out results

  if ( pulse_width > MAX_DIST ) {
    Serial.println("Out of range");
  } else {
    Serial.print(cm);
    Serial.println(" cm \t");
  }
  if (GetHue())  // get light state
  {
    showState();
    hueCmd = "";  // initialize command

   //else if ((hueButton == 0) && !hueOn)  // toggle light On
    hueCmd = "{\"on\":true";
    hueCmd += ",\"bri\":1";

    if (pulse_width > MAX_DIST || cm > MAX_CM)
      offlineHue = hueHueMax;
    else
      offlineHue = cm * hueHueMax / MAX_CM;
    if (offlineHue > hueHueMax)
      offlineHue = hueHueMax;
    else if (offlineHue < 0)
      offlineHue  = 0;
    
    /*offlineHue += hueHueStep * hueHueDir;
    if (offlineHue > hueHueMax)
    {
      hueHueDir = -1;
      offlineHue -= hueHueStep;
    }
    else if (offlineHue <= 0)
    {
      hueHueDir = 1;
      offlineHue += hueHueStep;
    }*/
    hueCmd += ",\"hue\":" + String(offlineHue) + "}";
    SetHue();
  }
//  delay(100);
}

