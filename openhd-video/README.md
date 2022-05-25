I think this module needs to be (re-written) just as mavlink telemetry, it is too complex in my opinion.

Why is it too complex: It breaks the biggest and most usefully paradigm of our old OpenHD releases:

Even when there is only a downlink from the Air pi to the ground pi, as long as enough packets are received, you
get video. Stephens approach was to do it with a mutable service on both the ground and air pi, breaking this paradigm.

My approach would be to do it with a mutable service on the air pi (that exposes changing the video resolution usw) but
an immutable service on the ground pi which responds to changes embedded in the downlink data.
This way, the video ground pi service never has to talk to the air pi video service.

Related to https://github.com/OpenHD/Open.HD/issues/623

However, we probably can re-use a good amount of code.