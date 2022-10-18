#ifndef __DATA_LOG_H__
#define __DATA_LOG_H__

#include "def.h"
#if USE_HTTPS
#include <ESP8266WebServerSecure.h>
#else
#include <ESP8266WebServer.h>
#endif
#include <Rtc_Pcf8563.h>

#define  LOG_JSON \
"{\n" \
"\"TimeStamp\":\"%04d/%02d/%02d %02d-%02d-%02d\",\n" \
"\"Grid_voltage\":%.2f,\n" \
"\"Grid_frequency\":%.2f,\n" \
"\"Grid_input_power\":%d,\n" \
"\"Grid_charge_current\":%.2f,\n" \
"\"PV_Voltage\":%.2f,\n" \
"\"PV_Charge_current\": %.2f,\n" \
"\"PV_Power\":%d,\n" \
"\"Battery_voltage\":%.2f,\n" \
"\"Battery_capacity\":%d,\n" \
"\"Battery_charge_current\":%.2f,\n" \
"\"Battery_discharge_current\":%.2f,\n" \
"\"Output_voltage\":%.2f,\n" \
"\"Output_frequency\":%.2f,\n" \
"\"Output_active_power\":%d,\n" \
"\"Output_aparent_power\":%d,\n" \
"\"Load_percentage\":%.2f,\n" \
"\"Grid_discharge_today\":%.2f,\n" \
"\"Grid_discharge_total\":%.2f,\n" \
"\"PV_Production_today\":%.2f,\n" \
"\"PV_Production_total\":%.2f,\n" \
"\"Battery_discharge_today\":%.2f,\n" \
"\"Battery_discharge_total\":%.2f,\n" \
"\"Inverter_temperature\":%.2f,\n" \
"\"DC-DC_temperature\":%.2f,\n" \
"\"Error\":\"%s\",\n" \
"\"Warning\":\"%s\",\n" \
"\"State\":\"%s\",\n" \
"\"Charge_priority\":\"%s\",\n" \
"\"Max_charge_current\":%d,\n" \
"\"AC_Charge_current\":%d\n" \
"}"

void Log_Init();
void Log_Loop(char *LogPath, Rtc_Pcf8563 *rtc);
void Log_ComposeLogJson(char *buffer, int bufferSize);
#if USE_HTTPS
void sendLogsName(BearSSL::ESP8266WebServerSecure *server);
void sendLogLen(BearSSL::ESP8266WebServerSecure *server);
void decodeSendLogData(BearSSL::ESP8266WebServerSecure *server);
#else
void sendLogsName(ESP8266WebServer *server);
void sendLogLen(ESP8266WebServer *server);
void decodeSendLogData(ESP8266WebServer *server);
#endif

#endif