### Vehicle Simulator part
<br>

Written in `C++` with `Qt5`.  
- Simulates electric vehicle charging process
- Sends data to Cloud Backend using either:
  - `Sigfox` - modem connected via UART, `AT protocol`
  - `WLAN` - (if direct connectivity is available), using `QtNetwork`
- GUI created using `QtWidgets`
- Separate components for all modules: *GUI*, *Simulator*, *Broadcaster*, *Logger*, all running in own `QThread`
<br>

Running on `Raspberry Pi 4B`.  
For communication frame format, see [format.txt](/format.txt).
