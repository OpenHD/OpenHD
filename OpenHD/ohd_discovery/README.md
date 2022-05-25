The intended use case of this module is to handle detection of hw that does not change during run time. Members should
adhere to the following paradigms:

1) After the discovery step, we developers assume that the connected hardware does not change.
2) HW Capabilities are written out to json, and then can be easily accessed by other modules. NOTE: This might change,
   we could also just as well not use json for that and do it all in code with const Structs.

See inc/OHDDiscovery.h for more info

## NOTE:
We should never introduce any dynamically changed settings here, since the purpose of this module is to create
deterministic results (on each re-start, if the hardware does not change, the output of this module does not change).

## Dev info:
If you need to debug this module, instead of re-starting OpenHD you can also just run test/test_discovery.cpp