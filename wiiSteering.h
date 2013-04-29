
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

void getWiiError();
void setWiiSteer();

