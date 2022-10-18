# GrowattMonitor-ShineWiFi-F

 ShineWiFi-F non official firmware replacement for monitoring Growatt SPF3500/5000ES

This is a non official replacement firmware for Growatt ShineWiFi-F dongle for those that don't want to use cloud storage and monitor the inverter by themselves.

The firmware is developed using Arduino IDE and has implemented an OTA firmware updater for easy update after first firmware upload.

This project is dependent of below libraries and boards:

* Generic ESP8266 Module
* arduFPGA-app-common-arduino by Iulian Gheorghiu
* Rtc_Pcf8563 by Joe Robertson
* NTPClient by Fabrice Weinberg

At this moment is able to:

* Report the inverter status in realtime using a browser pointed to the dongle IP "http://dongle-ip".

Response:

![page](https://raw.githubusercontent.com/MorgothCreator/GrowattMonitor-ShineWiFi-F/main/assets/screenshot.png)

* List all PV log file names "http://dongle-ip/pvlog?ls".

Response:

```
2022_10_16.json, 8.30KB
2022_10_17.json, 18.79KB
2022_10_18.json, 13.28KB
```

The above response can be split using "\n".

* Get the number of entry's in a log file "http://dongle-ip/pvlog?len=2022_10_18.json".

Response:

```
Entry's: 203
```

* Read entry's from a file: "http://dongle-ip/pvlog?get=2022_10_18.json&start=0&end=10", if "end" is -1 or bigger than the number of entry's in the file or not present will read from "start" to the last entry.

Response:

```
[
{
"TimeStamp":"2022/10/18 00-04-54",
"Grid_voltage":0.00,
"Grid_frequency":0.00,
"Grid_input_power":0,
"Grid_charge_current":0.00,
"PV_Voltage":0.00,
"PV_Charge_current": 0.00,
"PV_Power":0,
"Battery_voltage":49.75,
"Battery_capacity":72,
"Battery_charge_current":0.00,
"Battery_discharge_current":0.00,
"Output_voltage":230.00,
"Output_frequency":49.99,
"Output_active_power":213,
"Output_aparent_power":312,
"Load_percentage":4.20,
"Grid_discharge_today":0.00,
"Grid_discharge_total":19.30,
"PV_Production_today":15.00,
"PV_Production_total":648.60,
"Battery_discharge_today":4.10,
"Battery_discharge_total":306.10,
"Inverter_temperature":17.50,
"DC-DC_temperature":15.40,
"Error":"",
"Warning":"",
"State":"Discharge",
"Charge_priority":"",
"Max_charge_current":112,
"AC_Charge_current":20
},
{
"TimeStamp":"2022/10/18 00-09-54",
"Grid_voltage":0.00,
"Grid_frequency":0.00,
"Grid_input_power":0,
"Grid_charge_current":0.00,
"PV_Voltage":0.00,
"PV_Charge_current": 0.00,
"PV_Power":0,
"Battery_voltage":49.76,
"Battery_capacity":71,
"Battery_charge_current":0.00,
"Battery_discharge_current":0.00,
"Output_voltage":230.01,
"Output_frequency":49.99,
"Output_active_power":197,
"Output_aparent_power":306,
"Load_percentage":3.90,
"Grid_discharge_today":0.00,
"Grid_discharge_total":19.30,
"PV_Production_today":15.00,
"PV_Production_total":648.60,
"Battery_discharge_today":4.10,
"Battery_discharge_total":306.10,
"Inverter_temperature":17.30,
"DC-DC_temperature":15.10,
"Error":"",
"Warning":"",
"State":"Discharge",
"Charge_priority":"",
"Max_charge_current":112,
"AC_Charge_current":20
}
]
```

The last one is implemented that way because the device can be quite slow in sending data, if a web page with a chart and a bunch of JS is requesting the data, the JS implementation can buffer the data and request only the new entry's when become available using http://dongle-ip/pvlog?len=2022_10_18.json command checking the number of entry's in the desired file.

The credentials for WiFi AP and OTA updater is located in "credentials.h" file and has the below format:

```
#ifndef __CREDENTIALS_H__
#define __CREDENTIALS_H__

#define OTA_PASS "some_ota_password"

#define WIFI_CREDENTIALS { \
{ \
"SSID 1", \
"password_for_wifi_1" \
}, \
{ \
"SSID 2", \
"password_for_wifi_2" \
}, \
{ \
"", \
"" \
} \
};
#endif
```

You can use one access point or more access points as backup connections.

If user want to read the dongle data through the internet, a HTTPS server can be used changing "#define USE_HTTPS false" to "#define USE_HTTPS true" with the downside of the page access being slower (lot slower), and in this case you need to change the certificate located in "crt.h" file, both private key and public key, at this moment an authentication method is not implemented.

To read the log content the user can enter "http://dongle-ip/report", there the dongle will report what the dongle has done from last start/restart in a 4096 character buffer.

Response:

```
*** RTC PCF8563 ***
17:38:25 18/10/2022 Tuesday


Connecting to Morgoth-AP-MAG
.......
WiFi connected
192.168.1.6
MDNS responder started
Server started
RTC updated
17:38:30 18/10/2022 Tuesday
```

The time of the onboard RTC is automatically updated every start/restart of the dongle if the dongle has access to the internet, the timezone can be set changing "timeClient.setTimeOffset(3*3600);" in "GrowattMonitor-ShineWiFi-F.ino" file, the 3 represent the hours to add or if is negative to subtract from UTC time.


In order to upload for the first time the firmware you need to power up the dongle with the two pins marked as boot shorted, after that you can use the usb connector of the dongle and a USB-RS232 adapter because the USB connector of the dongle does not carry USB signals, instead they carry RS232 rx/tx signals, or you can use an USB-UART adapter wired to the onboard header where the rx/tx and ground is marked accordingly.

After that you can use the OTA and Arduino IDE for further updates.

![first-time-upload](https://raw.githubusercontent.com/MorgothCreator/GrowattMonitor-ShineWiFi-F/main/assets/ShineWiFi-F_Board.jpeg)

#TODO :
* Implementing a webpage with history charts.
* Implementing a FTP server to manage log files.
* A management page.

Any new feature to implement? I will be glad to hear about your ideas.

