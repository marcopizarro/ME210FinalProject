#include <Arduino.h>
#include <Metro.h>

int sensorL = 18;
int sensorR = 19;
int sensorM = 20;

int BlackL = 0;
int BlackH = 200;

int BlueL = 200;
int BluekH = 400;

int RedL = 400;
int RedH = 800;

int WhiteL = 800;


void setup(){
  Serial.begin(9600);
  pinMode(sensorL, INPUT);
}

void loop(){
  Serial.println(analogRead(sensorL));
}
