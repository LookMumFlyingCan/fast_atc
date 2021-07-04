#pragma once

struct telemetry {
  char type;
  char pad;
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
};

struct gps {
  char type;
  char pad0;

  unsigned short frameNum;
  char pad1;
  char pad2;
  char pad3;
  char pad4;

  double latitude;
  double longitude;
  double speed;
  double altitude;

  unsigned char timeHour;
  char pad5;
  unsigned char timeMin;
  char pad6;
  unsigned char timeSec;
  char pad7;

  unsigned char satNumber;
  char pad8;
};

struct sat_status {
  telemetry tel;
  gps pos;
};
