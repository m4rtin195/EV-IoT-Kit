---
title:  "Vehicle Simulator"
permalink: /docs/vehicle-simulator/
---

The first component of the system is the simulator of electric-vehicle, respectively, of its telematic unit (TCU - Telematic Control Unit), which is built on Raspberry Pi 4B. The microcomputer is housed in a box with small tocuhscreen, inside which is also the Sigfox communication module connected to UART interface, including small antena, and BMP280 temperature sensor, connected to I2C bus.

[fotka simulatora]

The Raspberry Pi OS runs an application written in C++ and the Qt framework, using which the GUI for simulator control is created.  Internally, the application consists from 4 components:
- `GUI`
- `Simulator`
- `Braodacaster`
- `Logger`

`Simulator` and `Broadcaster` components both runs in their own threads, so they can be enabled/disabled independently from each other, for example for testing purposes, and do not interfere each other. The exchange of data are realized through common `Vehicle` object, which all the components have reference to. For all read/write operations to `Vehicle` ’s attributes, Qt’s *QReadWriteLock* mechanism is used, to avoid race conditions. The components runs independently from each other, but if there is condition when action (redraw UI, broadcast, write log immidiately) is needed, the Qt’s *Signals and Slots* mechanism is used to notify each other on this events (vehicle state was manually changed, charging has completed, etc.). 
GUI consits of few simple elements and is designed for small touch display - user can change actual charge level, charging current, vehicle state (off, idle, charging, driving), and see actual values and logs written to “console” (all the `cout` outputs are redirected there). 

The application is executed as a `systemd` service upon system boot, and lunched in fullscreen mode.

## Simulation
The simulation process is made by mathematical logarithmic equations which imitates the process of real acumulator charging - from fast increase in lower charge levels to very slow increase close to full charge and depends on the set charging current. As the reference, the charging curve of Tesla Model S with 85kWh battery was used, on conventional “slow” charger with 50A current and on Tesla Supercharger stations, with 300A current. Equation parameters for currents between are calculated in a linear scale.

## Broadcasting
The “broadcasting” to cloud backend is possible by ways 
- through `Wi-Fi` mode - standard HTTP calls to backend API, over Raspberry’s network connectivity.
- through `Sigfox` technology ~(link na article)~ - modern IoT, LPWAN 
network, of which network is available on wide areas worldwide.

The idea behind this is, that Wi-Fi connectivity can be used when vehicle is charging on brand’s own or parner charging stations, where the vehicle is able to automatically connect to secure, local Wi-Fi network (be aware that W-Fi is not only 802.11b/g/n! ( ~link na wiki~ )) . The other scenario is, when owner is charging the vehicle at home. 
On other cases, when local network is not available, the Sigfox is used. In this case, the message is compressed into 12B long packet (this is maximum length for Sigfox payload) using bit-shifts and converting `floats`  into `half` data type. Using this, we can fit 9 values into this 12 bytes long packet. 
The content of message is like this:

| Attribute: | Length: | Data type: | Unit: |
|---|---|---|---|
| - state | (3 bits) | enum | - |
| - current charge | (7 bits) | uint8 | perc |
| - target charge | (7 bits) | uint8 | perc |
| - current | (10 bits) | uint16 | amps |
| - elapsed time | (13 bits) | uint16 | minutes |
| - remaining time | (13 bits) | uint16 | minutes |
| - current range | (11 bits) | uint16 | km |
| - outside temperature | (16 bits) | half | minutes |
| - inside temperature | (16 bits) | half | minutes |
| | 96 bits | | |
| | =12 bytes | | |

This is not all the parameters which application simulates/vehicle’s telematic unit can provide. Rest of them (location coordinates, desired temperature set for AC, maximal charging current, vehicle’s consumption, etc.) can be send as another packet with “extended” values, or can be send only via Wi-Fi connectivity. This part is not finished and the solution is opened.
After preparing the packet, it is sent to the serial port, where the communication module is connected. The module is controlled and configured using AT commands.

It’s needed to keep in mind, that Sigfox has a limitation in number of messages that can be sent from one device in one day, so also the broadcasting interval should to be different. In case of Wi-Fi connectivity available, the actual status can be reported to the cloud on a second basis. This configuration can be set in macros of component’s header files. 

In case of “broadcasting” via Wi-Fi, the application can evaluate the success of process; in case of Sigfox broadcasting, there is no acknowledgment from network possible. The availability of Wi-Fi, Sigfox, and possibly also some other connectivity is evaluated before each transmission, so the simulator/TCU can always make the decession for using the best suitable channel for each transmission. 
Each Sigfox and W-Fi broadcasting “modules” are made as individual classes, so it’s easy to replace them/add other communication technology, if it would be suitable. 

See ~LPWAN technologies~ post for more.

