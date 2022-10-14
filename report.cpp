#include "report.h"

#include "def.h"

#include <Arduino.h>

char reportBuf[LOG_BUFF_LEN] = {0};

void reportAppend(char *data) {
  int dataLen = strlen(data) + 1;
  char *rsBuf = reportBuf;
  while(1) {
    int reportBufLen = strlen(reportBuf);
    if(dataLen + reportBufLen < sizeof(reportBuf) - 2)
      break;
    while(*rsBuf != 0 && *rsBuf++ != '\n');
    strcpy(reportBuf, rsBuf);
    rsBuf = reportBuf;
  }
  strcat(reportBuf, data);
}

void reportAppend(char *data, int len) {
  char *rsBuf = reportBuf;
  while(1) {
    int reportBufLen = strlen(reportBuf);
    if(len + reportBufLen < sizeof(reportBuf) - 2)
      break;
    while(*rsBuf != 0 && *rsBuf++ != '\n');
    strcpy(reportBuf, rsBuf);
    rsBuf = reportBuf;
  }
  strcat(reportBuf, data);
}

void reportAppendLn(char *data) {
  reportAppend(data);
  reportAppend("<br>\n");
}

void reportPrintF(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    reportAppend(buffer, len);
    if (buffer != temp) {
        delete[] buffer;
    }
}

char *getReport() {
  return reportBuf;
}

