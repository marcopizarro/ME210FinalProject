#include <Arduino.h>
#include <Metro.h>

#define CALIB_TIME_INTERVAL 1000
#define START_TIME_INTERVAL 2000
#define TEST true;

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
int BlackR = 100;

int WhiteL = 200;
int WhiteM = 70;
int WhiteR = 100;

int ThreshL;
int ThreshM;
int ThreshR;

int speedL = 150;
int speedR = 150;

const int black = 0;
const int blue = 1;
const int red = 2;
const int white = 3;

// TO DO
const int statusBtn = 0;
const int toggleBtn = 0;
const int limitPin = 0;

int mode = 0; // red is 0, blue is 1

typedef enum
{
  MOVE,
  STOP,
  ADJUST,
  CALIBRATE_BLACK,
  CALIBRATE_WHITE,
  UNLOAD,
  LOAD
} States_t;

typedef enum
{
  MOVE_TO_START,
  MOVE_FORWARD,
  MOVE_BACKWARD,
  MOVE_CW,
  MOVE_CCW
} MoveStates_t;

States_t state;
MoveStates_t moveState;

void handleMoveForward(void);
void handleMoveBackward(void);
void TestCalibrateTimer(void);
void handleStop(void);
void handleCCW(void);
void handleCW(void);
/*
stop
cw
ccw
moving
goal
loading
*/
byte incoming;
static Metro calibrateTimer = Metro(CALIB_TIME_INTERVAL);
static Metro moveTimer = Metro(CALIB_TIME_INTERVAL);
static Metro moveStartTimer = Metro(START_TIME_INTERVAL);

void calibrateBlack(void)
{
  BlackL = analogRead(sensorL);
  BlackR = analogRead(sensorR);
  BlackM = analogRead(sensorM);
}

void calibrateWhite(void)
{
  WhiteL = analogRead(sensorL);
  WhiteM = analogRead(sensorR);
  WhiteR = analogRead(sensorM);
}

void setThresholds(void)
{
  ThreshL = (BlackL + WhiteL) / 2;
  ThreshM = (BlackM + WhiteM) / 2;
  ThreshR = (BlackR + WhiteR) / 2;
}

int color(int sensorRead, int blackT)
{
  if (sensorRead <= blackT)
  {
    return black;
  }
  // else if(sensorRead <= BlueH){return blue;}
  // else if(sensorRead <= RedH){return red;}
  else
  {
    return white;
  }
}

void setup()
{
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

  state = STOP;
  moveState = MOVE_FORWARD;
}

void loop()
{
  if (TEST)
  {
    move();
  }
  else
  {
    checkGlobalEvents();

    switch (state)
    {
    case CALIBRATE_BLACK:
      calibrateBlack();
      calibrateTimer.reset();
      handleMoveForward();
      break;
    case CALIBRATE_WHITE:
      calibrateWhite();
      state = MOVE;
      moveState = MOVE_FORWARD;
      break;
    case MOVE:
      switch (moveState)
      {
      case MOVE_FORWARD:
        handleMoveForward();
        break;
      case MOVE_BACKWARD:
        handleMoveBackward();
        break;
      case MOVE_CW:
        handleCW();
        break;
      case MOVE_CCW:
        handleCCW();
        break;
      }
    }
  }

  // analogWrite(enablePinA, speedL);
  // analogWrite(enablePinB, speedR);

  // digitalWrite(dirPin1A, HIGH);
  // digitalWrite(dirPin2A, LOW);

  // digitalWrite(dirPin1B, HIGH);
  // digitalWrite(dirPin2B, LOW);

  int sensorLRead = color(analogRead(sensorL), BlackL);
  int sensorRRead = color(analogRead(sensorR), BlackR);
  int sensorMRead = color(analogRead(sensorM), BlackM);

  Serial.print(sensorLRead);
  Serial.print(sensorMRead);
  Serial.println(sensorRRead);

  Serial.println(analogRead(sensorR));
  // Serial.print(analogRead(sensorM));
  // Serial.println(analogRead(sensorR));
  delay(500);

  if (sensorLRead == white && sensorRRead == white && sensorMRead == black)
  {
    Serial.println("all good");
  }
  else if (sensorLRead != white && sensorRRead != white)
  {
    Serial.println("reached 90deg turn");
  }
  else if (sensorLRead != white && sensorRRead == white)
  {
    Serial.println("steer left");
    while (sensorLRead != white && sensorRRead == white)
    {
      digitalWrite(dirPin1A, LOW);
      digitalWrite(dirPin2A, HIGH);
    }
    digitalWrite(dirPin1A, HIGH);
    digitalWrite(dirPin2A, LOW);
  }
  else if (sensorLRead == white && sensorRRead != white)
  {
    Serial.println("steer right");
    while (sensorLRead == white && sensorRRead != white)
    {
      digitalWrite(dirPin1B, LOW);
      digitalWrite(dirPin2B, HIGH);
    }
    digitalWrite(dirPin1B, HIGH);
    digitalWrite(dirPin2B, LOW);
  }
  else
  {
    Serial.println("UNKNOWN STATE");
  }
}

void move(void)
{
  int l = analogRead(sensorL);
  int r = analogRead(sensorR);
  int m = analogRead(sensorM);

  if ((l >= ThreshL) && (m >= ThreshM) && (r >= ThreshR))
  {
    handleStop();
  }
  else if ((l >= ThreshL) && (m >= ThreshM))
  {
    handleStop();
  }
  else if ((r >= ThreshR) && (m >= ThreshM))
  {
    handleStop();
  }
  else if ((r >= ThreshR))
  {
    handleCW();
  }
  else if ((l >= ThreshL))
  {
    handleCW();
  }
  else if ((m >= ThreshM))
  {
    handleMove();
  }
  else
  {
    state = MOVE;
    moveState = MOVE_FORWARD;
  }
}

void checkGlobalEvents(void)
{
  TestLine();
  if (TestCalibrateTimer())
    respToCalibrateTimer();
  if (TestLimit())
    respToLimit();
}

void testStatusBtn(void)
{
  int status = digitalRead(testStatusBtn);
  if (status)
  {
    if (state != STOP)
    {
      state = STOP;
    }
    else
    {
      if (digitalRead(toggleBtn))
      {
        mode = 1; // LED IND
      }
      else
      {
        mode = 0; // LED IND
      }
      state = CALIBRATE_BLACK;
    }
  }
}

uint8_t TestLimit(void)
{
  return (uint8_t)digitalRead(limitPin);
}

void respToLimit(void)
{
  moveState = MOVE_BACKWARD;
}

uint8_t TestCalibrateTimer(void)
{
  return (uint8_t)calibrateTimer.check();
}

void respToCalibrateTimer(void)
{
  if (state == CALIBRATE_BLACK)
  {
    state = CALIBRATE_WHITE;
    handleStop();
  }
}

// MOVING
void handleMove(void)
{
  if (moveState == MOVE_FORWARD)
  {
    handleMoveForward();
  }
  if (moveState == MOVE_BACKWARD)
  {
    handleMoveBackward();
  }
}

void handleMoveForward(void)
{
  digitalWrite(dirPin1A, HIGH);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, HIGH);
  digitalWrite(dirPin2B, LOW);
}

void handleMoveBackward(void)
{
  digitalWrite(dirPin1A, LOW);
  digitalWrite(dirPin2A, HIGH);

  digitalWrite(dirPin1B, LOW);
  digitalWrite(dirPin2B, HIGH)
}

void handleStop(void)
{
  digitalWrite(dirPin1A, LOW);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, LOW);
  digitalWrite(dirPin2B, LOW)
}

void handleStop(void)
{
  digitalWrite(dirPin1A, LOW);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, LOW);
  digitalWrite(dirPin2B, LOW)
}

void handleCCW(void)
{
  digitalWrite(dirPin1A, HIGH);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, LOW);
  digitalWrite(dirPin2B, HIGH)
}

void handleCW(void)
{
  digitalWrite(dirPin1A, LOW);
  digitalWrite(dirPin2A, HIGH);

  digitalWrite(dirPin1B, HIGH);
  digitalWrite(dirPin2B, LOW)
}
