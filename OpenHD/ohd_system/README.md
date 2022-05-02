This one is a bit more complex.

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