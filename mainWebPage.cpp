#include <sys/stdio.h>
#include <sys/_intsup.h>

#include "mainWebPage.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "pv.h"

static const char MainWebPage[] PROGMEM = R"EOF(
<!DOCTYPE html>
<html>
<meta http-equiv="refresh" content="2">
<style>
table {
font-family: arial, sans-serif;
border-collapse: collapse;
width: 100%;
}
td, th {
border: 1px solid #dddddd;
text-align: left;
padding: 8px;
}
tr:nth-child(even) {
background-color: #dddddd;
}
/* Container for flexboxes */
section {
	display: -webkit-flex;
	display: flex;
}
/* Style the navigation menu */
nav {
	-webkit-flex: 1;
	-ms-flex: 1;
	flex: 1;
	background: #ccc;
	padding: 20px;
}
/* Style the list inside the menu */
nav ul {
	list-style-type: none;
	padding: 0;
}
.loadbar1 {
	width:200px;
	height:25px;
	background-color:#fff;
	border:1px solid #ccc;
	position:relative;
}
.bar1 {
	line-height:25px;
	width:100%%;
	display:block;
	font-family:arial;
	font-size:12px;
	background-color:green;
	color:#fff;
	position:absolute;
	bottom:0;
}
.loadbar2 {
	width:200px;
	height:25px;
	background-color:#fff;
	border:1px solid #ccc;
	position:relative;
}
.bar2 {
	line-height:25px;
	width:100%%;
	display:block;
	font-family:arial;
	font-size:12px;
	background-color:red;
	color:#fff;
	position:absolute;
	bottom:0;
}
</style>
<body>
<h2>Growatt SPF3500ES/SPF5000ES report</h2>
<h2>%s</h2>
<h4>Pannels share for powering the house, the rest is charging the battery.</h4>
<div class="loadbar1">
	<strong class="bar1" style='width:%.1f%%;'>%.1f%%</strong>
</div> Energy delivred from pannels
<h4>Battery share for powering the house.</h4>
<div class="loadbar2">
	<strong class="bar2" style='width:%.1f%%;'>%.1f%%</strong>
</div>
<h4>Total output power is: %d W, %dW from battery and %dW from pannels.</h4>
<h4></h4>
<section>
<nav>
	<ul>
		<li><a href="/">Home</a></li>
	</ul>
</nav>
<table style="width:100%%">
	<tr>
		<th>Essential information</th>
		<th></th>
		<th></th>
		<th></th>
		<th>Extra information</th>
		<th></th>
	</tr>
	<tr>
		<td>Grid voltage</td>
		<td>%.1fV</td>
		<td>PV Voltage</td>
		<td>%.1fV</td>
		<td>Grid discharge today</td>
		<td>%.1fKW</td>
	</tr>
	<tr>
		<td>Grid frequency</td>
		<td>%.2fHz</td>
		<td>PV Charge current</td>
		<td>%.2fA</td>
		<td>Grid discharge total</td>
		<td>%.1fKW</td>
	</tr>
	<tr>
		<td>Grid input power</td>
		<td>%dW</td>
		<td style="border-bottom:1px solid black">PV Power</td>
		<td style="border-bottom:1px solid black">%dW</td>
		<td>PV Production today</td>
		<td>%.1fKW</td>
	</tr>
	<tr>
		<td style="border-bottom:1px solid black">Grid charge current</td>
		<td style="border-bottom:1px solid black">%.2fA</td>
		<td>Output voltage</td>
		<td>%.2fV</td>
		<td>PV Production total</td>
		<td>%.1fKW</td>
	</tr>
	<tr>
		<td>Battery voltage</td>
		<td>%.2fV</td>
		<td>Output frequency</td>
		<td>%.2fHz</td>
		<td>Battery discharge today</td>
		<td>%.1fKW</td>
	</tr>
	<tr>
		<td>Battery capacity</td>
		<td>%d%%</td>
		<td>Output active power</td>
		<td>%dW</td>
		<td>Battery discharge total</td>
		<td>%.1fKW</td>
	</tr>
	<tr>
		<td>Charge current</td>
		<td>%.2fA</td>
		<td>Output aparent power</td>
		<td>%dW</td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td>Discharge current</td>
		<td>%.2fA</td>
		<td>Load percent</td>
		<td>%.1f%%</td>
		<td>Inverter temperature</td>
		<td>%.1f&#8451;</td>
	</tr>
	<tr>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td>DC-DC temperature</td>
		<td>%.1f&#8451;</td>
	</tr>
	<tr>
		<td></th>
		<td></th>
		<td></th>
		<td></th>
		<td></th>
		<td></th>
	</tr>
	<tr>
		<td>Error</td>
		<td>%s</td>
		<td>Warning</td>
		<td>%s</td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td>State</td>
		<td>%s</td>
		<td>Charge priority</td>
		<td>%s</td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<th>Rating information</th>
		<th></th>
		<th></th>
		<th></th>
		<th></th>
		<th></th>
	</tr>
	<tr>
		<td>Grid rated voltage</td>
		<td>%dV</td>
		<td>Output voltage</td>
		<td>%dV</td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td>Grid rated frequency</td>
		<td>%dHz</td>
		<td>Output frequency</td>
		<td>%dHz</td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td>Rated battery voltage</td>
		<td>%dV</td>
		<td>Output power</td>
		<td>%.0fW</td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td>Max charge current</td>
		<td>%dA</td>
		<td>AC Charge current</td>
		<td>%dA</td>
		<td></td>
		<td></td>
	</tr>
</table>
</section>
</body>
</html>
)EOF";


static const char MainPageJson[] PROGMEM = R"EOF(
HTTP/1.1 200 OK
Content-Type: json: charset=UTF-8
{
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
"Grid_rated_voltage":%d,
"Grid_rated_frequency":%d,
"Rated_battery_voltage":%d,
"Max_charge_current":%d,
"Rated_Output_voltage":%d,
"Rated_Output_frequency":%d,
"Rated_Output_power":%.0f,
"AC_Charge_current":%d
"Model":"%s"
"Serial_number":"%s"
"Equipment_mode":"%s"
"Inverter_CPU_version":"%s"
"MPPT_CPU_version":"%s"
"Date_of_manufacture":"%s"
}
)EOF";

char *MainWebPage_ComposeHtml() {
  size_t len = snprintf_P(nullptr, 0, MainWebPage,
		PowerSourceStatus, PV_PowerLoad, PV_PowerLoad, Battery_PowerLoad, Battery_PowerLoad, Output_active_power, BatteryPower, PV_Power,
		Grid_voltage, PV_Voltage, Grid_discharge_today,
		Grid_frequency, PV_Charge_current, Grid_discharge_total,
		Grid_input_power, PV_Power, PV_Production_today,
		Grid_charge_current, Output_voltage, PV_Production_total,
		Battery_voltage, Output_frequency, Battery_discharge_today,
		Battery_capacity, Output_active_power, Battery_discharge_total,
		Battery_charge_current, Output_aparent_power,
		Battery_discharge_current, Load_percentage, Inverter_temperature,
		DC_DC_temperature,
		Error, Warning,
		State, Charge_priority,
		Grid_rated_voltage, Rated_Output_voltage,
		Grid_rated_frequency, Rated_Output_frequency,
		Rated_battery_voltage, Rated_Output_power,
		Max_charge_current, AC_Charge_current);
  char *buff = new char[len + 1];
  if(!buff)
    return nullptr;
	snprintf_P(buff, len, MainWebPage,
    //WEB_PAGE_COMMON,
		PowerSourceStatus, PV_PowerLoad, PV_PowerLoad, Battery_PowerLoad, Battery_PowerLoad, Output_active_power, BatteryPower, PV_Power,
		Grid_voltage, PV_Voltage, Grid_discharge_today,
		Grid_frequency, PV_Charge_current, Grid_discharge_total,
		Grid_input_power, PV_Power, PV_Production_today,
		Grid_charge_current, Output_voltage, PV_Production_total,
		Battery_voltage, Output_frequency, Battery_discharge_today,
		Battery_capacity, Output_active_power, Battery_discharge_total,
		Battery_charge_current, Output_aparent_power,
		Battery_discharge_current, Load_percentage, Inverter_temperature,
		DC_DC_temperature,
		Error, Warning,
		State, Charge_priority,
		Grid_rated_voltage, Rated_Output_voltage,
		Grid_rated_frequency, Rated_Output_frequency,
		Rated_battery_voltage, Rated_Output_power,
		Max_charge_current, AC_Charge_current);
    return buff;
}

void MainWebPage_ComposeJson(char *buffer, int bufferSize) {
	 snprintf(buffer, bufferSize, MainPageJson, 
		Grid_voltage, Grid_frequency, Grid_input_power, Grid_charge_current,
		PV_Voltage, PV_Charge_current, PV_Power,
		Battery_voltage,Battery_capacity, Battery_charge_current, Battery_discharge_current,
		Output_voltage, Output_frequency, Output_active_power, Output_aparent_power, Load_percentage,
		Grid_discharge_today, Grid_discharge_total, PV_Production_today, PV_Production_total, Battery_discharge_today, Battery_discharge_total,
		Inverter_temperature, DC_DC_temperature,
		Error, Warning,
		State, Charge_priority,
		Grid_rated_voltage, Grid_rated_frequency, Rated_battery_voltage, Max_charge_current, Rated_Output_voltage, Rated_Output_frequency, Rated_Output_power, AC_Charge_current,
		Model, Serial_number, Equipment_mode, Inverter_CPU_version, MPPT_CPU_version, Date_of_manufacture);
}