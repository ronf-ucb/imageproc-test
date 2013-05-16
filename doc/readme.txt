May 14, 2013
Duncan's Iptest-wii

1) python com9-> com4
2) python: DEST_ADDR
3) radio_settings.h CHAN, PANID, etc
3.1) switch radio_settings.h to use Motile + 0x2052

#define RADIO_MY_CHAN 0x0e
#define RADIO_PAN_ID 0x3000
//Hard code in destination address (basestation) for now, update to be dynamic later
#define RADIO_DEST_ADDR 0x3001
#define RADIO_SRC_ADDR 0x2052


4) motor driver B: sign change from Duncan's robot
PIDStartMotors 0x81

l: set linear an rotational velocity
p: turn on PID
<sp>: off

SetDiffSteer should be initialized in setGain

Wii camera needs to be connected to run properly

try commenting out wii stuff in pid-ip2.5


