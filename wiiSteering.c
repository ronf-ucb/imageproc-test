extern WiiBlob wiiBlobs[4]

typedef struct{
    int centroid[2];  //x,y
    int centroidNominal[2];
    int xDist;
    int yDist;
    int kTheta;
    int kV;
    int xDistNominal;
    int yDistNominal;
    char centroidFlag;
    char faultFlag;
    char enableFlag;
} wiiSteer;

wiiSteer wiiSteering;

void getWiiError(void){
    
    int temp;
    char blobCount = 0 , i;

    //Blob math
    for(i=0;i<4;i++){
        if(wiiSteering.size[i] |= 255){
            blobCount++;
            wiiSteering.centroid[0] += wiiBlobs[i].x;
            wiiSteering.centroid[1] += wiiBlobs[i].y;
        }
    }
    if(blobCount == 0){
        wiiSteering.faultFlag = 1;
    } else {
        wiiSteering.faultFlag = 0;
    }
    wiiSteering.centroid[0]=wiiSteering.centroid[0]/blobCount;
    wiiSteering.centroid[1]=wiiSteering.centroid[1]/blobCount;

    if(blobCount == 2){
       temp = wiiBlobs.[1].x - wiiBlobs.[0].x;
       centroidFlag = 0;
       
       if(temp<0){ // If right blob idx = 0
           wiiSteering.xDist = -temp;
           wiiSteering.yDist = wiiBlobs.[1].y - wiiBlobs.[0].y;
       } else {
           wiiSteering.xDist = temp;
           wiiSteering.yDist = wiiBlobs.[0].y - wiiBlobs.[1].y;
       }
   } else {
    // Centroid based wiiSteering
    centroidFlag = 1;
   }
}

void setWiiSteer(void){
    int v, omega;
    int interval[NUM_VELS], delta[NUM_VELS], vel[NUM_VELS];
    int idx = 0, i = 0;

    if(wiiSteering.centroidFlag == 0 && wiiSteering.faultFlag == 0){
        v = (wiiSteering.xDist - wiiSteering.xDistNominal)*wiiSteering.kV;
        omega = (wiiSteering.yDist - wiiSteering.yDistNominal)*wiiSteering.kTheta;
        LED_1 = 1; LED_2 = 0; LED_3 = 0;   
    } 
    if(wiiSteering.centroidFlag == 1 && wiiSteering.faultFlag == 0){ //Going to need gain divisor
        v = (wiiSteering.centroid[1] - wiiSteering.centroidNominal[1])*wiiSteering.kV;
        omega = (wiiSteering.centroid[0] - wiiSteering.centroidNominal[0])*wiiSteering.kTheta;
        LED_1 = 0; LED_2 = 1; LED_3 = 0; 
    }
    if(wiiSteering.faultFlag == 1){
        v = 0;
        omega = 0;
        LED_1 = 1; LED_2 = 1; LED_3 = 1;
    }

    //Set PID profile
    for(i = 0; i < NUM_VELS; i ++) {
        vel[i] = v + omega;
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
        vel[i] = v - omega;
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

}