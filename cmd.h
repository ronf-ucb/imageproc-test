#ifndef __CMD_H
#define __CMD_H


#include "mac_packet.h"
#include "cmd_const.h"

#define CMD_TEST_RADIO          0x00
#define CMD_TEST_MPU            0x06
#define CMD_SET_THRUST_OPENLOOP 0x80
#define CMD_PID_START_MOTORS    0x81
#define CMD_SET_PID_GAINS       0x82
#define	CMD_PID_TELEMETRY       0x83
#define CMD_GET_AMS_POS         0x84
#define CMD_GET_WII_BLOBS       0x85
#define CMD_SET_WII_SENSE       0x86
#define CMD_SET_VEL_PROFILE     0x8C
#define CMD_WHO_AM_I            0x8D
#define CMD_PID_STOP_MOTORS     0x8E
#define CMD_ZERO_POS            0x8F
#define CMD_ENABLE_WII_STEER    0x90
#define CMD_SET_WII_GAINS       0x91
#define CMD_SET_WII_POSITION    0x92
#define CMD_STOP_WII_STEER      0x93

void cmdSetup(void);
void cmdPushFunc(MacPacket rx_packet);

#endif // __CMD_H
