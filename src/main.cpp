#include <Arduino.h>
#include <Metro.h>

int sensorL = 21;
int sensorR = 20;
int sensorM = 19;

int enablePinA = 16;
int dirPin1A = 17;
int dirPin2A = 18;

int enablePinB = 9;
int dirPin1B = 8;
int dirPin2B = 7;

int BlackL = 200;
int BlackM = 70;
int BlackR = 200;

int speedL = 150;
int speedR = 150;

const int black = 0;
const int blue = 1;
const int red = 2;
const int white = 3;

byte incoming;

int color(int sensorRead, int blackT){
  if(sensorRead <= blackT){return black;}
  // else if(sensorRead <= BlueH){return blue;}
  // else if(sensorRead <= RedH){return red;}
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
  // analogWrite(enablePinA, speedL);
  // analogWrite(enablePinB, speedR);

  // digitalWrite(dirPin1A, HIGH);
  // digitalWrite(dirPin2A, LOW);

  // digitalWrite(dirPin1B, HIGH);
  // digitalWrite(dirPin2B, LOW);

  int sensorLRead = color(analogRead(sensorL), BlackL);
  int sensorRRead = color(analogRead(sensorR), BlackR);
  int sensorMRead = color(analogRead(sensorM), BlackM);


 if (Serial.available() > 0)
  {
  incoming = Serial.read();
  digitalWrite(dirPin1A, LOW);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, LOW);
  digitalWrite(dirPin2B, LOW);
  }

  Serial.print(sensorLRead);
  Serial.print(sensorMRead);
  Serial.println(sensorRRead);

  // Serial.println(analogRead(sensorR));
  // Serial.print(analogRead(sensorM));
  // Serial.println(analogRead(sensorR));
  delay(500);

  // if(sensorLRead == white && sensorRRead == white && sensorMRead == black){
  //   Serial.println("all good");
  // } else if(sensorLRead != white && sensorRRead != white){
  //   Serial.println("reached 90deg turn");
  // } else if(sensorLRead != white && sensorRRead == white){
  //   Serial.println("steer left"); 
  //   while(sensorLRead != white && sensorRRead == white){
  //     digitalWrite(dirPin1A, LOW);
  //     digitalWrite(dirPin2A, HIGH);
  //   }
  //   digitalWrite(dirPin1A, HIGH);
  //   digitalWrite(dirPin2A, LOW);
  // } else if(sensorLRead == white && sensorRRead != white){
  //   Serial.println("steer right"); 
  //   while(sensorLRead == white && sensorRRead != white){
  //     digitalWrite(dirPin1B, LOW);
  //     digitalWrite(dirPin2B, HIGH);
  //   }
  //   digitalWrite(dirPin1B, HIGH);
  //   digitalWrite(dirPin2B, LOW);
  // } else{
  //   Serial.println("UNKNOWN STATE"); 
  // }
}
