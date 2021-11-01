---
title:  "Vehicle Simulator"
permalink: /docs/vehicle-simulator/
---

The first component of the system is the simulator of electric-vehicle, respectively, of its **telematics unit** (<abbr title="Telematics Control Unit">TCU</abbr>), which is built on `Raspberry Pi 4B`. The microcomputer is housed in a box with a small touchscreen, inside which is also the Sigfox communication module connected to the `UART` interface, including a small antenna, and `BMP280` temperature sensor, connected to the `I2C` bus.

[fotka simulatora]

The `Raspberry Pi OS` runs an application written in `C++` and the `Qt framework`, using which the GUI for simulator control is created.  Internally, the application consists from 4 components:
- **GUI**
- **Simulator**
- **Braodacaster**
- **Logger**

*Simulator* and *Broadcaster* components both run in their own `QThread`s, so they can be enabled/disabled independently from each other, for example for testing purposes, and do not interfere with each other. The exchange of data is realized through a mutual *Vehicle* object, to which all the components have a reference. For all read/write operations to *Vehicle* attributes, Qt’s `QReadWriteLock` mechanism is used, to avoid race conditions. The components run independently from each other, but if there is a condition when some action (redraw UI, broadcast, write a log immediately) is needed, Qt’s `Signals and Slots` mechanism is used to notify each other of these events (vehicle state was manually changed, charging has completed, etc.). 

GUI consists of a few simple elements and is designed for small touch display - user can change actual charge level, charging current, vehicle state (*off, idle, charging, driving*), and see actual values and logs written to “console” (all the `cout` outputs are redirected there). 

The application is executed using a `systemd` service upon system boot, and is launched in a fullscreen mode.

### Simulation
The simulation process is made by mathematical logarithmic equations which imitate the process of real accumulator charging - from fast increasing in a lower charge levels, to very slow increasing close to full charge, also depending on the charging current. As the reference, the charging curve of the *Tesla Model S* with 85kWh battery was used, on a conventional “slow” charger with 50A DC current and on *Tesla Supercharger* stations, with 300A DC current. Equation parameters for currents between these values are calculated in a linear scale.

### Broadcasting
The “broadcasting” to the cloud backend is possible by 2 ways: 
- through <b><u>WLAN</u></b> - standard HTTP calls to backend API, over Raspberry’s network connectivity.
- through <b><u><a href="/docs/sigfox">Sigfox</a></u></b> technology - modern IoT, LPWAN 
network, of which network is available on wide areas worldwide.

The idea behind this is, that WLAN connectivity can be used when the vehicle is located at the brand’s own or partners charging stations, where the vehicle is able to automatically connect to a <u>secure</u>, local Wi-Fi network (be aware that *Wi-Fi* doesn't have to be only 802.11b/g/n! ([see wikipedia](https://en.wikipedia.org/wiki/IEEE_802.11))). The other scenario is when the owner is charging his vehicle at home.  
In other cases, when a local network is not available, Sigfox is used. In this case, the message is compressed into a 12B long packet (this is the maximum length for Sigfox payload) using bit-shifts and converting `floats`  into `half` data type. Using this, we can fit 9 values into this 12 bytes long packet. 
The content of the message is like this:

<table style="width:70%; height: 140px; margin-left:5%; font-size:13px;">
	<colgroup>
		<col width="30%" />
		<col width="30%" />
		<col width="20%" />
		<col width="20%" />
	</colgroup>
	<thead>
		<tr class="header">
			<th>Attribute:</th>
			<th style="text-align:center">Length:</th>
			<th>Data type:</th>
			<th>Unit:</th>
		</tr>
	</thead>
	<tbody>
		<tr>
			<td>• state</td>
			<td style="text-align:center">(3 bits)</td>
			<td>enum</td>
			<td>-</td>
		</tr>
		<tr>
			<td>• current charge</td>
			<td style="text-align:center">(7 bits)</td>
			<td>uint8</td>
			<td>perc</td>
		</tr>
		<tr>
			<td>• target charge</td>
			<td style="text-align:center">(7 bits)</td>
			<td>uint8</td>
			<td>perc</td>
		</tr>
		<tr>
			<td>• current</td>
			<td style="text-align:center">(10 bits)</td>
			<td>uint16</td>
			<td>amps</td>
		</tr>
		<tr>
			<td>• elapsed time</td>
			<td style="text-align:center">(13 bits)</td>
			<td>uint16</td>
			<td>minutes</td>
		</tr>
		<tr>
			<td>• remaining time</td>
			<td style="text-align:center">(13 bits)</td>
			<td>uint16</td>
			<td>minutes</td>
		</tr>
		<tr>
			<td>• current range</td>
			<td style="text-align:center">(11 bits)</td>
			<td>uint16</td>
			<td>km</td>
		</tr>
		<tr>
			<td>• outside temperature</td>
			<td style="text-align:center">(16 bits)</td>
			<td>half</td>
			<td>minutes</td>
		</tr>
		<tr>
			<td>• inside temperature</td>
			<td style="text-align:center">(16 bits)</td>
			<td>half</td>
			<td>minutes</td>
		</tr>
		<tr>
			<td></td>
			<td style="text-align:center"><b>96 bits = <u>12 bytes</u></b></td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
		</tr>
	</tbody>
</table>

This is not all the parameters which application simulates/vehicle’s telematics unit can provide. Rest of them (*location coordinates, desired temperature set for AC, maximal charging current, vehicle’s consumption, etc.*) can be send as another packet with “extended” values, or can be send only via Wi-Fi connectivity. This part is not finished and the solution is opened.
After preparing the packet, it is sent to the serial port, where the communication module is connected. The module is controlled and configured using `AT commands`.

It’s needed to keep in mind, that Sigfox has a limitation in number of messages that can be sent from one device in one day, so also the broadcasting interval should to be different. In case of Wi-Fi connectivity available, the actual status can be reported to the cloud on a second basis. This configuration can be set in macros of component’s header files. 

In case of “broadcasting” via Wi-Fi, the application can evaluate the success of process; in case of Sigfox broadcasting, there is no acknowledgment from network possible. The availability of Wi-Fi, Sigfox, and possibly also some other connectivity is evaluated before each transmission, so the simulator/TCU can always make the decession for using the best suitable channel for each transmission. 
Each Sigfox and W-Fi broadcasting “modules” are made as individual classes, so it’s easy to replace them/add other communication technology, if it would be suitable. 

See [Communication Technologies](/docs/communication-technologies/) article for more.

