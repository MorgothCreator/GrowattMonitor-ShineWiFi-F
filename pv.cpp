#include <arduFPGA-app-common-arduino.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
////#include <sys/stat.h>
//#include <unistd.h>
//#include <stdio.h>
//#include <fcntl.h>
#include <math.h>

#include <ESP8266WiFi.h>

#include "pv.h"
//#include "../sdk/sys/timer.h"
#include "mainWebPage.h"

#define DEVICE_ID	1

static int sentCommand = 3;
static int sentAddress = 0;
static char uartReceiveArray[128];
static int uartReceiveCnt;
unsigned short holdingReg[135];
unsigned short inputReg[90];
static sTimer *tDataRead;
static sTimer *tReadDataTimeout;
static long dataRefreshCnt = 0;

static unsigned int CRC16_2(unsigned char *buf, int len) {
	unsigned int crc = 0xFFFF;
	for (int pos = 0; pos < len; pos++) {
		crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc
		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			} else                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}
	return crc;
}

static void getModbus(unsigned short *array, int len) {
	for (int i = 0; i < len; i++) {
		array[i] = ((array[i] >> 8) & 0xFF) | ((array[i] << 8) & 0xFF00);
	}
}
		
		
int lastFault = 0;
int lastWarning = 0;
short systemStatus = 0;

double Grid_voltage=220;
double Grid_frequency=50.00;
int Grid_input_power=0;
double Grid_charge_current=0;
double PV_Voltage=230.0;
double PV_Charge_current=57.0;
int PV_Power=3072;
double Battery_voltage=50.69;
int Battery_capacity=93;
double Battery_charge_current=30.4;
double Battery_discharge_current=0;
double Output_voltage=230;
double Output_frequency=50.01;
int Output_active_power=293;
int Output_aparent_power=387;
double Load_percentage=6.1;
double Grid_discharge_today=0;
double Grid_discharge_total=108.4;
double PV_Production_today=10.4;
double PV_Production_total=1534.7;
double Battery_discharge_today=1.5;
double Battery_discharge_total=516.1;
double Inverter_temperature=26.4;
double DC_DC_temperature=24.4;
char Error[257]="";
char Warning[257]="";
char State[33]="PV_Charge_and_discharge";
char Charge_priority[32]="PV_Only";
int Grid_rated_voltage=230;
int Grid_rated_frequency=50;
int Rated_battery_voltage=48;
int Max_charge_current=75;
int Rated_Output_voltage=230;
int Rated_Output_frequency=50;
double Rated_Output_power=5000;
int AC_Charge_current=20;
char Model[17]="";
char Serial_number[17]="RKG6BGA0E4";
char Equipment_mode[17]="";
char Inverter_CPU_version[17]="040.04";
char MPPT_CPU_version[17]="041.04";
char Date_of_manufacture[17]="";

int BatteryPower = 0;
char PowerSourceStatus[129] = "POWERED FROM GRID";
double PV_PowerLoad = 20;
double Battery_PowerLoad = 30;

static double rssf( unsigned short *array, int index, int scale) {
	return (double)array[index] / scale;
}

static double rsdf( unsigned short *array, int index, int scale) {
	return (double)(array[index] << 16 | array[index + 1]) / scale;
}
#define LED1_Pin  16

static void requestData(char devId, short command, short addr, short len) {
	uartReceiveCnt = 0;
	memset(uartReceiveArray, 0, sizeof(uartReceiveArray));
	unsigned char array[8];
	array[0] = devId;
	array[1] = command;
	array[2] = addr >> 8;
	array[3] = addr;
	array[4] = len >> 8;
	array[5] = len;
	short crc = CRC16_2(array, 6);
	array[6] = crc;
	array[7] = crc >> 8;
  digitalWrite(LED1_Pin, 1);
	Serial.write(array, sizeof(array));
  digitalWrite(LED1_Pin, 0);
}

void PV_Init() {
	tDataRead = new sTimer(20);
	tDataRead->Stop();
#if USE_HTTPS
	tReadDataTimeout = new sTimer(1000); // increase data read timeout because of HTTPS lock for a longer time.
#else
	tReadDataTimeout = new sTimer(100);
#endif
	tReadDataTimeout->Stop();
}

void PV_Loop() {
	int c;
	if((c = Serial.read()) != -1) {
		if(uartReceiveCnt < sizeof(uartReceiveArray)) {
			tReadDataTimeout->Start();
			uartReceiveArray[uartReceiveCnt] = c;
			uartReceiveCnt++;
		}
	}
	if(tDataRead->Tick()) {
		tDataRead->Stop();
		//printf("Tick\n");
		//holdingReg.clear();
		//inputReg.clear();
		sentCommand = 3;
		sentAddress = 0;
		requestData(DEVICE_ID, 3, 0x00, 0x2D);
		//uart_flush(uart);
	}
	bool dataHasBeenRefreshed = false;
	if(tReadDataTimeout->Tick()) {
		tReadDataTimeout->Stop();
		int cnt = 0;
		//for (cnt = 0; cnt < uartReceiveCnt; cnt++) {
		//	printf("0x%X ", uartReceiveArray[cnt]);
		//}
		//printf("\n%d\n", uartReceiveCnt);
		switch (sentCommand) {
			case 3: {
				switch (sentAddress) {
					case 0x00: {
						memcpy((char *)&holdingReg[0], &uartReceiveArray[3], 0x2D * sizeof(short));
						sentAddress = 0x2D;
						requestData(DEVICE_ID, sentCommand, sentAddress, 0x2D);
					}
					break;
					case 0x2D: {
						memcpy((char *)&holdingReg[45], &uartReceiveArray[3], 0x2D * sizeof(short));
						sentAddress = 0x54;
						requestData(DEVICE_ID, sentCommand, sentAddress, 0x2D);
					}
					break;
					case 0x54: {
						memcpy((char *)&holdingReg[90], &uartReceiveArray[3], 0x2D * sizeof(short));
						getModbus(holdingReg, sizeof(holdingReg) / sizeof(holdingReg[0]));
						//for (cnt = 0; cnt < 135; cnt++) {
						//	printf("0x%X ", holdingReg[cnt]);
						//}
						//printf("\n\n", uartReceiveCnt);
						sentCommand = 4;
						sentAddress = 0x00;
						requestData(DEVICE_ID, sentCommand, sentAddress, 0x2D);
					}
					break;
				}
			}
			break;
			case 4: {
			}
				switch (sentAddress) {
					case 0x00: {
						memcpy((char *)&inputReg[0], &uartReceiveArray[3], 0x2D * sizeof(short));
						sentAddress = 0x2D;
						requestData(DEVICE_ID, sentCommand, sentAddress, 0x2D);
					}
					break;
					case 0x2D: {
						memcpy((char *)&inputReg[45], &uartReceiveArray[3], 0x2D * sizeof(short));
						getModbus(inputReg, sizeof(inputReg) / sizeof(short));
						//for (cnt = 0; cnt < 90*2; cnt++) {
						//	printf("0x%X ", inputReg[cnt]);
						//}
						//printf("\n\n", uartReceiveCnt);
						sentCommand = 3;
						sentAddress = 0x00;
						dataHasBeenRefreshed = true;
					}
					break;
				}
			break;
		}
	}
	
	if(dataHasBeenRefreshed) {
		memset(State, 0, sizeof(State));
		memset(Charge_priority, 0, sizeof(Charge_priority));
		memset(Model, 0, sizeof(Model));
		memset(Serial_number, 0, sizeof(Serial_number));
		memset(Equipment_mode, 0, sizeof(Equipment_mode));
		memset(Inverter_CPU_version, 0, sizeof(Inverter_CPU_version));
		memset(MPPT_CPU_version, 0, sizeof(MPPT_CPU_version));
		memset(Date_of_manufacture, 0, sizeof(Date_of_manufacture));
		
		unsigned short *holdingReg = PV_getHoldingReg();
		unsigned short *inputReg = PV_getInputReg();
		
		Inverter_CPU_version[0] = holdingReg[9] >> 8;
		Inverter_CPU_version[1] = holdingReg[9];
		Inverter_CPU_version[2] = holdingReg[10] >> 8;
		Inverter_CPU_version[3] = holdingReg[10];
		Inverter_CPU_version[4] = holdingReg[11] >> 8;
		Inverter_CPU_version[5] = holdingReg[11];
		
		MPPT_CPU_version[0] = holdingReg[12] >> 8;
		MPPT_CPU_version[1] = holdingReg[12];
		MPPT_CPU_version[2] = holdingReg[13] >> 8;
		MPPT_CPU_version[3] = holdingReg[13];
		MPPT_CPU_version[4] = holdingReg[14] >> 8;
		MPPT_CPU_version[5] = holdingReg[14];
		
		Serial_number[0] = holdingReg[23] >> 8;
		Serial_number[1] = holdingReg[23];
		Serial_number[2] = holdingReg[24] >> 8;
		Serial_number[3] = holdingReg[24];
		Serial_number[4] = holdingReg[25] >> 8;
		Serial_number[5] = holdingReg[25];
		Serial_number[6] = holdingReg[26] >> 8;
		Serial_number[7] = holdingReg[26];
		Serial_number[8] = holdingReg[27] >> 8;
		Serial_number[9] = holdingReg[27];
		
		int modelNumberInt = (holdingReg[28] << 16) | holdingReg[29];
		Model[0] = 'T';
		Model[0] = (modelNumberInt >> 20) & 0x0F;
		Model[0] = 'Q';
		Model[0] = (modelNumberInt >> 16) & 0x0F;
		Model[0] = 'P';
		Model[0] = (modelNumberInt >> 12) & 0x0F;
		Model[0] = 'U';
		Model[0] = (modelNumberInt >> 8) & 0x0F;
		Model[0] = 'M';
		Model[0] = (modelNumberInt >> 4) & 0x0F;
		Model[0] = 'S';
		Model[0] = modelNumberInt & 0x0F;
		
		bool charging = rsdf(inputReg, 77, 10) < 0 ? 1 : 0;
		int faultCode = inputReg[42];
		int warningCode = inputReg[43];
			
		Grid_voltage = rssf(inputReg, 20, 10);
		Grid_frequency = rssf(inputReg, 21, 100);
		Grid_input_power = rsdf(inputReg, 36, 10);
		Grid_charge_current = rsdf(inputReg, 13, 10) / rssf(inputReg, 17, 100);
		PV_Voltage = rssf(inputReg, 1, 10);
		PV_Charge_current = rsdf(inputReg, 3, 10) / rssf(inputReg, 1, 10);
		if(isnan(PV_Charge_current) || isinf(PV_Charge_current))
			PV_Charge_current = 0;
		PV_Power = rsdf(inputReg, 3, 10);
		Battery_voltage = rssf(inputReg, 17, 100);
		Battery_capacity = rssf(inputReg, 18, 1);
		Battery_charge_current = charging ? (0 - rsdf(inputReg, 77, 10)) / Battery_voltage : 0;
		Battery_discharge_current = charging ? 0 : rsdf(inputReg, 77, 10) / Battery_voltage;
		Output_voltage = rssf(inputReg, 22, 10);
		Output_frequency = rssf(inputReg, 23, 100);
		Output_active_power = rsdf(inputReg, 9, 10);
		Output_aparent_power = rsdf(inputReg, 11, 10);
		Load_percentage = rssf(inputReg, 27, 10);
		Grid_discharge_today = rsdf(inputReg, 64, 10);
		Grid_discharge_total = rsdf(inputReg, 66, 10);
		PV_Production_today = rsdf(inputReg, 48, 10);
		PV_Production_total = rsdf(inputReg, 50, 10);
		Battery_discharge_today = rsdf(inputReg, 60, 10);
		Battery_discharge_total = rsdf(inputReg, 62, 10);
		Inverter_temperature = rssf(inputReg, 25, 10);
		DC_DC_temperature = rssf(inputReg, 26, 10);
		Grid_rated_voltage = rssf(holdingReg, 90, 10);
		Grid_rated_frequency = rssf(holdingReg, 91, 10);
		Rated_battery_voltage = rssf(holdingReg, 92, 10);
		Max_charge_current = rssf(holdingReg, 34, 1);
		
		BatteryPower = (int)charging ? 0 : rsdf(inputReg, 77,10);
		
		strcpy(PowerSourceStatus, "POWERED FROM PANELS AND BATTERY");
		if(PV_Power) {
			PV_PowerLoad = ((double)((double)Output_active_power / (double)PV_Power) * 100.0);
			if(PV_PowerLoad > 100)
				PV_PowerLoad = 100;
			else
				strcpy(PowerSourceStatus, "POWERED FROM PANELS");
		} else {
				PV_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM BATTERY");
		}
		double batteryDischargePower = rsdf(inputReg, 77, 10) > 0.00001 ? rsdf(inputReg, 77, 10) : 0.00001;
		double pvPower = PV_Power > 0.00001 ? PV_Power : 0.00001;
		Battery_PowerLoad = 100 / ((batteryDischargePower + pvPower) / batteryDischargePower);
		if(Battery_PowerLoad > 100)
			Battery_PowerLoad = 100;


		switch(holdingReg[18]) {
			case 0:
				Rated_Output_voltage = 208;
				break;
			case 1:
				Rated_Output_voltage = 230;
				break;
			case 2:
				Rated_Output_voltage = 240;
				break;
			default:
				Rated_Output_voltage = 0;
				break;
		}
		switch(holdingReg[19]) {
			case 0:
				Rated_Output_frequency = 50;
				break;
			case 1:
				Rated_Output_frequency = 60;
				break;
			default:
				Rated_Output_frequency = 0;
				break;
		}
		Rated_Output_power = rsdf(holdingReg, 76, 10);
		AC_Charge_current=rssf(holdingReg, 99, 1);
		
		char faultStr[257];
		memset(faultStr, 0, sizeof(faultStr));
		if (faultCode & 0x0001)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x0002)
			strncat(Error, "CPU A->B Communication error", sizeof(Error));
		if (faultCode & 0x0004)
			strncat(Error, "Battery sample inconsistent", sizeof(Error));
		if (faultCode & 0x0008)
			strncat(Error, "BUCK over current", sizeof(Error));
		if (faultCode & 0x0010)
			strncat(Error, "BMS communication fault", sizeof(Error));
		if (faultCode & 0x0020)
			strncat(Error, "Battery unnormal", sizeof(Error));
		if (faultCode & 0x0040)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x0080)
			strncat(Error, "Battery high voltage", sizeof(Error));
		if (faultCode & 0x0100)
			strncat(Error, "Over temperature", sizeof(Error));
		if (faultCode & 0x0200)
			strncat(Error, "Over load", sizeof(Error));
		if (faultCode & 0x0400)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x0800)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x1000)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x2000)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x4000)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x8000)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x00010000)
			strncat(Error, "Battery reverse connaction", sizeof(Error));
		if (faultCode & 0x00020000)
			strncat(Error, "BUS soft start fail", sizeof(Error));
		if (faultCode & 0x00040000)
			strncat(Error, "DC-DC unnormal", sizeof(Error));
		if (faultCode & 0x00080000)
			strncat(Error, "DC voltage high", sizeof(Error));
		if (faultCode & 0x00100000)
			strncat(Error, "CT detect failed", sizeof(Error));
		if (faultCode & 0x00200000)
			strncat(Error, "CPU B->A Communication error", sizeof(Error));
		if (faultCode & 0x00400000)
			strncat(Error, "BUS voltage high", sizeof(Error));
		if (faultCode & 0x00800000)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x01000000)
			strncat(Error, "MOV break", sizeof(Error));
		if (faultCode & 0x02000000)
			strncat(Error, "Output short circuit", sizeof(Error));
		if (faultCode & 0x04000000)
			strncat(Error, "Li-Battery over load", sizeof(Error));
		if (faultCode & 0x08000000)
			strncat(Error, "Output voltage high", sizeof(Error));
		if (faultCode & 0x10000000)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x20000000)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x40000000)
			strncat(Error, "\\", sizeof(Error));
		if (faultCode & 0x80000000)
			strncat(Error, "\\", sizeof(Error));
		
		
		for (int i = 0; i < 32; i++) {
			lastWarning++;
			if(lastWarning >= 31) {
				lastWarning = 0;
			}
			if(warningCode & (1 << lastWarning)) {
				warningCode &= 1 << lastWarning;
				break;
			}
		}
		
		char warningStr[257];
		memset(warningStr, 0, sizeof(warningStr));
		if (warningCode & 0x0001)
			strncat(Warning, "Battery voltage low", sizeof(Warning));
		if (warningCode & 0x0002)
			strncat(Warning, "Over temperature warning", sizeof(Warning));
		if (warningCode & 0x0004)
			strncat(Warning, "Over load warning", sizeof(Warning));
		if (warningCode & 0x0008)
			strncat(Warning, "Fail to read EEPROM", sizeof(Warning));
		if (warningCode & 0x0010)
			strncat(Warning, "Firmware version unmatch", sizeof(Warning));
		if (warningCode & 0x0020)
			strncat(Warning, "Fail to write EEPROM", sizeof(Warning));
		if (warningCode & 0x0040)
			strncat(Warning, "BMS warning", sizeof(Warning));
		if (warningCode & 0x0080)
			strncat(Warning, "Li-Battery over load warninge", sizeof(Warning));
		if (warningCode & 0x0100)
			strncat(Warning, "Li-Battery aging warning", sizeof(Warning));
		if (warningCode & 0x0200)
			strncat(Warning, "Fan lock", sizeof(Warning));
		if (warningCode & 0x0400)
			strncat(Warning, "\\", sizeof(Warning));
		if (warningCode & 0x0800)
			strncat(Warning, "\\", sizeof(Warning));
		if (warningCode & 0x1000)
			strncat(Warning, "\\", sizeof(Warning));
		if (warningCode & 0x2000)
			strncat(Warning, "\\", sizeof(Warning));
		if (warningCode & 0x4000)
			strncat(Warning, "\\", sizeof(Warning));
		if (warningCode & 0x8000)
			strncat(Warning, "\\", sizeof(Warning));
		
		systemStatus = inputReg[0];
		switch(systemStatus) {
			case 0:
				strcpy(State, "StandBy");
				break;
			case 1:
				strcpy(State, "NotUse");
				break;
			case 2:
				strcpy(State, "Discharge");
				break;
			case 3:
				strcpy(State, "Fault");
				break;
			case 4:
				strcpy(State, "Flash");
				break;
			case 5:
				strcpy(State, "PV Charge");
				break;
			case 6:
				strcpy(State, "AC Charge");
				break;
			case 7:
				strcpy(State, "Combine Charge");
				break;
			case 8:
				strcpy(State, "Combine Charge and Bypass");
				PV_PowerLoad = 0;
				Battery_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM GRID");
				break;
			case 9:
				strcpy(State, "PV Charge and Bypass");
				PV_PowerLoad = 0;
				Battery_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM GRID");
				break;
			case 10:
				strcpy(State, "AC Charge and Bypass");
				PV_PowerLoad = 0;
				Battery_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM GRID");
				break;
			case 11:
				strcpy(State, "Bypass");
				PV_PowerLoad = 0;
				Battery_PowerLoad = 0;
				strcpy(PowerSourceStatus, "POWERED FROM GRID");
				break;
			case 12:
				strcpy(State, "PV Charge and Discharge");
				break;
			default:
				strcpy(State, "Not defined");
			break;
		}
		
		switch(holdingReg[2]) {
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
		dataRefreshCnt++;
		PV_triggerDataRead();
		/*struct stat st = {0};
		if (stat("/tmp/www", &st) == -1) {
			mkdir("/tmp/www", 0777);
		}*/
		//MainWebPage_ComposeHtml
    //MainWebPage_ComposeHtml(mainPageBuffer, sizeof(mainPageBuffer));
		/*int destFile = open("/tmp/www/index.html", O_CREAT | O_WRONLY, 0777);
		write(destFile, mainPageBuffer, strlen(mainPageBuffer));
		close(destFile);
		
		struct stat sts = {0};
		int result = lstat("/tmp/www/log", &sts);
		if(result) {
			char fn[]="/media/usb0/log/";
			char sln[]="/tmp/www/log";
			int stat = symlink(fn, sln);
		}*/
	}
}

void PV_triggerDataRead() {
	tDataRead->Start();
}

long PV_getDataFefreshCnt() {
	return dataRefreshCnt;
}

unsigned short *PV_getHoldingReg() {
	return holdingReg;
}

unsigned short *PV_getInputReg() {
	return inputReg;
}


