#ifndef __PV_H__
#define __PV_H__

#define MAIN_PAGE_BUFFER_LEN  1

extern double Grid_voltage;
extern double Grid_frequency;
extern int Grid_input_power;
extern double Grid_charge_current;
extern double PV_Voltage;
extern double PV_Charge_current;
extern int PV_Power;
extern double Battery_voltage;
extern int Battery_capacity;
extern double Battery_charge_current;
extern double Battery_discharge_current;
extern double Output_voltage;
extern double Output_frequency;
extern int Output_active_power;
extern int Output_aparent_power;
extern double Load_percentage;
extern double Grid_discharge_today;
extern double Grid_discharge_total;
extern double PV_Production_today;
extern double PV_Production_total;
extern double Battery_discharge_today;
extern double Battery_discharge_total;
extern double Inverter_temperature;
extern double DC_DC_temperature;
extern char Error[];
extern char Warning[];
extern char State[];
extern char Charge_priority[];
extern int Grid_rated_voltage;
extern int Grid_rated_frequency;
extern int Rated_battery_voltage;
extern int Max_charge_current;
extern int Rated_Output_voltage;
extern int Rated_Output_frequency;
extern double Rated_Output_power;
extern int AC_Charge_current;
extern char Model[];
extern char Serial_number[];
extern char Equipment_mode[];
extern char Inverter_CPU_version[];
extern char MPPT_CPU_version[];
extern char Date_of_manufacture[];

extern int BatteryPower;
extern char PowerSourceStatus[];
extern double PV_PowerLoad;
extern double Battery_PowerLoad;

extern uint32_t faultCode;
extern uint32_t warningCode;
extern uint16_t systemStatus;
extern uint8_t chargePriority;


typedef struct {
  float Grid_voltage;
  float Grid_frequency;
  uint16_t Grid_input_power;
  float Grid_charge_current;
  float PV_Voltage;
  float PV_Charge_current;
  uint16_t PV_Power;
  float Battery_voltage;
  uint8_t Battery_capacity;
  float Battery_charge_current;
  float Battery_discharge_current;
  float Output_voltage;
  float Output_frequency;
  uint16_t Output_active_power;
  uint16_t Output_aparent_power;
  float Load_percentage;
  float Inverter_temperature;
  float DC_DC_temperature;
  uint8_t Max_charge_current;
  uint8_t AC_Charge_current;
}spiffsLog_t;

void PV_Init();
void PV_Loop();
void PV_triggerDataRead();
long PV_getDataFefreshCnt();
unsigned short *PV_getHoldingReg();
unsigned short *PV_getInputReg();

#endif