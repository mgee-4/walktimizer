# walktimizer
A project inspired from a fact from a Physics for the Birds video about why humans like music or something. A general simplification of the the mechanics of walking is that of the inverted pendulum. From many introductory physics classes, we learn about the resonant frequency of pendulums. Combining this with my enjoyment of going outside and walking, I thought it would be fun to make a device that tracks optimal walking pace based on the length of our pendulum modelled legs. Additionally, I haven't had the chance to make many silly or just for fun projects in school, part of this is to scratch that itch.

Components Used:
- BN-220 GPS Module
- I2C OLED Display
- Arduino Micro
  - Arduino Nano 33 BLE (Retired, I accidently broke the USB port off 🤦. I'll resolder a new port one day!)

Implemented:
- Speed tracker with a GPS module
- Speed efficiency tracking compared to a hard coded test value
- Information printing to a display

To Implement:
- A state machine to allow switching between display mode and user paramter update mode
- A mathematical model to calculate an ideal speed based on user parameters
