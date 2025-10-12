#include <Servo.h>

const int sp1 = 3;
const int sp2 = 4;
const int ut1 = 6;
const int ue1 = 7;
const int ut2 = 9;
const int ue2 = 8;

Servo s1;
Servo s2;
int a1 = 0;
int a2 = 120;

float dis1;
float dis2;

const int numSamples = 3;
const float graphConstant = 1.4;
const long maxTaps = 10;

float tapAngle = 60;
unsigned long lastTap = 0;
float tapSpeed = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ut1, OUTPUT);
  pinMode(ut2, OUTPUT);
  pinMode(ue1, INPUT);
  pinMode(ue2, INPUT);
  s1.attach(sp1);
  s2.attach(sp2);
  s1.write(a1);
  s2.write(a2);
}

float measureDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 25000);
  if (duration == 0) return -1;
  float distance = duration * 0.034 / 2;
  if (distance < 2 || distance > 900) return -1;
  return distance;
}

float getStableDistance(int trig, int echo) {
  float sum = 0;
  int count = 0;
  for (int i = 0; i < numSamples; i++) {
    float d = measureDistance(trig, echo);
    if (d > 0) {
      sum += d;
      count++;
    }
  }
  if (count == 0) return -1;
  return sum / count;
}

float distanceToOscFactor(float dist) {
  if (dist < 0) return 0;
  if (dist <= 50) return 1.0;
  if (dist >= 300) return 0.0;
  float norm = (dist - 50.0) / 250.0;
  float val = 1.0 - pow(norm, graphConstant);
  return constrain(val, 0.0, 1.0);
}

void tapAtSpeed(float tapSpeedDelay) {
  s1.write(160);
  delay(600 - tapSpeedDelay);
  s1.write(60);
  delay(600 - tapSpeedDelay);
}

int aggregatedVals = 0;
float aggregatedSum = 0;
float height; 
int read = 0;

void loop() {
  dis1 = getStableDistance(ut1, ue1);
  dis2 = getStableDistance(ut2, ue2);
  if (aggregatedVals < 10) {
    if (dis2 > 100) {
      aggregatedSum += dis2;
      aggregatedVals++;
      height = aggregatedSum / aggregatedVals;
    }
  } else {
    if (dis2 > 0) {
      if (read % 5 == 0) {
        aggregatedSum += dis2;
        aggregatedVals++;
        height = aggregatedSum / aggregatedVals;
      }
      if (abs(dis2 - height) > 15 && abs(dis2 - height) < 80) {
        s2.write(230);
      } else {
        s2.write(a2);
      }
    }
  }
  if (dis1 > 0) {
    float oscFactor = distanceToOscFactor(dis1);
    float tapSpeedDelay = oscFactor * 520;
    tapAtSpeed(tapSpeedDelay);
  }
}
