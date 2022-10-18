#include "util/util.h"
#include <cstdio>
#include <sys/pgmspace.h>
#include <sys/stdio.h>
#include <sys/_stdint.h>
#include <vector>
#include <algorithm>

#include "dataLog.h"

#include <string.h>
#include <math.h>

#include <FS.h>

#include <arduFPGA-app-common-arduino.h>
#include "pv.h"

long refreshCnt = 0;
int sampleCount = 0;
sTimer tLogAppend;
char buffer[8192];
char lastFilePath[128] = "";
char lastFilePathWww[128] = "";

char LogJson[] = LOG_JSON;

double Log_Grid_voltage=0;
double Log_Grid_frequency=0;
int Log_Grid_input_power=0;
double Log_Grid_charge_current=0;
double Log_PV_Voltage=0;
double Log_PV_Charge_current=0;
int Log_PV_Power=0;
double Log_Battery_voltage=0;
int Log_Battery_capacity=0;
double Log_Battery_charge_current=0;
double Log_Battery_discharge_current=0;
double Log_Output_voltage=0;
double Log_Output_frequency=0;
int Log_Output_active_power=0;
int Log_Output_aparent_power=0;
double Log_Load_percentage=0;
double Log_Grid_discharge_today=0;
double Log_Grid_discharge_total=0;
double Log_PV_Production_today=0;
double Log_PV_Production_total=0;
double Log_Battery_discharge_today=0;
double Log_Battery_discharge_total=0;
double Log_Inverter_temperature=0;
double Log_DC_DC_temperature=0;
char Log_Error[257]="";
char Log_Warning[257]="";
char Log_State[64]="";
char Log_Charge_priority[32]="";
int Log_Grid_rated_voltage=0;
int Log_Grid_rated_frequency=0;
int Log_Rated_battery_voltage=0;
int Log_Max_charge_current=0;
int Log_Rated_Output_voltage=0;
int Log_Rated_Output_frequency=0;
double Log_Rated_Output_power=0;
int Log_AC_Charge_current=0;

uint32_t _Log_Error=0;
uint32_t _Log_Warning=0;
uint16_t _Log_State=0;
uint8_t _Log_Charge_priority=0;


typedef struct {
  uint8_t version;
  uint8_t sec;
  uint8_t min;
  uint8_t hour;
  uint16_t Log_Grid_voltage;//X10
  uint16_t Log_Grid_frequency;//X100
  uint16_t Log_Grid_input_power;
  uint16_t Log_Grid_charge_current;//X100
  uint16_t Log_PV_Voltage;//X10
  uint16_t Log_PV_Charge_current;//X100
  uint16_t Log_PV_Power;
  uint16_t Log_Battery_voltage;//X100
  uint8_t Log_Battery_capacity;
  int16_t Log_Battery_charge_current;//X100
  uint16_t Log_Output_voltage;//X100
  uint16_t Log_Output_frequency;//X100
  uint16_t Log_Output_active_power;
  uint16_t Log_Output_aparent_power;
  uint16_t Log_Load_percentage;//X10
  uint16_t Log_Grid_discharge_today;//X10
  uint16_t Log_Grid_discharge_total;//X10
  uint16_t Log_PV_Production_today;//X10
  uint16_t Log_PV_Production_total;//X10
  uint16_t Log_Battery_discharge_today;//X10
  uint16_t Log_Battery_discharge_total;//X10
  uint16_t Log_Inverter_temperature;//X10
  uint16_t Log_DC_DC_temperature;//X10
  uint32_t Log_Error;
  uint32_t Log_Warning;
  uint16_t Log_State;
  uint8_t Log_Charge_priority;
  uint16_t Log_Max_charge_current;
  uint8_t Log_Rated_Output_voltage;
  uint8_t Log_AC_Charge_current;
}log_v1_t;


void Log_Init() {
	sampleCount = 0;
	tLogAppend.Start(300000);
}

void Log_Loop(char *LogPath, Rtc_Pcf8563 *rtc) {
	if(refreshCnt != PV_getDataFefreshCnt()) {
		refreshCnt = PV_getDataFefreshCnt();
		
		Log_Grid_voltage += Grid_voltage;
		Log_Grid_frequency += Grid_frequency;
		Log_Grid_input_power += Grid_input_power;
		Log_Grid_charge_current += Grid_charge_current;
		Log_PV_Voltage += PV_Voltage;
		Log_PV_Charge_current += PV_Charge_current;
		Log_PV_Power += PV_Power;
		Log_Battery_voltage += Battery_voltage;
		Log_Battery_capacity += Battery_capacity;
		Log_Battery_charge_current += Battery_charge_current;
		Log_Battery_discharge_current += Battery_discharge_current;
		Log_Output_voltage += Output_voltage;
		Log_Output_frequency += Output_frequency;
		Log_Output_active_power += Output_active_power;
		Log_Output_aparent_power += Output_aparent_power;
		Log_Load_percentage += Load_percentage;
		Log_Grid_discharge_today += Grid_discharge_today;
		Log_Grid_discharge_total += Grid_discharge_total;
		Log_PV_Production_today += PV_Production_today;
		Log_PV_Production_total += PV_Production_total;
		Log_Battery_discharge_today += Battery_discharge_today;
		Log_Battery_discharge_total += Battery_discharge_total;
		Log_Inverter_temperature += Inverter_temperature;
		Log_DC_DC_temperature += DC_DC_temperature;
    _Log_Error |= faultCode;
		if(!strlen(Error)) {
			strncpy(Log_Error, Error, sizeof(Log_Error));
		}
    _Log_Warning |= warningCode;
		if(!strlen(Warning)) {
			strncpy(Log_Warning, Warning, sizeof(Log_Warning));
		}
    _Log_State |= systemStatus;
		if(!strlen(State)) {
			strncpy(Log_State, State, sizeof(Log_State));
		}
    _Log_Charge_priority |= chargePriority;
		if(!strlen(Charge_priority)) {
			strncpy(Log_Charge_priority, Charge_priority, sizeof(Log_Charge_priority));
		}
		Log_Max_charge_current += Max_charge_current;
		Log_AC_Charge_current += AC_Charge_current;
		
		sampleCount++;
	}
	if(tLogAppend.Tick()) {
		//printf("%s\n", tmpBuff);
		time_t T = time(NULL);
		struct tm tm = *localtime(&T);
		char path[128];
		snprintf(path, sizeof(path), "%s20%02d_%02d_%02d.%s", LogPath,
			rtc->getYear(), rtc->getMonth(), rtc->getDay(), "bin");

		Log_Grid_voltage /= sampleCount;
		Log_Grid_frequency /= sampleCount;
		Log_Grid_input_power /= sampleCount;
		Log_Grid_charge_current /= sampleCount;
		Log_PV_Voltage /= sampleCount;
		Log_PV_Charge_current /= sampleCount;
		if(isnan(Log_PV_Charge_current))
			//Log_PV_Charge_current /= sampleCount;
		//else
			Log_PV_Charge_current = 0;
		Log_PV_Power /= sampleCount;
		Log_Battery_voltage /= sampleCount;
		Log_Battery_capacity /= sampleCount;
		Log_Battery_charge_current /= sampleCount;
		Log_Battery_discharge_current /= sampleCount;
		Log_Output_voltage /= sampleCount;
		Log_Output_frequency /= sampleCount;
		Log_Output_active_power /= sampleCount;
		Log_Output_aparent_power /= sampleCount;
		Log_Load_percentage /= sampleCount;
		Log_Grid_discharge_today /= sampleCount;
		Log_Grid_discharge_total /= sampleCount;
		Log_PV_Production_today /= sampleCount;
		Log_PV_Production_total /= sampleCount;
		Log_Battery_discharge_today /= sampleCount;
		Log_Battery_discharge_total /= sampleCount;
		Log_Inverter_temperature /= sampleCount;
		Log_DC_DC_temperature /= sampleCount;
		Log_Max_charge_current /= sampleCount;
		Log_AC_Charge_current /= sampleCount;

    log_v1_t logEntry;
    logEntry.version = 1;
    logEntry.sec = rtc->getSecond();
    logEntry.min = rtc->getMinute();
    logEntry.hour = rtc->getHour();
    logEntry.Log_Grid_voltage = Log_Grid_voltage * 10;//X10
    logEntry.Log_Grid_frequency = Log_Grid_frequency * 100;//X100
    logEntry.Log_Grid_input_power = Log_Grid_input_power;
    logEntry.Log_Grid_charge_current = Log_Grid_charge_current * 100;//X100
    logEntry.Log_PV_Voltage = Log_PV_Voltage * 10;//X10
    logEntry.Log_PV_Charge_current = Log_PV_Charge_current * 100;//X100
    logEntry.Log_PV_Power = Log_PV_Power;
    logEntry.Log_Battery_voltage = Log_Battery_voltage * 100;//X100
    logEntry.Log_Battery_capacity = Log_Battery_capacity;
    logEntry.Log_Battery_charge_current = Log_Battery_charge_current * 100;//X100
    logEntry.Log_Output_voltage = Log_Output_voltage * 100;//X100
    logEntry.Log_Output_frequency = Log_Output_frequency * 100;//X100
    logEntry.Log_Output_active_power = Log_Output_active_power;
    logEntry.Log_Output_aparent_power = Log_Output_aparent_power;
    logEntry.Log_Load_percentage = Log_Load_percentage * 10;//X10
    logEntry.Log_Grid_discharge_today = Log_Grid_discharge_today * 10;//X10
    logEntry.Log_Grid_discharge_total = Log_Grid_discharge_total * 10;//X10
    logEntry.Log_PV_Production_today = Log_PV_Production_today * 10;//X10
    logEntry.Log_PV_Production_total = Log_PV_Production_total * 10;//X10
    logEntry.Log_Battery_discharge_today = Log_Battery_discharge_today * 10;//X10
    logEntry.Log_Battery_discharge_total = Log_Battery_discharge_total * 10;//X10
    logEntry.Log_Inverter_temperature = Log_Inverter_temperature * 10;//X10
    logEntry.Log_DC_DC_temperature = Log_DC_DC_temperature * 10;//X10
    logEntry.Log_Error = _Log_Error;
    logEntry.Log_Warning = _Log_Warning;
    logEntry.Log_State = _Log_State;
    logEntry.Log_Charge_priority = _Log_Charge_priority;
    logEntry.Log_Max_charge_current = Log_Max_charge_current;
    logEntry.Log_Rated_Output_voltage = Log_Rated_Output_voltage;
    logEntry.Log_AC_Charge_current = Log_AC_Charge_current;

		File pFile = SPIFFS.open(path, "a");
		if(pFile) {
      int wLen = pFile.write((char *)&logEntry, sizeof(logEntry));
      if(wLen != sizeof(logEntry)) {
		    pFile.close();
        Dir dir = SPIFFS.openDir("pvlog/");
        std::vector<string> filenames;
        while (dir.next()) {
          File entry = dir.openFile("r");
          if(entry) {
            string tmpStr;
            tmpStr.str = (char *)entry.name();
            tmpStr.size = strlen(entry.name());
            filenames.insert(filenames.end(), tmpStr);
          }
        }
        std::sort(filenames.begin(), filenames.end(), [](string a, string b) { return strcmp(a.str, b.str); });
        SPIFFS.remove(filenames[0].str);
        pFile = SPIFFS.open(path, "a");
        pFile.write((char *)&logEntry + wLen, sizeof(logEntry) - wLen);
      }
		  pFile.close();
		}
		strcpy(lastFilePath, path);

		Log_Grid_voltage = 0;
		Log_Grid_frequency = 0;
		Log_Grid_input_power = 0;
		Log_Grid_charge_current = 0;
		Log_PV_Voltage = 0;
		Log_PV_Charge_current = 0;
		Log_PV_Power = 0;
		Log_Battery_voltage = 0;
		Log_Battery_capacity = 0;
		Log_Battery_charge_current = 0;
		Log_Battery_discharge_current = 0;
		Log_Output_voltage = 0;
		Log_Output_frequency = 0;
		Log_Output_active_power = 0;
		Log_Output_aparent_power = 0;
		Log_Load_percentage = 0;
		Log_Grid_discharge_today = 0;
		Log_Grid_discharge_total = 0;
		Log_PV_Production_today = 0;
		Log_PV_Production_total = 0;
		Log_Battery_discharge_today = 0;
		Log_Battery_discharge_total = 0;
		Log_Inverter_temperature = 0;
		Log_DC_DC_temperature = 0;
		memset(Log_Error, 0, sizeof(Log_Error));
		memset(Log_Warning, 0, sizeof(Log_Warning));
		memset(Log_State, 0, sizeof(Log_State));
		memset(Log_Charge_priority, 0, sizeof(Log_Charge_priority));
		Log_Max_charge_current = 0;
		Log_AC_Charge_current = 0;
		
    _Log_Error=0;
    _Log_Warning=0;
    _Log_State=0;
    _Log_Charge_priority=0;

		sampleCount = 0;
	}
}

static const char LogDecodeDataJson[] PROGMEM = R"EOF(
{
"TimeStamp":"%s/%s/%s %02d-%02d-%02d",
"Grid_voltage":%.2f,
"Grid_frequency":%.2f,
"Grid_input_power":%d,
"Grid_charge_current":%.2f,
"PV_Voltage":%.2f,
"PV_Charge_current": %.2f,
"PV_Power":%d,
"Battery_voltage":%.2f,
"Battery_capacity":%d,
"Battery_charge_current":%.2f,
"Battery_discharge_current":%.2f,
"Output_voltage":%.2f,
"Output_frequency":%.2f,
"Output_active_power":%d,
"Output_aparent_power":%d,
"Load_percentage":%.2f,
"Grid_discharge_today":%.2f,
"Grid_discharge_total":%.2f,
"PV_Production_today":%.2f,
"PV_Production_total":%.2f,
"Battery_discharge_today":%.2f,
"Battery_discharge_total":%.2f,
"Inverter_temperature":%.2f,
"DC-DC_temperature":%.2f,
"Error":"%s",
"Warning":"%s",
"State":"%s",
"Charge_priority":"%s",
"Max_charge_current":%d,
"AC_Charge_current":%d
})EOF";

static uint32_t lastFault = 0;
static uint32_t lastWarning = 0;

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

#if USE_HTTPS
void sendLogsName(BearSSL::ESP8266WebServerSecure *server) {
#else
void sendLogsName(ESP8266WebServer *server) {
#endif
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/plain", "");
  Dir dir = SPIFFS.openDir("pvlog");
  bool filesFound = false;
  while (dir.next()) {
    File entry = dir.openFile("r");
    if(entry) {
      String content;
      String name = entry.name();
      name.replace(".bin", ".json");
      name.replace("pvlog/", "");
      content += name;
      content += ", ";
      content += formatBytes(entry.size());
      content += "\n";
      server->sendContent(content);
      entry.close();
      filesFound = true;
    }
  }
  if(!filesFound)
    server->sendContent("Directory empty.\n");
  server->client().stop();
}

#if USE_HTTPS
void sendLogLen(BearSSL::ESP8266WebServerSecure *server) {
#else
void sendLogLen(ESP8266WebServer *server) {
#endif
  String path;
  path = "pvlog/";
  path += server->arg(0);
  path.replace(F(".json"), F(".bin"));
  File pFile = SPIFFS.open((char *)path.c_str(), "r");
  if(!pFile) {
    server->send(200, "text/plain", "Not found.");
    return;
  }
  char buf[17];
  snprintf_P(buf, sizeof(buf), "Entry's: %d", pFile.size() / sizeof(log_v1_t));
  server->send(200, "text/plain", buf);
  return;
}

#if USE_HTTPS
void decodeSendLogData(BearSSL::ESP8266WebServerSecure *server) {
#else
void decodeSendLogData(ESP8266WebServer *server) {
#endif
  if(server->arg(0).indexOf(".json") < 0) {
    server->send(200, "text/plain", "Wrong extension.");
    return;
  }
  String path;
  path = "pvlog/";
  path += server->arg(0);
  path.replace(F(".json"), F(".bin"));
  File pFile = SPIFFS.open((char *)path.c_str(), "r");
  if(!pFile) {
    server->send(200, "text/plain", "Not found.");
    return;
  }
  if(!pFile.size()) {
    server->send(200, "text/plain", "File is empty.");
    pFile.close();
    return;
  }
  log_v1_t entry;
  int cnt = 0;
  int end = pFile.size();
  if(server->argName(1).equals("start") && server->argName(2).equals("end")) {
    int _start = server->arg(1).toInt();
    int _end = server->arg(2).toInt();
    if(_start > (_end == -1 ? (end / sizeof(log_v1_t)) : _end)) {
      server->send(200, "text/plain", "Start need to be less or equal to End.");
      pFile.close();
      return;
    }
    if(_start * sizeof(log_v1_t) >= end) {
      server->send(200, "text/plain", "Start out of range.");
      pFile.close();
      return;
    }
    cnt = _start * sizeof(log_v1_t);
    if(_end != -1)
      end = (_end + 1) * sizeof(log_v1_t);
    if(end > pFile.size())
      end = pFile.size();
  } else if(server->argName(1).equals("start")) {
    int _start = server->arg(1).toInt();
    if(_start * sizeof(log_v1_t) >= end) {
      server->send(200, "text/plain", "Start out of range.");
      pFile.close();
      return;
    }
    cnt = _start * sizeof(log_v1_t);
  } else if(!server->argName(1).equals("")) {
      server->send(200, "text/plain", "Invalid request.");
      pFile.close();
      return;
  }
  String tmpStr = server->arg(0);
  String year = tmpStr.substring(0, tmpStr.indexOf("_"));
  tmpStr = tmpStr.substring(tmpStr.indexOf("_") + 1);
  String month = tmpStr.substring(0, tmpStr.indexOf("_"));
  tmpStr = tmpStr.substring(tmpStr.indexOf("_") + 1);
  String day = tmpStr.substring(0, tmpStr.indexOf("."));
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/json", "");
  server->sendContent("[");
  for(; cnt < end; ) {
    pFile.seek(cnt);
    if(sizeof(log_v1_t) != pFile.readBytes((char *)&entry, sizeof(log_v1_t))) {
      server->send(200, "text/plain", "Unable to read file.");
      pFile.close();
      return;
    }
    char error[257] = {0};
    char warning[257] = {0};
    char state[65] = {0};
    char priority[65] = {0};

    int sec = entry.sec;
    int min = entry.min;
    int hour = entry.hour;
    double log_Grid_voltage = (double)entry.Log_Grid_voltage / 10.0;//X10
    double log_Grid_frequency = (double)entry.Log_Grid_frequency / 100.0;//X100
    int log_Grid_input_power = entry.Log_Grid_input_power;
    double log_Grid_charge_current = (double)entry.Log_Grid_charge_current / 100.0;//X100
    double log_PV_Voltage = (double)entry.Log_PV_Voltage / 10.0;//X10
    double log_PV_Charge_current = (double)entry.Log_PV_Charge_current / 100.0;//X100
    int log_PV_Power = entry.Log_PV_Power;
    double log_Battery_voltage = (double)entry.Log_Battery_voltage / 100.0;//X100
    int log_Battery_capacity = entry.Log_Battery_capacity;
    double log_Battery_charge_current = (double)entry.Log_Battery_charge_current / 100.0;//X100
    double log_Output_voltage = (double)entry.Log_Output_voltage / 100.0;//X100
    double log_Output_frequency = (double)entry.Log_Output_frequency / 100.0;//X100
    int log_Output_active_power = entry.Log_Output_active_power;
    int log_Output_aparent_power = entry.Log_Output_aparent_power;
    double log_Load_percentage = (double)entry.Log_Load_percentage / 10.0;//X10
    double log_Grid_discharge_today = (double)entry.Log_Grid_discharge_today / 10.0;//X10
    double log_Grid_discharge_total = (double)entry.Log_Grid_discharge_total / 10.0;//X10
    double log_PV_Production_today = (double)entry.Log_PV_Production_today / 10.0;//X10
    double log_PV_Production_total = (double)entry.Log_PV_Production_total / 10.0;//X10
    double log_Battery_discharge_today = (double)entry.Log_Battery_discharge_today / 10.0;//X10
    double log_Battery_discharge_total = (double)entry.Log_Battery_discharge_total / 10.0;//X10
    double log_Inverter_temperature = (double)entry.Log_Inverter_temperature / 10.0;//X10
    double log_DC_DC_temperature = (double)entry.Log_DC_DC_temperature / 10.0;//X10
    int log_Max_charge_current = entry.Log_Max_charge_current;
    int log_Rated_Output_voltage = entry.Log_Rated_Output_voltage;
    int log_AC_Charge_current = entry.Log_AC_Charge_current;

		memset(error, 0, sizeof(error));
		if (entry.Log_Error & 0x0001)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x0002)
			strncat(error, "CPU A->B Communication error", sizeof(error));
		if (entry.Log_Error & 0x0004)
			strncat(error, "Battery sample inconsistent", sizeof(error));
		if (entry.Log_Error & 0x0008)
			strncat(error, "BUCK over current", sizeof(error));
		if (entry.Log_Error & 0x0010)
			strncat(error, "BMS communication fault", sizeof(error));
		if (entry.Log_Error & 0x0020)
			strncat(error, "Battery unnormal", sizeof(error));
		if (entry.Log_Error & 0x0040)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x0080)
			strncat(error, "Battery high voltage", sizeof(error));
		if (entry.Log_Error & 0x0100)
			strncat(error, "Over temperature", sizeof(error));
		if (entry.Log_Error & 0x0200)
			strncat(error, "Over load", sizeof(error));
		if (entry.Log_Error & 0x0400)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x0800)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x1000)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x2000)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x4000)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x8000)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x00010000)
			strncat(error, "Battery reverse connaction", sizeof(error));
		if (entry.Log_Error & 0x00020000)
			strncat(error, "BUS soft start fail", sizeof(error));
		if (entry.Log_Error & 0x00040000)
			strncat(error, "DC-DC unnormal", sizeof(error));
		if (entry.Log_Error & 0x00080000)
			strncat(error, "DC voltage high", sizeof(error));
		if (entry.Log_Error & 0x00100000)
			strncat(error, "CT detect failed", sizeof(error));
		if (entry.Log_Error & 0x00200000)
			strncat(error, "CPU B->A Communication error", sizeof(error));
		if (entry.Log_Error & 0x00400000)
			strncat(error, "BUS voltage high", sizeof(error));
		if (entry.Log_Error & 0x00800000)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x01000000)
			strncat(error, "MOV break", sizeof(error));
		if (entry.Log_Error & 0x02000000)
			strncat(error, "Output short circuit", sizeof(error));
		if (entry.Log_Error & 0x04000000)
			strncat(error, "Li-Battery over load", sizeof(error));
		if (entry.Log_Error & 0x08000000)
			strncat(error, "Output voltage high", sizeof(error));
		if (entry.Log_Error & 0x10000000)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x20000000)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x40000000)
			strncat(error, "\\", sizeof(error));
		if (entry.Log_Error & 0x80000000)
			strncat(error, "\\", sizeof(error));
		
		
		for (int i = 0; i < 32; i++) {
			lastWarning++;
			if(lastWarning >= 31) {
				lastWarning = 0;
			}
			if(entry.Log_Warning & (1 << lastWarning)) {
				entry.Log_Warning &= 1 << lastWarning;
				break;
			}
		}
		
		memset(warning, 0, sizeof(warning));
		if (entry.Log_Warning & 0x0001)
			strncat(warning, "Battery voltage low", sizeof(warning));
		if (entry.Log_Warning & 0x0002)
			strncat(warning, "Over temperature warning", sizeof(warning));
		if (entry.Log_Warning & 0x0004)
			strncat(warning, "Over load warning", sizeof(warning));
		if (entry.Log_Warning & 0x0008)
			strncat(warning, "Fail to read EEPROM", sizeof(warning));
		if (entry.Log_Warning & 0x0010)
			strncat(warning, "Firmware version unmatch", sizeof(warning));
		if (entry.Log_Warning & 0x0020)
			strncat(warning, "Fail to write EEPROM", sizeof(warning));
		if (entry.Log_Warning & 0x0040)
			strncat(warning, "BMS warning", sizeof(warning));
		if (entry.Log_Warning & 0x0080)
			strncat(warning, "Li-Battery over load warninge", sizeof(warning));
		if (entry.Log_Warning & 0x0100)
			strncat(warning, "Li-Battery aging warning", sizeof(warning));
		if (entry.Log_Warning & 0x0200)
			strncat(warning, "Fan lock", sizeof(warning));
		if (entry.Log_Warning & 0x0400)
			strncat(warning, "\\", sizeof(warning));
		if (entry.Log_Warning & 0x0800)
			strncat(warning, "\\", sizeof(warning));
		if (entry.Log_Warning & 0x1000)
			strncat(warning, "\\", sizeof(warning));
		if (entry.Log_Warning & 0x2000)
			strncat(warning, "\\", sizeof(warning));
		if (entry.Log_Warning & 0x4000)
			strncat(warning, "\\", sizeof(warning));
		if (entry.Log_Warning & 0x8000)
			strncat(warning, "\\", sizeof(warning));
		
		switch(entry.Log_State) {
			case 0:
				strcpy(state, "StandBy");
				break;
			case 1:
				strcpy(state, "NotUse");
				break;
			case 2:
				strcpy(state, "Discharge");
				break;
			case 3:
				strcpy(state, "Fault");
				break;
			case 4:
				strcpy(state, "Flash");
				break;
			case 5:
				strcpy(state, "PV Charge");
				break;
			case 6:
				strcpy(state, "AC Charge");
				break;
			case 7:
				strcpy(state, "Combine Charge");
				break;
			case 8:
				strcpy(state, "Combine Charge and Bypass");
				PV_PowerLoad = 0;
				Battery_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM GRID");
				break;
			case 9:
				strcpy(state, "PV Charge and Bypass");
				PV_PowerLoad = 0;
				Battery_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM GRID");
				break;
			case 10:
				strcpy(state, "AC Charge and Bypass");
				PV_PowerLoad = 0;
				Battery_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM GRID");
				break;
			case 11:
				strcpy(state, "Bypass");
				PV_PowerLoad = 0;
				Battery_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM GRID");
				break;
			case 12:
				strcpy(state, "PV Charge and Discharge");
				break;
			default:
				strcpy(state, "Not defined");
			break;
		}
		switch(entry.Log_Charge_priority) {
			case 0:
				strcpy(Charge_priority, "PV First");
				break;
			case 1:
				strcpy(Charge_priority, "PV & Utility");
				break;
			case 2:
				strcpy(Charge_priority, "PV Only");
				break;
			default:
				strcpy(Charge_priority, "?");
				break;
		}
    double batteryChargeCurrent = (double)entry.Log_Battery_charge_current;
    bool charging = batteryChargeCurrent < 0 ? true : false;
    int finalLen = snprintf_P(nullptr, 0, LogDecodeDataJson,
      year, month, day, entry.hour, entry.min, entry.sec,
      log_Grid_voltage, log_Grid_frequency, log_Grid_input_power, log_Grid_charge_current,
      log_PV_Voltage, log_PV_Charge_current, log_PV_Power,
      log_Battery_voltage, log_Battery_capacity, charging ? 0.0 - log_Battery_charge_current : 0.0, charging ? 0.0 : log_Battery_charge_current,
      log_Output_voltage, log_Output_frequency, log_Output_active_power, log_Output_aparent_power, log_Load_percentage,
      log_Grid_discharge_today, log_Grid_discharge_total, log_PV_Production_today, log_PV_Production_total, log_Battery_discharge_today, log_Battery_discharge_total,
      log_Inverter_temperature, log_DC_DC_temperature,
      error, warning,
      state, priority,
      log_Max_charge_current, log_AC_Charge_current
    ); 
    //server->send(200, "text/plain", "Success.");
    //return;
    if(finalLen < 1) {
      server->send(200, "text/plain", "Invalid len.");
      pFile.close();
      return;
    }   
    char *buf = new char[finalLen + 1];
    if(!buf) {
      server->send(200, "text/plain", F("Error allocating buffer."));
      pFile.close();
      return;
    }   
    snprintf_P(buf, finalLen + 1, LogDecodeDataJson,
      year, month, day, entry.hour, entry.min, entry.sec,
      log_Grid_voltage, log_Grid_frequency, log_Grid_input_power, log_Grid_charge_current,
      log_PV_Voltage, log_PV_Charge_current, log_PV_Power,
      log_Battery_voltage, log_Battery_capacity, charging ? 0.0 - log_Battery_charge_current : 0.0, charging ? 0.0 : log_Battery_charge_current,
      log_Output_voltage, log_Output_frequency, log_Output_active_power, log_Output_aparent_power, log_Load_percentage,
      log_Grid_discharge_today, log_Grid_discharge_total, log_PV_Production_today, log_PV_Production_total, log_Battery_discharge_today, log_Battery_discharge_total,
      log_Inverter_temperature, log_DC_DC_temperature,
      error, warning,
      state, priority,
      log_Max_charge_current, log_AC_Charge_current
    );
    server->sendContent(buf, finalLen);
    if(pFile.position() != end) {
      server->sendContent(",");
    }
    delete []buf;
    cnt += sizeof(log_v1_t);
  }
  server->sendContent("\n]");
  server->client().stop();
  pFile.close();
}

void Log_ComposeLogJson(char *buffer, int bufferSize) {
	time_t T = time(NULL);
	struct tm tm = *localtime(&T);
	 snprintf(buffer, bufferSize, LogJson,
	 	tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
		Grid_voltage, Grid_frequency, Grid_input_power, Grid_charge_current,
		PV_Voltage, PV_Charge_current, PV_Power,
		Battery_voltage,Battery_capacity, Battery_charge_current, Battery_discharge_current,
		Output_voltage, Output_frequency, Output_active_power, Output_aparent_power, Load_percentage,
		Grid_discharge_today, Grid_discharge_total, PV_Production_today, PV_Production_total, Battery_discharge_today, Battery_discharge_total,
		Inverter_temperature, DC_DC_temperature,
		Error, Warning,
		State, Charge_priority,
		Max_charge_current, AC_Charge_current,
		Model, Serial_number, Equipment_mode, Inverter_CPU_version, MPPT_CPU_version, Date_of_manufacture);
}


