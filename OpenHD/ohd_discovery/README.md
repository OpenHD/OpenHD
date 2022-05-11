
The intended use case of this module is to handle detection of hw that does not change during run time.
Members should adhere to the following paradigms:
1) After the discovery step, we developers assume that the connected hardware does not change.
2) HW Capabilities are written out to json, and then can be easily accessed by other modules.
    NOTE: This might change, we could also just as well not use json for that and do it all in code with const Structs.

See inc/OHDSystem.h for more info

## Old notes
I think stephens idea here was to run this service at startup, and it figures out the platform with
its connected devices (wifi, ethernet, camera(s)) and writes this information into json files.

In my opinion, there are Really good ideas here:
1) This service runs at startup. After that, all other services can be started.
2) This service only generates data, that then MUST NOT be changed / modified.
   This means this service is easily to maintain and debug. - for debugging, just look at the gernerated data.
3) By generating data and writing it easily accessible for other modules to read, it is easy to understand - especially
 if we follow the paradigma that this service is NOT responsible for changing or storing settings, but only exposing
 the hardware and its capabilities.

How exactly all this stuff here works should be documented though.