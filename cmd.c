#include "cmd.h"
#include "cmd_const.h"
#include "dfmem.h"
#include "utils.h"
#include "ports.h"
#include "sclock.h"
#include "led.h"
#include "blink.h"
#include "payload.h"
#include "mac_packet.h"
#include "dfmem.h"
#include "radio.h"
#include "dfmem.h"
#include "tests.h"
#include "version.h"
#include "radio_settings.h"
#include "timer.h"
#include "tih.h"
#include "pid-ip2.5.h"
#include "ams-enc.h"
#include "carray.h"
#include "wii.h"
#include "wiiSteering.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned char (*cmd_func[MAX_CMD_FUNC])(unsigned char, unsigned char, unsigned char, unsigned char*);
void cmdError(void);

extern pidPos pidObjs[NUM_PIDS];
extern EncObj encPos[NUM_ENC];
extern wiiSteer wiiSteering;
extern volatile CircArray fun_queue;


/*-----------------------------------------------------------------------------
 *          Declaration of static functions
-----------------------------------------------------------------------------*/
static unsigned char cmdNop(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdWhoAmI(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdGetAMSPos(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);

//Wii Commands
static unsigned char cmdGetWiiBlobs(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdSetWiiSensitivity(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdEnableWiiSteering(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdStopWiiSteering(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdSetWiiGains(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdSetWiiPosition(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);

//Motor and PID functions
static unsigned char cmdSetThrustOpenLoop(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdSetPIDGains(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdPIDStartMotors(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdPIDStopMotors(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdSetVelProfile(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);
static unsigned char cmdZeroPos(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame);

/*-----------------------------------------------------------------------------
 *          Public functions
-----------------------------------------------------------------------------*/
void cmdSetup(void) {

    unsigned int i;

    // initialize the array of func pointers with Nop()
    for(i = 0; i < MAX_CMD_FUNC; ++i) {
        cmd_func[i] = &cmdNop;
    }
    cmd_func[CMD_TEST_RADIO] = &test_radio;
    cmd_func[CMD_TEST_MPU] = &test_mpu;
    cmd_func[CMD_SET_THRUST_OPENLOOP] = &cmdSetThrustOpenLoop;
    cmd_func[CMD_SET_PID_GAINS] = &cmdSetPIDGains;
    cmd_func[CMD_PID_START_MOTORS] = &cmdPIDStartMotors;
    cmd_func[CMD_PID_STOP_MOTORS] = &cmdPIDStopMotors;
    cmd_func[CMD_SET_VEL_PROFILE] = &cmdSetVelProfile;
    cmd_func[CMD_ZERO_POS] = &cmdZeroPos;
    cmd_func[CMD_WHO_AM_I] = &cmdWhoAmI;
    cmd_func[CMD_GET_AMS_POS] = &cmdGetAMSPos;
    cmd_func[CMD_GET_WII_BLOBS] = &cmdGetWiiBlobs;
    cmd_func[CMD_SET_WII_SENSE] = &cmdSetWiiSensitivity;
    cmd_func[CMD_ENABLE_WII_STEER] = &cmdEnableWiiSteering;
    cmd_func[CMD_SET_WII_GAINS] = &cmdSetWiiGains;
    cmd_func[CMD_SET_WII_POSITION] = &cmdSetWiiPosition;
    cmd_func[CMD_STOP_WII_STEER] = &cmdStopWiiSteering;

}

void cmdPushFunc(MacPacket rx_packet) {
    Payload rx_payload;
    unsigned char command;

    rx_payload = macGetPayload(rx_packet);
    if(rx_payload != NULL) {
        command = payGetType(rx_payload);

        if(command < MAX_CMD_FUNC && cmd_func[command] != NULL) {
            rx_payload->test = cmd_func[command];
            carrayAddTail(fun_queue, rx_packet);
        } else {
            cmdError();   // halt on error - could also just ignore....
        }
    }
}

// send robot info when queried
unsigned char cmdWhoAmI(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame) {
    unsigned char i, string_length; unsigned char *version_string;
    // maximum string length to avoid packet size limit
    version_string = (unsigned char *)"DHALDANE_VRoACH;PID-HARD;STEER-HARD: Tue Feb 12 14:04:47 2013";
    i = 0;
    while((i < 127) && version_string[i] != '\0') {
        i++;
    }
    string_length=i;
    radioSendData(RADIO_DEST_ADDR, status, CMD_WHO_AM_I,
            string_length, version_string, 0);
    return 1; //success
}

unsigned char cmdGetAMSPos(unsigned char type, unsigned char status,
        unsigned char length, unsigned char *frame) {
    long motor_count[2];
    motor_count[0] = pidObjs[0].p_state;
    motor_count[1] = pidObjs[1].p_state;

    radioSendData(RADIO_DEST_ADDR, status, CMD_GET_AMS_POS,
            sizeof(motor_count), (unsigned char *)motor_count, 0);
    return 1;
}

unsigned char cmdGetWiiBlobs(unsigned char type, unsigned char status,
        unsigned char length, unsigned char *frame){
    WiiBlob blobs[4];
    wiiStartAsyncRead(); //Testing
    delay_ms(5);
    wiiGetData(blobs);
    radioSendData(RADIO_DEST_ADDR, status, CMD_GET_WII_BLOBS,
            sizeof(blobs), (unsigned char *)blobs, 0);
    return 1;
}

// Commands: enable, set gains, calibrate position
unsigned char cmdEnableWiiSteering(unsigned char type, unsigned char status,
        unsigned char length, unsigned char *frame){
    wiiSteering.enableFlag = 1;
    pidObjs[0].onoff = 1;
    pidObjs[1].onoff = 1;
    return 1;
}

unsigned char cmdStopWiiSteering(unsigned char type, unsigned char status,
        unsigned char length, unsigned char *frame){
    wiiSteering.enableFlag = 0;
    pidObjs[0].onoff = 0;
    pidObjs[1].onoff = 0;
    return 1;
}

unsigned char cmdSetWiiGains(unsigned char type, unsigned char status,
        unsigned char length, unsigned char *frame){
    int velGain = frame[0] + (frame[1] << 8);
    int thetaGain = frame[2] + (frame[3] << 8);
    wiiSteering.kV = velGain;
    wiiSteering.kTheta = thetaGain;
    return 1;
}

unsigned char cmdSetWiiPosition(unsigned char type, unsigned char status,
        unsigned char length, unsigned char *frame){
    getWiiError();
    wiiSteering.xDistNominal = wiiSteering.xDist;
    wiiSteering.yDistNominal = wiiSteering.yDist;
    wiiSteering.centroidNominal[0] = wiiSteering.centroid[0];
    wiiSteering.centroidNominal[1] = wiiSteering.centroid[1];
    return 1;
}


unsigned char cmdSetWiiSensitivity(unsigned char type, unsigned char status, 
        unsigned char length, unsigned char *frame){
    int sensitivity = frame[0];
    wiiSetupAdvance(sensitivity, 0x33);
    return 1;
}


// ==== Motor PID Commands ======================================================================================
// ================================================================================================================ 

unsigned char cmdSetThrustOpenLoop(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame) {
    int thrust1 = frame[0] + (frame[1] << 8);
    int thrust2 = frame[2] + (frame[3] << 8);
    unsigned int run_time_ms = frame[4] + (frame[5] << 8);

    DisableIntT1;	// since PID interrupt overwrites PWM values

    tiHSetDC(1, thrust1);
    tiHSetDC(2, thrust2);
    delay_ms(run_time_ms);
    tiHSetDC(1,0);
    tiHSetDC(2,0);

    EnableIntT1;
    return 1;
 } 

 unsigned char cmdSetPIDGains(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame) {
    int Kp, Ki, Kd, Kaw, ff;
    int idx = 0;

    Kp = frame[idx] + (frame[idx+1] << 8); idx+=2;
    Ki = frame[idx] + (frame[idx+1] << 8); idx+=2;
    Kd = frame[idx] + (frame[idx+1] << 8); idx+=2;
    Kaw = frame[idx] + (frame[idx+1] << 8); idx+=2;
    ff = frame[idx] + (frame[idx+1] << 8); idx+=2;
    pidSetGains(0,Kp,Ki,Kd,Kaw, ff);
    Kp = frame[idx] + (frame[idx+1] << 8); idx+=2;
    Ki = frame[idx] + (frame[idx+1] << 8); idx+=2;
    Kd = frame[idx] + (frame[idx+1] << 8); idx+=2;
    Kaw = frame[idx] + (frame[idx+1] << 8); idx+=2;
    ff = frame[idx] + (frame[idx+1] << 8); idx+=2;
    pidSetGains(1,Kp,Ki,Kd,Kaw, ff);

    //Send confirmation packet
    // WARNING: Will fail at high data throughput
    //radioConfirmationPacket(RADIO_DEST_ADDR, CMD_SET_PID_GAINS, status, 20, frame);
    return 1; //success
}

unsigned char cmdSetVelProfile(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame) {
    int interval[NUM_VELS], delta[NUM_VELS], vel[NUM_VELS];
    int idx = 0, i = 0;

    for(i = 0; i < NUM_VELS; i ++) {
        vel[i] = frame[idx]+ (frame[idx+1]<<8);
        if(vel[i]<0){
            delta[i] = -0x4000;   //hardcoded for now
            interval[i] = delta[i]/vel[i];
        } else if(vel[i]>0) {
            delta[i] = 0x4000;
            interval[i] = delta[i]/vel[i];
        } else {
            delta[i] = 0;
            interval[i] = 100;    //Fudge factor
        }

        idx+=2;
    }

    pidSetVelProfile(0, interval, delta, vel);

    for(i = 0; i < NUM_VELS; i ++) {
        vel[i] = frame[idx]+ (frame[idx+1]<<8);
        if(vel[i]<0){
            delta[i] = -0x4000;   //hardcoded for now
            interval[i] = delta[i]/vel[i];
        } else if(vel[i]>0) {
            delta[i] = 0x4000;
            interval[i] = delta[i]/vel[i];
        } else {
            delta[i] = 0;
            interval[i] = 100;    //Fudge factor
        }

        idx+=2;
    }
    pidSetVelProfile(1, interval, delta, vel);
    
    pidSetSync(frame[idx]);

    //Send confirmation packet
    // WARNING: Will fail at high data throughput
    //radioConfirmationPacket(RADIO_DEST_ADDR, CMD_SET_VEL_PROFILE, status, 48, frame);
    return 1; //success
}

unsigned char cmdPIDStartMotors(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame) {
    pidSetInput(0, 0);
    pidOn(0);
    pidSetInput(1, 0);
    pidOn(1);
    return 1;
}

unsigned char cmdPIDStopMotors(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame) {
    pidObjs[0].onoff = 0;
    pidObjs[1].onoff = 0;
    return 1;
}

unsigned char cmdZeroPos(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame) {
    long motor_count[2];
    motor_count[0] = pidObjs[0].p_state;
    motor_count[1] = pidObjs[1].p_state;

    radioSendData(RADIO_DEST_ADDR, status, CMD_GET_AMS_POS,
        sizeof(motor_count), (unsigned char *)motor_count, 0);
    pidZeroPos(0); pidZeroPos(1);
    return 1;
}

void cmdError() {
    int i;
    pidEmergencyStop();
    for(i= 0; i < 10; i++) {
        LED_1 ^= 1;
        delay_ms(200);
        LED_2 ^= 1;
        delay_ms(200);
        LED_3 ^= 1;
        delay_ms(200);
    }
}

static unsigned char cmdNop(unsigned char type, unsigned char status, unsigned char length, unsigned char *frame) {
    return 1;
}
