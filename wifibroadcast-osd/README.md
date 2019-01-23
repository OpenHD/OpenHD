# Wifibroadcast OSD
OSD for HD wireless FPV system based on wifibroadcast from befi

This project uses the openvg library to draw 2d objects onto the screen. It is an OSD that uses the telemetry of already existing systems like mavlink, frsky direct GPS and so on.

If some steps in setting up the osd is unclear visit the blog from befi and check if it helps:
https://befinitiv.wordpress.com/2015/07/06/telemetry-osd-for-wifibroadcast/

Most of the steps should be the same. **Requires latest wifibroadcast version**

# Telemetry Protocols

Currently supports Frsky D-Series Protocol and Light Telemetry (LTM). Mavlink and others might follow later.

# Possible issues
1) The AHI might indicate the opposite direction. This can be fixed by changing this part in render.c
```
//set to 1 or -1
#define INVERT_ROLL 1
#define INVERT_PITCH 1
```
-1 for reversed and 1 for normal.

2) Roll and pitch might be exchanged. So when roll angle changes it shows AHI change on pitch. This can be exchanged by uncommenting EXCHANGE_ROLL_AND_PITCH:
```
//#define EXCHANGE_ROLL_AND_PITCH
```
to
```
#define EXCHANGE_ROLL_AND_PITCH
```

3) The home arrow might move into the wrong direction. This can be fixed by changing this part in render.c
```
//set to 1 or -1
#define INVERT_HOME_ARROW 1
```

#Installation
1) increase split memory for GPU in case it is not already 128MB

```
sudo raspi-config
```
In advanced -> Memory Split -> 128MB


2) install requirements for openvg
```
sudo apt-get install libjpeg8-dev indent libfreetype6-dev ttf-dejavu-core
```

3) download and install modified openvg library (uses layer 1 instead of 0, default background is transparent, allows outlines on text)
```
cd
git clone https://github.com/SamuelBrucksch/openvg
cd openvg
make library
sudo make install
```

4) Download and compile osd source code
```
cd
git clone https://github.com/SamuelBrucksch/wifibroadcast_osd.git
cd wifibroadcast_osd
make
```

#Configuration
All current configuration values can be set in [osdconfig.h](https://github.com/SamuelBrucksch/wifibroadcast_osd/blob/master/osdconfig.h)

# Starting OSD
I uploaded my start scripts as a sample how to start wifibroadcast with 1 video and 1 telemetry stream. have a look at those and adapt your own start scripts based on that.

Telemetry serial of your flight control can directly be connected to the Raspi serial interface. TX of flight control needs to be connected to RX on Raspi (GPIO Pin 10). You need to disable the Linux Serial Console for this on the transmitter side. Start raspi-config and go advanced -> serial and disable. Later it will be possible to passthrough the telemetry on RX, then there you need to disable Serial as well, right now there is no need to disable Serial on the RX. This will be helpful if you want to pass the telemetry to an antenna tracker.

**The Raspi Serial expects TTL 3.3V Level, so do not use with any telemetry source that uses 5V TTL Level or even RS232.**
