/*--------------------------------------------------
HTTP 1.1 Webserver as AccessPoint for ESP8266 
for ESP8266 adapted Arduino IDE

by Stefan Thesen 08/2015 - free for anyone

Does HTTP 1.1 with defined connection closing.
Handles empty requests in a defined manner.
Handle requests for non-exisiting pages correctly.

This demo allows to switch two functions:
Function 1 creates serial output and toggels GPIO2
Function 2 just creates serial output.

Serial output can e.g. be used to steer an attached
Arduino, Raspberry etc.
--------------------------------------------------*/
/////////////////////
// Pin Definitions //
/////////////////////
//static const uint8_t D0   = 16;
//static const uint8_t D1   = 5;
//static const uint8_t D2   = 4;
//static const uint8_t D3   = 0;
//static const uint8_t D4   = 2;
//static const uint8_t D5   = 14;
//static const uint8_t D6   = 12;
//static const uint8_t D7   = 13;
//static const uint8_t D8   = 15;
//static const uint8_t D9   = 3;
//static const uint8_t D10  = 1;

#include <ESP8266WiFi.h>

const char* ssid = "Robot";
const char* password = "sebastien";  // set to "" for open access point w/o passwortd

int led = 16;

// Create an instance of the server on Port 80
WiFiServer server(80);

int motor1_enablePin = 0; //D3 //pwm
int motor1_in1Pin = 5; //D1
int motor1_in2Pin = 4; //D2
 
int motor2_enablePin = 2; // D4 //pwm
int motor2_in1Pin = 14; //D5
int motor2_in2Pin = 12; // D6


void setup() 
{

  //init engine 1
  pinMode(motor1_in1Pin, OUTPUT);
  pinMode(motor1_in2Pin, OUTPUT);
  pinMode(motor1_enablePin, OUTPUT);
 
  //init engine 2
  pinMode(motor2_in1Pin, OUTPUT);
  pinMode(motor2_in2Pin, OUTPUT);
  pinMode(motor2_enablePin, OUTPUT);
  
  // prepare GPIO2
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  
  // start serial
  Serial.begin(9600);
  delay(1);
  
  // AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.begin();
}

void loop() 
{ 
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    Serial.println("client connection time-out!");
    return; 
  }
  
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
    }
  }
  
  
  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sHeader;
  
  ////////////////////////////
  // 404 for non-matching path
  ////////////////////////////
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
    
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  ///////////////////////
  // format the html page
  ///////////////////////
  else
  {
    sResponse  = "<html><head><title>Robot</title></head><body>";
    sResponse += "<font color=\"#000000\"><body bgcolor=\"#d0d0f0\">";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>Robot</h1>";
    sResponse += "<FONT SIZE=+1>";
    sResponse += "<p> LED : <a href=\"?pin=on\"><button>Allumer</button></a>&nbsp;<a href=\"?pin=off\"><button>Eteindre</button></a></p>";
    sResponse += "<p> <a href=\"?pin=avancer\"><button>Avancer</button></a>&nbsp;<a href=\"?pin=reculer\"><button>Reculer</button></a></p>";
    sResponse += "<p> <a href=\"?pin=gauche\"><button>Gauche</button></a>&nbsp;<a href=\"?pin=droite\"><button>Droite</button></a></p>";
    sResponse += "<p> <a href=\"?pin=arret\"><button>Arret</button></a>&nbsp;</p>";

    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length()>0)
    {
      // write received command to html page
      sResponse += "commande :" + sCmd + "<BR>";
      
      // switch GPIO
      if(sCmd.indexOf("avancer")>=0)
      {
        avancer();
      }
      else if(sCmd.indexOf("reculer")>=0)
      {
        reculer();
      }
       else if(sCmd.indexOf("gauche")>=0)
      {
        gauche();
      }
       else if(sCmd.indexOf("droite")>=0)
      {
        droite();
      }
      else if(sCmd.indexOf("arret")>=0)
      {
        arret();
      }

      // switch GPIO
      if(sCmd.indexOf("on")>=0)
      {
        digitalWrite(led,HIGH);
      }
      else if(sCmd.indexOf("off")>=0)
      {
         digitalWrite(led,LOW);
      }
      
    }
    
    sResponse += "<FONT SIZE=-2>";
    sResponse += "</body></html>";
    
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  
  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);
  
  // and stop the client
  client.stop();
  Serial.println("Client disonnected");
}

void avancer()
{
  SetMotor2(1024, false);
  SetMotor1(1024, false);
}
void reculer()
{
  SetMotor2(1024, true);
  SetMotor1(1024, true);
}
void gauche()
{
  SetMotor2(650, false);
  SetMotor1(650, true);
}
void droite()
{
   SetMotor2(650, true);
   SetMotor1(650, false);
}
void arret()
{
   SetMotor2(0, true);
   SetMotor1(0, false);
}

//Fonction qui set le moteur1
void SetMotor1(int speed, boolean reverse)
{
  analogWrite(motor1_enablePin, speed);
  digitalWrite(motor1_in1Pin, ! reverse);
  digitalWrite(motor1_in2Pin, reverse);
}
 
//Fonction qui set le moteur2
void SetMotor2(int speed, boolean reverse)
{
  analogWrite(motor2_enablePin, speed);
  digitalWrite(motor2_in1Pin, ! reverse);
  digitalWrite(motor2_in2Pin, reverse);
}
