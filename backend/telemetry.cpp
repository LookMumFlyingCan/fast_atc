struct telemetry {
    char type;
    unsigned short frameNum;

    float tempPCB;
    float tempSDR;
    float tempENV1;
    float tempENV2;
    float tempRPI;

    double pressure;

    float accX;
    float accY;
    float accZ;

    float psu_Vbat;
    float psu_Abat;
    float psu_V5V;
    float psu_A5V;
    float psu_V3V3;
    float psu_A3V3;

    char rpiStatus[2];
    unsigned short flags;    
}
