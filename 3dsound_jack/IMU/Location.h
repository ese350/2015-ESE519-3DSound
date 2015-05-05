/*
 * Header file to extract the angle of head in the yaw axis.
 * Angle is given by the function getAzimuth()
 * It utilizes sensor.h which is a function utilised from open source library by Mark Williams to find out the 
 * Acceleration and gyroscopic values from LSM 303.
 */
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <ctime>
#include "sensor.h"
#define X   0
#define Y   1
#define Z   2

#define DT 0.02         // [s/loop] loop period. 20ms                                                                                                                                                     
#define AA 0.98         // complementary filter constant                                                                                                                                                  
#define A_GAIN 0.0573      // [deg/LSB]

#define G_GAIN 0.070     // [deg/s/LSB]
#define RAD_TO_DEG 57.29578
#define M_PI 3.14159265358979323846
                                                                                                                                                     

int  *Pacc_raw;
int  *Pmag_raw;
int  *Pgyr_raw;

int  acc_raw[3];
int  mag_raw[3];
int  gyr_raw[3];

float accXnorm,accYnorm,pitch,roll,heading,magXcomp,magYcomp;
int  startInt = 0;
class Location {
private:	
  float heading;
  void compute();
  void init();
public:
  int  getAzimuth();
  float getElevation();
  Location();
};

void  INThandler(int sig)
{
  signal(sig, SIG_IGN);
  exit(0);
}


Location::Location() {
  signal(SIGINT, INThandler);
  enableIMU();
  init();
}

int
Location::getAzimuth(){
  compute();
  int result = (int) heading;
  return result;
}
float 
Location::getElevation(){
  return 0;
}

void 
Location::init(){
  acc_raw[3] = {0};
  mag_raw[3] = {0};
  gyr_raw[3] = {0};
                                            
  Pacc_raw = acc_raw;
  Pmag_raw = mag_raw;
  Pgyr_raw = gyr_raw;
}

void 
Location::compute() {
  readMAG(Pmag_raw);
  readACC(Pacc_raw);
  readGYR(Pgyr_raw);
                                                                                                                                                             
                                                                                                                                                                                                     
  heading = 180 * atan2(mag_raw[1],mag_raw[0])/M_PI;
  if(heading < 0)
    heading += 360;

  //Normalize accelerometer raw values.
  accXnorm = acc_raw[0]/sqrt(acc_raw[0] * acc_raw[0] + acc_raw[1] * acc_raw[1] + acc_raw[2] * acc_raw[2]);
  accYnorm = acc_raw[1]/sqrt(acc_raw[0] * acc_raw[0] + acc_raw[1] * acc_raw[1] + acc_raw[2] * acc_raw[2]);

  //Calculate pitch and roll
  pitch = asin(accXnorm);
  roll = -asin(accYnorm/cos(pitch));

  //Calculate the new tilt compensated values
  magXcomp = mag_raw[0]*cos(pitch)+mag_raw[02]*sin(pitch);
  magYcomp = mag_raw[0]*sin(roll)*sin(pitch)+mag_raw[1]*cos(roll)-mag_raw[2]*sin(roll)*cos(pitch);


  //Calculate heading
  heading = 180*atan2(magYcomp,magXcomp)/M_PI;

  //Convert heading to 0 - 360
  if(heading < 0)
    heading += 360;
}

