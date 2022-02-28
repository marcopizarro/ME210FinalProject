#include <Arduino.h>
#include <Metro.h>

int sensorL = 1;
int sensorR = 2;
int sensorM = 3;

int enablePinA = 16;
int dirPin1A = 17;
int dirPin2A = 18;

int enablePinB = 9;
int dirPin1B = 8;
int dirPin2B = 7;

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

  pinMode(dirPin1A, OUTPUT);
  pinMode(dirPin1B, OUTPUT);
  pinMode(enablePinA, OUTPUT);
  pinMode(dirPin2A, OUTPUT);
  pinMode(dirPin2B, OUTPUT);
  pinMode(enablePinB, OUTPUT);
}

void loop(){
  analogWrite(enablePinA, 255);
  analogWrite(enablePinB, 255);

  digitalWrite(dirPin1A, HIGH);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, HIGH);
  digitalWrite(dirPin2B, LOW);

  // delay(2000);

  // digitalWrite(dirPin1A, LOW);
  // digitalWrite(dirPin2A, HIGH);

  // digitalWrite(dirPin1B, LOW);
  // digitalWrite(dirPin2B, HIGH);
  // delay(2000);

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
