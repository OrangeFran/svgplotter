// Code to use plotter with joystick
// From Ivo Blöchliger - KSBG

#include <Arduino.h>
#include "joystick.h"
#include "move.h"

int joyPins[3] = {34,35,26};

long int last = 0;
long int lastMove = 0;
long int maxdelay = 5000;
long int stepdelay = maxdelay;
int stepnum = 1;

void penDown();
void PenUp();

int dirs[] = {0,0};
int shorter[] = {0,1};
void setDir(int stepper, int dir) {
  if (dir==-1) dir=0;
  dirs[stepper] = dir;
  digitalWrite(dirPins[stepper], dir==1 ? shorter[stepper]:1-shorter[stepper]);
}
void setDirs(int d0, int d1) {
  setDir(0, d0);
  setDir(1, d1);
}

void joystick() {
  int a[2];
  a[0] = analogRead(joyPins[0]);
  a[1] = analogRead(joyPins[1]);
  // int sw = digitalRead(joyPins[2]);
  
  int vx=(a[1]<500 ? -1 : (a[1]>3500 ? 1 : 0));
  int vy=(a[0]<500 ? -1 : (a[0]>3500 ? 1 : 0));

  int m0 = vx-vy;
  int m1 = -vx-vy;
  if (m0!=0 || m1!=0) {
    if (millis()-lastMove<2) {
      if (stepdelay>100) {
        stepdelay = maxdelay*(sqrt(stepnum+1)-sqrt(stepnum));
        stepnum++;
      }
    } else {
      stepdelay=maxdelay;
      stepnum = 1;
    }
    delayMicroseconds(stepdelay);
    lastMove=millis();
  }
  setDirs(m0<0 ? 1:0, m1<0 ? 1:0);
  
  if (m0!=0) {
    step(0);
  }
  if (m1!=0) {
    step(1);
  }
}
