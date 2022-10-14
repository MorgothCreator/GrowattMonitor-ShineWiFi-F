#ifndef __REPORT_H__
#define __REPORT_H__

void reportAppend(char *data);
void reportAppend(char *data, int len);
void reportAppendLn(char *data);
void reportPrintF(const char *format, ...);
char *getReport();

#endif