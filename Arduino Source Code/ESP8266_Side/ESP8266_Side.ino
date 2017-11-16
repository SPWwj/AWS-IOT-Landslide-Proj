/**************** Library include directives ****************/

#include <Arduino.h>
#include <Stream.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266TrueRandom.h>

//AWS
#include "sha256.h"
#include "Utils.h"
#include "AWSClient2.h"

//WEBSockets
#include <Hash.h>
#include <WebSocketsClient.h>

//MQTT PAHO
#include <SPI.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

//AWS MQTT Websocket
#include "Client.h"
#include "AWSWebSocketClient.h"
#include "CircularByteBuffer.h"

/**************** Constants declarations ****************/
// Please change the parameters below to match your local configuration
// The variables below are for reference only
char wifi_ssid[]       = "Your Wifi SSID";
char wifi_password[]   = "Your Wifi Password";
char aws_endpoint[]    = "AWS ENDPOINT";
char aws_key[]         = "AWS KEY";
char aws_secret[]      = "AWS SECRET";
char aws_region[]      = "REGION";
const char* aws_topic  = "Your Topic"//"$aws/things/MakeSense1/shadow/update";
int port = 443;

#define DEBUG_PRINT 0

const int maxMQTTpackageSize = 512;
const int maxMQTTMessageHandlers = 1;

/**************** Variable declarations ****************/
ESP8266WiFiMulti WiFiMulti;

AWSWebSocketClient awsWSclient(1000);

IPStack ipstack(awsWSclient);
MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers> *client = NULL;

long connection = 0;

/**************** Helper function declarations ****************/

//generate random mqtt clientID
char* generateClientID() {
  char* cID = new char[23]();
  for (int i=0; i<22; i+=1)
  cID[i]=(char)random(1, 256);
  return cID;
}

//count messages arrived
int arrivedcount = 0;

//callback to handle mqtt messages
void messageArrived(MQTT::MessageData& md) {
  MQTT::Message &message = md.message;

  if (DEBUG_PRINT) {
    Serial.print("Message ");
    Serial.print(++arrivedcount);
    Serial.print(" arrived: qos ");
    Serial.print(message.qos);
    Serial.print(", retained ");
    Serial.print(message.retained);
    Serial.print(", dup ");
    Serial.print(message.dup);
    Serial.print(", packetid ");
    Serial.println(message.id);
    Serial.print("Payload ");
    char* msg = new char[message.payloadlen+1]();
    memcpy (msg,message.payload,message.payloadlen);
    Serial.println(msg);
    delete msg;
  }
}

//connects to websocket layer and mqtt layer
bool connect() {
  if (client == NULL) {
    client = new MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers>(ipstack);
  } else {

    if (client->isConnected()) {
      client->disconnect();
    }
    delete client;
    client = new MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers>(ipstack);
  }

  //delay is not necessary... it just help us to get a "trustful" heap space value
  delay (1000);
  if (DEBUG_PRINT) {
    Serial.print (millis());
    Serial.print (" - conn: ");
    Serial.print (++connection);
    Serial.print (" - (");
    Serial.print (ESP.getFreeHeap ());
    Serial.println (")");
  }

  int rc = ipstack.connect(aws_endpoint, port);
  if (rc != 1)
  {
    if (DEBUG_PRINT) {
      Serial.println("error connection to the websocket server");
    }
    return false;
  } else {
    if (DEBUG_PRINT) {
      Serial.println("websocket layer connected");
    }
  }

  if (DEBUG_PRINT) {
    Serial.println("MQTT connecting");
  }

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  char* clientID = generateClientID();
  data.clientID.cstring = clientID;
  rc = client->connect(data);
  delete[] clientID;
  if (rc != 0)
  {
    if (DEBUG_PRINT) {
      Serial.print("error connection to MQTT server");
      Serial.println(rc);
      return false;
    }
  }
  if (DEBUG_PRINT) {
    Serial.println("MQTT connected");
  }
  return true;
}

//subscribe to a mqtt topic
void subscribe() {
  //subscribe to a topic
  int rc = client->subscribe(aws_topic, MQTT::QOS0, messageArrived);
  if (rc != 0) {
    if (DEBUG_PRINT) {
      Serial.print("rc from MQTT subscribe is ");
      Serial.println(rc);
    }
    return;
  }
  if (DEBUG_PRINT) {
    Serial.println("MQTT subscribed");
  }
}

void waitForWifi() {
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
    if (DEBUG_PRINT) {
      Serial.println("waiting for wifi");
    }
  }
  if (DEBUG_PRINT) {
    Serial.println("\nconnected to network " + String(wifi_ssid) + "\n");
  }
}

String getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

/**************** Main Program ****************/

void setup() {
  Serial.begin(9600);
  WiFiMulti.addAP(wifi_ssid, wifi_password);

  waitForWifi();

  awsWSclient.setAWSRegion(aws_region);
  awsWSclient.setAWSDomain(aws_endpoint);
  awsWSclient.setAWSKeyID(aws_key);
  awsWSclient.setAWSSecretKey(aws_secret);
  awsWSclient.setUseSSL(true);
}

void loop() {
  // Perform reading from Arduino side
  // ESP8266 sends a single '.' character (ASCII code 0x2E)
  // Arduino returns CSV style with LF line ending
  // Format: Soil[0,1023],Rain[0,1023],Accl_X[-255,255],Accl_Y[-255,255],Accl_Z[-255,255]
  unsigned long timeout = millis() + 2000UL; // 2 second timeout
  Serial.print(".");
  String inData = "";
  while (1) {
    if (millis() > timeout) {
      break;
    }
    while (Serial.available()) {
      char received = Serial.read();
      if (received == '\n') {
        goto response;
      }
      inData += received;
    }
  }

response:
  bool sensorError = false;
  if (inData.length() < 1) {
    if (DEBUG_PRINT) {
      Serial.println("Error reading device sensor");
    }
    sensorError = true;
  }

  // Split up response
  String soil = getValue(inData, ',', 0);
  String rain = getValue(inData, ',', 1);
  String accl_x = getValue(inData, ',', 2);
  String accl_y = getValue(inData, ',', 3);
  String accl_z = getValue(inData, ',', 4);
  String gyro_x = getValue(inData, ',', 5);
  String gyro_y = getValue(inData, ',', 6);
  String gyro_z = getValue(inData, ',', 7);

  if (sensorError) {
    soil = "null";
    rain = "null";
    accl_x = "null";
    accl_y = "null";
    accl_z = "null";
    gyro_x = "null";
    gyro_y = "null";
    gyro_z = "null";
  }

  // Construct message to send to AWS IoT
  String values = "{\"state\":{\"reported\":{}},\"entryUUID\":\"";
  uint8_t uuid[16];
  ESP8266TrueRandom.uuid(uuid);
  values += ESP8266TrueRandom.uuidToString(uuid);
  values += "\",\"deviceID\":0";
  if (sensorError) {
    values += ",\"status\":-1";
  } else {
    values += ",\"status\":0";
    values += ",\"soil\":"+ soil;
    values += ",\"rain\":"+ rain;
    values += ",\"accl_x\":"+ accl_x;
    values += ",\"accl_y\":"+ accl_y;
    values += ",\"accl_z\":"+ accl_z;
    values += ",\"gyro_x\":"+ gyro_x;
    values += ",\"gyro_y\":"+ gyro_y;
    values += ",\"gyro_z\":"+ gyro_z;
  }
  values += "}";
  const char *publish_message = values.c_str();

  // Connect (if not yet connected) and send message
  if (awsWSclient.connected()) {
    client->yield();
    subscribe();
    //publish
    MQTT::Message message;
    char buf[1000];
    strcpy(buf, publish_message);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;
    int rc = client->publish(aws_topic, message);
  } else {
    //handle reconnection
    connect();
  }

  // Send next message 5 seconds later
  delay(1000);
}
