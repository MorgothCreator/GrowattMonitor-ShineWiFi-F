
// Generic ESP8266 Module
// arduFPGA-app-common-arduino by Iulian Gheorghiu
// Rtc_Pcf8563 by Joe Robertson
// NTPClient by Fabrice Weinberg

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "def.h"
#if USE_HTTPS
#include <ESP8266WebServerSecure.h>
#include "crt.h"
#else
#include <ESP8266WebServer.h>
#endif
#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <FS.h>

#include <arduFPGA-app-common-arduino.h>
#include <Rtc_Pcf8563.h>
#include <NTPClient.h>

#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

#include "mainWebPage.h"
#include "pv.h"
#include "dataLog.h"
#include "report.h"

const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0
int analogValue = 0;
bool buttonState = false;
sTimer analogScanTimer(100);
bool wifiConnectStatusState = false;
int wiFiCredentialsCnt = 0;
int wiFiConnectRetryCnt = CONNECT_RETRY_COUNT;
sTimer wifiConnectingTimer(500);
int _progress = 0;

const char* host = OTA_NAME;
const char* otaPass = OTA_PASS;
wiFiCredentials_t wiFiCredentials[] = WIFI_CREDENTIALS;

#if USE_HTTPS
BearSSL::ESP8266WebServerSecure server(443);
BearSSL::ServerSessions serverCache(5);
#else
ESP8266WebServer server(80);
#endif
Rtc_Pcf8563 rtc;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};



void printTime() {
  reportPrintF("%02d:%02d:%02d %02d/%02d/20%02d %s",
  rtc.getHour(), rtc.getMinute(), rtc.getSecond(),
  rtc.getDay(), rtc.getMonth(), rtc.getYear(), 
  weekDays[rtc.getWeekday()].c_str());
  reportAppendLn("");
}

File updateLogFile;

void handleRoot() {
  digitalWrite(LED1_BLUE, 1);
  char *page = MainWebPage_ComposeHtml();
  if(page) {
    server.send(200, "text/html", page);
    delete [] page;
  } else {
    reportAppendLn("Unable to allocate");
  }
  digitalWrite(LED1_BLUE, 0);
}

void handleReport() {
  digitalWrite(LED1_BLUE, 1);
  String reportPage = "<!DOCTYPE HTML>\r\n<html>\r\n\r\n";
  reportPage += getReport();
  reportPage += "</html>\n";
  server.send(200, "text/html", reportPage);
  digitalWrite(LED1_BLUE, 0);
}

void handleNotFound(){
  digitalWrite(LED1_BLUE, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED1_BLUE, 0);
}

void setup(void){
  Serial.begin(9600);
  Wire.begin(5, 4);
  //Wire.pins(5, 4);
  //Serial1.begin(9600);
  pinMode(LED1_BLUE, OUTPUT);
  digitalWrite(LED1_BLUE, 0);
  pinMode(LED5_RED, OUTPUT);
  digitalWrite(LED5_RED, 0);
  pinMode(LED6_GREEN, OUTPUT);
  digitalWrite(LED6_GREEN, 0);
/*********************************************/
  reportAppendLn("");
  reportAppendLn("*** RTC PCF8563 ***");
/*********************************************/
#if 0
  rtc.initClock();
  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setDate(8, 6, 10, 1, 22);
  //hr, min, sec
  rtc.setTime(20, 43, 0);
#endif
  printTime();
/*********************************************/
#if USE_INTERNAL_SPIFFS
  SPIFFS.begin();
#endif
/*********************************************/
  PV_Init();
  Log_Init();
  PV_triggerDataRead();
  wifiConnectingTimer.Start();
}

void loop(void){
  PV_Loop();
  Log_Loop("pvlog/", &rtc);
/*********************************************/
  if(analogScanTimer.Tick()) {
    analogValue = analogRead(analogInPin);
    if(analogValue < 200 && buttonState == false) {
      buttonState = true;
      reportAppendLn("Button pressed.");
    } else if(analogValue > 400 && buttonState == true) {
      buttonState = false;
      reportAppendLn("Button released.");
    }
  }
/*********************************************/
  switch (ScetchWiFiState) {
    case DISCONNECTED:
      digitalWrite(LED6_GREEN, 0);
      reportAppendLn("");
      reportAppendLn("");
      reportAppend("Connecting to ");
      reportAppendLn(wiFiCredentials[wiFiCredentialsCnt].ssid);
      WiFi.begin(wiFiCredentials[wiFiCredentialsCnt].ssid, wiFiCredentials[wiFiCredentialsCnt].password);
      ScetchWiFiState = CONNECTING;
      wiFiCredentialsCnt++;
      if(wiFiCredentials[wiFiCredentialsCnt].ssid[0] == 0) {
        wiFiCredentialsCnt = 0;
      }
      wiFiConnectRetryCnt = CONNECT_RETRY_COUNT;
      break;
    case CONNECTING:
      if(wifiConnectingTimer.Tick()) {
        wifiConnectStatusState = !wifiConnectStatusState;
        digitalWrite(LED6_GREEN, wifiConnectStatusState);
        reportAppend(".");
        if(!wiFiConnectRetryCnt--) {
          ScetchWiFiState = DISCONNECTED;
        }
      }
      if (WiFi.status() == WL_CONNECTED) {
        ScetchWiFiState = CONNECTED;
        digitalWrite(LED6_GREEN, 1);
        reportAppendLn("");
        reportAppendLn("WiFi connected");
        // Print the IP address
        reportAppendLn((char *)WiFi.localIP().toString().c_str());
        delay(100);
        if (MDNS.begin(MDNS_NAME)) {
          reportAppendLn("MDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);        
        }
#if USE_HTTPS
        configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
        // Cache SSL sessions to accelerate the TLS handshake.
        server.getServer().setCache(&serverCache);
#endif
        server.on(F("/"), handleRoot);
        server.on(F("/report"), handleReport);
#if USE_INTERNAL_SPIFFS
        server.on(F("/pvlog"), []() {
          if(server.argName(0).equals("ls")) {
            sendLogsName(&server);
          } else if(server.argName(0).equals("get")) {
            decodeSendLogData(&server);
          } else if(server.argName(0).equals("len")) {
            sendLogLen(&server);
          } else {
            server.send(200, "text/plain", "Unknown command");            
          }
        });
        server.on(UriBraces("/ls/{}"), []() {
          String user = server.pathArg(0);
          server.send(200, "text/plain", "ls: '" + user + "'");
        });
#endif

        /*server.on(UriRegex("^\\/users\\/([0-9]+)\\/devices\\/([0-9]+)$"), []() {
          String user = server.pathArg(0);
          String device = server.pathArg(1);
          server.send(200, "text/plain", "User: '" + user + "' and Device: '" + device + "'");
        });*/
        server.on(F("/favicon.ico"), []() {
          static const uint8_t gif[] PROGMEM = {
            0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80, 0x01,
            0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
            0x10, 0x00, 0x10, 0x00, 0x00, 0x02, 0x19, 0x8c, 0x8f, 0xa9, 0xcb, 0x9d,
            0x00, 0x5f, 0x74, 0xb4, 0x56, 0xb0, 0xb0, 0xd2, 0xf2, 0x35, 0x1e, 0x4c,
            0x0c, 0x24, 0x5a, 0xe6, 0x89, 0xa6, 0x4d, 0x01, 0x00, 0x3b
          };
          char gif_colored[sizeof(gif)];
          memcpy_P(gif_colored, gif, sizeof(gif));
          // Set the background to a random set of colors
          gif_colored[16] = millis() % 256;
          gif_colored[17] = millis() % 256;
          gif_colored[18] = millis() % 256;
          server.send(200, "image/gif", gif_colored, sizeof(gif_colored));
        });
        server.onNotFound(handleNotFound);
        server.begin();
        // Port defaults to 8266
        // ArduinoOTA.setPort(8266);

        // Hostname defaults to esp8266-[ChipID]
        ArduinoOTA.setHostname(host);

        // No authentication by default
        // ArduinoOTA.setPassword("admin");

        // Password can be set with it's md5 value as well
        // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
        // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
        ArduinoOTA.setPassword(otaPass);

        ArduinoOTA.onStart([]() {
          String type;
          if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
          else // U_SPIFFS
            type = "filesystem";
          // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
          reportAppend("Start updating ");
          reportAppendLn((char *)type.c_str());
#if USE_SPIFFS_LOG && USE_INTERNAL_SPIFFS
          updateLogFile = SPIFFS.open("updatelog.txt", "w");
          updateLogFile.print("Start updating\n");
#endif
          _progress = -1;
        });
        ArduinoOTA.onEnd([]() {
          reportAppendLn("\nEnd");
#if USE_SPIFFS_LOG && USE_INTERNAL_SPIFFS
          updateLogFile.print("\nEnd");
          updateLogFile.close();
#endif
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
          if(_progress != (progress / (total / 100))) {
            _progress = (progress / (total / 100));
            reportPrintF("Progress: %u%%\r", (progress / (total / 100)));
#if USE_SPIFFS_LOG && USE_INTERNAL_SPIFFS
            updateLogFile.printf("Progress: %u%%\n", (progress / (total / 100)));
#endif
          }
        });
        ArduinoOTA.onError([](ota_error_t error) {
          reportPrintF("Error[%u]: ", (int)error);
          if (error == OTA_AUTH_ERROR) reportAppendLn("Auth Failed");
          else if (error == OTA_BEGIN_ERROR) reportAppendLn("Begin Failed");
          else if (error == OTA_CONNECT_ERROR) reportAppendLn("Connect Failed");
          else if (error == OTA_RECEIVE_ERROR) reportAppendLn("Receive Failed");
          else if (error == OTA_END_ERROR) reportAppendLn("End Failed");
        });
        ArduinoOTA.begin();
        reportAppendLn("Server started");
        // Initialize a NTPClient to get time
        timeClient.begin();
        // Set offset time in seconds to adjust for your timezone, for example:
        // GMT +1 = 3600
        // GMT +8 = 28800
        // GMT -1 = -3600
        // GMT 0 = 0
        timeClient.setTimeOffset(3*3600);
        if(timeClient.update()) {
          time_t epochTime = timeClient.getEpochTime();
          struct tm *ptm = gmtime ((time_t *)&epochTime);
          rtc.initClock();
          //day, weekday, month, century(1=1900, 0=2000), year(0-99)
          rtc.setDate(ptm->tm_mday, ptm->tm_wday, ptm->tm_mon+1, 1, ptm->tm_year - 100);
          //hr, min, sec
          rtc.setTime(timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());
          reportAppendLn("RTC updated");
          printTime();
        }
      }
      break;
    case CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        reportAppendLn("WiFi disconnected");
        ScetchWiFiState = DISCONNECTED;
      } else {
        ArduinoOTA.handle();
        server.handleClient();
        MDNS.update();
      }
      break;
  }
}
