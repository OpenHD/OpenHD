# Developer information

## Preconditions

This service assumes a connection between the air and ground pi (bidirectional) setup by openhd-interface. Aka both air
and ground service send their data / listen on specific UDP localhost ports. However,if they are wired up to a
wifibroadcast instance (and if the link works) the service cannot and does not need to know. The rx/tx ports are set up
in such a way that if both the air and ground service run on the same machine, they talk to each other via UDP. Aka the
UDP port the air unit sends data to is the same UDP port the ground unit listens on, and vice versa. This way one can
easily debug this module not running on OpenHD.

## Routing

Routing: I recommend reading this first: https://github.com/mavlink-router/mavlink-router
In generall, this module implements the following 2 basic functionalities:

1) Provide a bidirectional connection from the ground pi directly to the flight controller. This way the telemetry data
   from the FC connected to the Air pi makes it via wifibroadcast to the ground pi and can be distributed to the ground
   station(s) connected to the ground pi. However, the ground station(s) connected to the OHD Ground pi can also talk to
   the FC. This way, one can change settings on the FC itself (via QGroundControl, for example). NOTE: For this to work,
   we must keep this module such that QGroundControll (for example) doesn't even know it is not directly connected to
   the FC, but rather connected to the FC with OpenHD in between.

2) Provide a bidirectional connection between the ground pi and the air pi. This way the application connected to the
   ground station (like QOpenHD) can receive OpenHD-specific telemetry data (fire and forget) but also can change OHD
   Settings.

Using the functionalities above, this module should also handle ALL OpenHD settings with acknowledgements. If necessary,
We can have this module talk to another module (for example a camera/video) module locally via mavlink. However, in my
opinion we should avoid this as much as possible - one single executable / code base makes development and especially
debugging much easier.

## System ID's

This module declares two mavlink sys IDs - one for the Air unit and one for the Ground unit. The routing does not
neccessarily follow the mavlink standarts, have a look at the implementation and fully read this readme for more
information.

## Component ID's

Some mavlink commands are handled by this module itself, some need to be handled by other components (for example video)
. If a message needs to be handled by another component, TODO implement some generic way to echange messages, ideally "
better" than TCP/UDP. For now, the general rule is: If comp_id == Self, the message is handled internally. Otherwise,
the message is forwarded to the corresponding component.

## Handling commands

If a command (for example reboot) does not have any special platform dependencies or side effects, it can (and SHOULD!)
be handled by this module itself. Otherwise, it might be forwarded to the proper module.

## Running / Executing

This module exists of 2 executables - one executable that runs on the OHD air pi / OHD air unit and one executable that
runs on the OHD grund pi / OHD ground unit.

## External (HW) systems

When you consider this module running on both the air and ground pi, it only needs to talk to the following 2 external
systems

- The Flight controller (connected via UART to the air pi) and the Ground Station (running or connected to the ground
  pi).

## Connecting the  Ground station application to this module running on the Ground Pi (QOpenHD or QGroundControl)

Rn, there is only one way to connect either QOpenHD or QGroundControl to his module running on the ground pi - a
bidirectional connection made up of 2 UDP ports. TODO: When QOpenHD / QGroundControl is not running on the ground
station itself, but rather on another device (for example a smartphone connected to the ground pi), another difficulty
arises from the necessity to route the messages over another network. For this, we probably should go with TCP, but the
TCPEndpoint still needs some work. The corresponding endpoint in QOpenHD can be found
here: https://github.com/OpenHD/QOpenHD/blob/consti-test/app/telemetry/OHDConnection.h

## Destination sys IDs

(Telemetry) Messages that only need to make their way from either the FC, the air pi or the ground pi to the Ground
clients (QOpenHD, QGroundControll) don't need a target sys id - they are routed through as long as they go in direction
towards the ground pi.

## Does this module need any settings / platform info:

Well, the amount of needed options - so to say - will probably increase with added features. So far there are:

1) UART -> the module needs to know the (file)-name of the UART for the serial connection to the flight controller, as
   well as the Baud rate, usw.
