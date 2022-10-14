# GrowattMonitor-ShineWiFi-F

 ShineWiFi-F non official firmware replacement for monitoring Growatt SPF3500/5000ES

This is a non official replacement firmware for Growatt ShineWiFi-F dongle for those that don't want to use cloud storage and monitor the inverter by themselves.

The firmware is developed using Arduino IDE and has implemented an OTA firmware updater for easy update after first firmware writing.

This project is dependent of below libraries and boards:
* Generic ESP8266 Module
* arduFPGA-app-common-arduino by Iulian Gheorghiu
* Rtc_Pcf8563 by Joe Robertson
* NTPClient by Fabrice Weinberg

At this moment is able to report the inverter status in realtime using a browser pointed to the dongle IP.

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

If user want to read the dongle data through the internet, a HTTPS server can be used using changing "#define USE_HTTPS false" to "#define USE_HTTPS true" with the downside of the page access being slower, around 1/2 seconds, and in this case you need to change the certificate located in "crt.h" file, both private key and public key, at this moment an authentication method is not implemented.

To read the log content the user can enter "http://dongle-ip/report", there the dongle will report what the dongle has done from last start/restart in a 4096 character buffer.

The time of the onboard RTC is automatically updated every start/restart of the dongle is the dongle has access to the internet, the timezone can be set changing "timeClient.setTimeOffset(3*3600);" in "main.cpp" file, the 3 represent the hours to add or if is negative to subtract from UTC time.


In order to upload for the first time the firmware you need to power up the dongle with the two pins marked as boot shorted, after that you can use the usb connector of the dongle and a USB-RS232 adapter because the USB connector of the dongle toes not carry USB signals, instead they carry RS232 rx/tx signals, or you can use an USB-UART adapter wired to the onboard header where the rx/tx and ground is marked accordingly.

After that you can use the OTA for further updates.

![first-time-upload](https://raw.githubusercontent.com/MorgothCreator/GrowattMonitor-ShineWiFi-F/main/assets/ShineWiFi-F_Board.jpeg)

![page](https://raw.githubusercontent.com/MorgothCreator/GrowattMonitor-ShineWiFi-F/main/assets/screenshot.png)

