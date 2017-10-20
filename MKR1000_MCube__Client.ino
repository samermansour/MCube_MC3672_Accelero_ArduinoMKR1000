/*Simple Sketch to send accelerometer data from MCube MC3672
  through a TCP client socket running on this MKR1000 to a TCP server socket.
  ATTENTION ------> Change the server IP below and port as needed.
  If you don't want serial debugging change
  serialDebug to false. In fact you must change it to False when
  the MKR is working without being directly connected to the computer.
  By SM based on the WiFi 101 library example.
*/
#include <SPI.h>
#include <WiFi101.h>
#include <MC3672.h>

#define STX 0x02
#define ETX 0x03
#define NUL ','

MC3672 MC3672_acc = MC3672();
int Ts            = 1000;//sampling period

byte ServerIPAddr [] = {192, 168, 8, 105};//Your server's IP address
int port             = 5555;
bool serialDebug     = true;//Do you want serial debugging?

char ssid[]   = "// your network SSID (name)";
char pass[]   = "// your network password";
int  keyIndex = 0; // your network key Index number (needed only for WEP)

WiFiClient client;
int status = WL_IDLE_STATUS;

void setup() {

  MC3672_acc.start();

  if (serialDebug) {
    Serial.begin(9600);
  }
  delay(10);

  if (serialDebug) {
    while (!Serial) {
      //Just wait for serial
    }
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to wifi");
  printWiFiStatus();

  //Try to connect to the server socket
  client.connect(ServerIPAddr, port);
  while (!client.connected()) {
    delay(100);
    if (serialDebug) {
      Serial.println("Waiting to connect");
    }
  }
  if (serialDebug) {
    Serial.write("Connected to server for first time");
  }
}

void loop() {
  if (!client.connected()) {
    client.connect(ServerIPAddr, port);
    delay(1000);
  }
  //  while(!client.connected()) {
  //    delay(100);
  //    if(serialDebug) {
  //       Serial.println("Waiting to connect to server");
  //    }
  //  }
  //


  if (client.connected()) {
    if (serialDebug) {
      Serial.write("Connected to server:");
    }
    MC3672_acc_t rawAccel = MC3672_acc.readRawAccel();
    delay(10);
    // Display the results (acceleration is measured in m/s^2). They are text also
    if (serialDebug) {
      Serial.print("X"); Serial.print(rawAccel.XAxis_g);
      Serial.print("Y"); Serial.print(rawAccel.YAxis_g);
      Serial.print("Z"); Serial.print(rawAccel.ZAxis_g);
      Serial.println();
    }
    client.write(STX);

    client.print(String(rawAccel.XAxis_g) + NUL );

    client.print(String(rawAccel.YAxis_g) + NUL);

    client.print(String(rawAccel.ZAxis_g));
    client.write(ETX);
    delay(Ts - 10);
    //Close client. If you like to close after sending each sample
    //client.stop(); //We use this if we want to disconnect after each sampling period
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
