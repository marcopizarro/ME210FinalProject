#include <Arduino.h>
#include <Metro.h>

int sensorL = 18;
int sensorR = 19;
int sensorM = 20;

int BlackH = 200;
int BlueH = 400;
int RedH = 800;

const int black = 0;
const int blue = 1;
const int red = 2;
const int white = 3;

int color(int sensorRead){
  if(sensorRead <= BlackH){return black;}
  else if(sensorRead <= BlueH){return blue;}
  else if(sensorRead <= RedH){return red;}
  else {return white;}
}

void setup(){
  Serial.begin(9600);
  pinMode(sensorL, INPUT);
  pinMode(sensorR, INPUT);
  pinMode(sensorM, INPUT);
}

void loop(){
  Serial.println(analogRead(sensorL));
  int sensorLRead = color(analogRead(sensorL));
  int sensorRRead = color(analogRead(sensorR));
  int sensorMRead = color(analogRead(sensorM));

  if(sensorLRead == white && sensorRRead == white && sensorMRead == black){
    Serial.println("all good");
  } else if(sensorMRead == black && sensorLRead != white && sensorRRead != white){
    Serial.println("reached 90deg turn");
  } else if(sensorMRead == black && sensorLRead != white && sensorRRead == white){
    Serial.println("steer left"); 
  } else if(sensorMRead == black && sensorLRead == white && sensorRRead != white){
    Serial.println("steer right"); 
  }
}
