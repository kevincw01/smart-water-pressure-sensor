/****************************************************************************************************************************************************
 *  DESCRIPTION:monitors water pressure from nano every connected to 5v pressure transducer and sends to mqtt 
 *
 *  Author: Kevin Williams
 ****************************************************************************************************************************************************/

//#define DEBUG //turn on debug statements
//#define SIM_RX //simulate other microcontroller and pressure transducer w rand pressures

#ifdef DEBUG
  #define DEBUG_PRINTLN(x)  Serial.println (x)
  #define DEBUG_PRINT(x)  Serial.print (x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

#include <WiFiNINA.h> 
#include <PubSubClient.h>
#include "credentials.h"
#include <stdlib.h>
#include <ArduinoJson.h>

#define LED_PIN   LED_BUILTIN

const char* ssid = networkSSID;
const char* password = networkPASSWORD;
const char* mqttServer = mqttSERVER;
const char* mqttUsername = mqttUSERNAME;
const char* mqttPassword = mqttPASSWORD;
const char* pubTopicPost = pubTopicPOST;

char pubTopic[256];       //prefix 'arduino/<clientID>'; payload[0] will have pressure value

const byte numChars = 32;
char receivedChars[numChars];

boolean newData = false;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
long lastMsg = 0;
long lastPublish = 0;
char msg[50];
int value = 0;
int last_value = -10;
byte mac[6];
char MAC_addr[24];
int sensorValue = -1;

uint16_t calcCRC(char* stringData)
{
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
    
  for (int i = 0; i < strlen(stringData); i++)
  {
    sum1 = (sum1 + stringData[i]) % 255;
    sum2 = (sum2 + sum1) % 255;
  }
   return (sum2 << 8) | sum1;
}

bool checkChecksum(char* stringData, char* strArray[]) {
  bool success = false;
  char *str;
  int count = 0;
  
  if(strlen(stringData) < 5) {
    Serial.print("somthings wrong, skipping.  strlen=");
    Serial.println(strlen(stringData));  
  } else {
    while ((str = strtok_r(stringData, ",", &stringData)) != NULL)
    {
      DEBUG_PRINTLN(str);
      strArray[count] = str;
      count++;
    }
  
    char str1[16];
    strcpy(str1, strArray[0]);
    strcat(str1, ",");
    strcat(str1, strArray[1]);
    char *checkResult = str1;
    DEBUG_PRINT("split off checksum: ");
    DEBUG_PRINTLN(checkResult);
    
    uint16_t checksum = calcCRC(checkResult);
    DEBUG_PRINT("checksum=");
    DEBUG_PRINTLN(checksum);
    if(checksum == atoi(strArray[2])) {
      success=true;
      DEBUG_PRINTLN("checksum passed");
    } else
      Serial.println("checksum failed");
  }
  
  return success;
}

void setup_wifi() 
{
  delay(10);
  long last = 0;
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    long now = millis();
    if (now - last > 10000) 
    {
      last = now;
      Serial.println("Wifi not connecting....trying to reset");
    }
      //show status externally via led (fast blink means wifi no connected)
      digitalWrite(LED_PIN, HIGH);
      delay(150);
      Serial.print(".");
      digitalWrite(LED_PIN, LOW);
      delay(150);
      NVIC_SystemReset(); 
    
  }
  digitalWrite(LED_PIN, LOW);  //ensure led is off after wifi connect

  randomSeed(micros());

  WiFi.macAddress(mac);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  sprintf(MAC_addr,"%02X:%02X:%02X:%02X:%02X:%02X",mac[5],mac[4],mac[3],mac[2],mac[1],mac[0]);
  Serial.println(MAC_addr);
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    char clientId[25] = "Arduino-";
    char prefix[] = "arduino/";
    strcat(clientId,MAC_addr);
    strcpy(pubTopic,prefix);
    strcat(pubTopic,clientId);
    strcat(pubTopic,pubTopicPost);
    
    // Attempt to connect
    if (client.connect(clientId, mqttUsername, mqttPassword))
    {
      Serial.print("connected w clientId=");
      Serial.println(clientId);
      // ... and resubscribe
      //client.subscribe(subTopic);
    } else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      //show status externally via led (slow blink means mqtt not connected)
      for(int i=0;i<5;i++) {  
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(500);                       // wait for a second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(500);
      }
    }
  }
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial1.available() > 0 && newData == false) {
        rc = Serial1.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewData() {
    if (newData == true) {
        DEBUG_PRINT("This just in ... ");
        DEBUG_PRINTLN(receivedChars);
        newData = false;
    }
}

void setup() 
{
  pinMode(LED_PIN, OUTPUT);     
  Serial.begin(115200);
  Serial1.begin(9600); 
  setup_wifi();
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void publish_mqtt(int v, char* v_s, int l) {
  
  Serial.println(v_s);
  Serial.print("on topic: ");
  Serial.print(pubTopic);
  Serial.print(" at time= ");
  Serial.println(l);

  StaticJsonDocument<200> doc;
  char jsonStr[128];

  doc["pressure"] = v_s;
  doc["wifi"] = WiFi.RSSI();

  serializeJson(doc, jsonStr);
  client.publish(pubTopic,jsonStr); 
}

void loop() 
{
  char value_s[128];
  
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) 
  {
    lastMsg = now;

    #ifdef SIM_RX
      newData = true; //debug
    #else
      recvWithStartEndMarkers();
    #endif
            
    if (newData == true) {
      DEBUG_PRINTLN(receivedChars);
      newData = false;

      char* b = receivedChars;

      char* strArray[3];
      
      #ifdef SIM_RX
        if(1) {
          value = rand() % 100 ; 
      #else
        if(checkChecksum(b, strArray)) {
          //checksum passed for data rx from serial board
          value = atoi(strArray[1]);
      #endif   
        
        itoa(value,value_s,10);
    
        DEBUG_PRINT("rx pressure: ");
        DEBUG_PRINTLN(value_s);

        //only publish every 5 minutes or if pressure has changed by 2 psi or more
        if(abs(last_value - value) > 1) {           
          //pressure differnt by 2+ psi, publish
          lastPublish = now;  //record last time we published
          
          publish_mqtt(value,value_s,lastPublish); 
        } else {
          //publsh even if not new every 5 minutes
          if(now - lastPublish > 300000) {
            //unique value read, publish
            lastPublish = now;  //record last time we published

            Serial.print("publishing: ");
            publish_mqtt(value,value_s,lastPublish); 
          } else {
            Serial.println("not publishing bc not 2+psi different");
          }
        }
        last_value = value;
      }else {
        Serial.println("checksum failed, discarding");
      }

      memset(receivedChars, 0, 32);
    } else
      Serial.println("Error: pressure transducer microcontroller not functioning");
  }
}
