#include <Arduino.h>
#include <Metro.h>

#define CALIB_TIME_INTERVAL 1000
#define START_TIME_INTERVAL 2000
#define TEST true

int sensorL = 22;
int sensorR = 21;
int sensorM = 23;

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

const int speed = 30;

const int black = 0;
const int blue = 1;
const int red = 2;
const int white = 3;

// TO DO
const int statusBtn = 5;
const int toggleBtn = 6;
// const int limitPin = 0;

const int redLED = 4;
const int blueLED = 3;
const int orangeLED = 2;


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

void handleMove(void);
void handleMoveForward(void);
void handleMoveBackward(void);
void TestCalibrateTimer(void);
void handleStop(void);
void handleCCW(void);
void handleCW(void);
void move(void);
void respToCalibrateTimer(void);
void checkGlobalEvents(void);

int incomingByte;
bool Move = false;
int calib = 0;
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
  WhiteM = analogRead(sensorM);
  WhiteR = analogRead(sensorR);
}

void setThresholds(void)
{
  ThreshL = (BlackL + WhiteL) / 2;
  ThreshM = (BlackM + WhiteM) / 2;
  ThreshR = (BlackR + WhiteR) / 2;
}

void getReadings(void) {
  int l = analogRead(sensorL);
  int r = analogRead(sensorR);
  int m = analogRead(sensorM);

  if ((l >= ThreshL) && (m >= ThreshM) && (r >= ThreshR))
  {
    Serial.println(111);
  }
  else if ((l >= ThreshL) && (m >= ThreshM))
  {
    Serial.println(110);
  }
  else if ((r >= ThreshR) && (m >= ThreshM))
  {
    Serial.println(011);
  }
  else if ((r >= ThreshR))
  {
    Serial.println(001);
  }
  else if ((l >= ThreshL))
  {
    Serial.println(100);
  }
  else if ((m >= ThreshM))
  {
    Serial.println(010);
  }
  else
  {
    Serial.println(000);
  }
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

void button1(){
    calibrateBlack();
    Serial.printf("%d, %d, %d\n", BlackL, BlackM, BlackR);
}

void button2(){
    calibrateWhite();
    Serial.printf("%d, %d, %d\n", WhiteL, WhiteM, WhiteR);
    setThresholds();
    Serial.printf("%d, %d, %d\n", ThreshL, ThreshM, ThreshR);
}

void button3(){
    Move = true;
    digitalWrite(orangeLED, HIGH);
}

void button4(){
    Move = false;
    digitalWrite(orangeLED, LOW);
    handleStop();
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

  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(orangeLED, OUTPUT);

  pinMode(statusBtn,INPUT_PULLUP);
  pinMode(toggleBtn,INPUT_PULLUP);

  state = STOP;
  moveState = MOVE_FORWARD;
  handleStop();
  analogWrite(enablePinA, speed);
  analogWrite(enablePinB, speed);
  // handleMoveForward();
  // delay(3000);
  // handleMoveBackward();
  // delay(3000);
  // handleCW();
  // delay(3000);
  // handleCCW();
  // delay(3000);
  // handleStop();
}

void loop()
{
  if(digitalRead(toggleBtn) == HIGH){
    mode = 0;
    digitalWrite(redLED, HIGH);
    digitalWrite(blueLED, LOW);
  } else {
    mode = 1;
    digitalWrite(redLED, LOW);
    digitalWrite(blueLED, HIGH);
  }

  if(digitalRead(statusBtn) == LOW){
    if (calib == 0) {
      button1();
      calib++;
    } 
    else if (calib == 1) {
      button2();
      calib++;
    } else if (calib == 2) {
      button3();
      calib++;
    }
    else {
      button4();
      calib = 0;
    }
    delay(500);
  }

  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    if (calib == 0) {
      button1();
      calib++;
    } 
    else if (calib == 1) {
      button2();
      calib++;
    } else if (calib == 2) {
      button3();
      calib++;
    }
    else {
      button4();
      calib = 0;
    }
  }
  if (TEST)
  {
    if (Move) {
      getReadings();
      move();
    } else {
      // Serial.printf("%d, %d, %d\n", analogRead(sensorL), analogRead(sensorM), analogRead(sensorR));
    }
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
}

void move(void)
{
  int l = analogRead(sensorL);
  int r = analogRead(sensorR);
  int m = analogRead(sensorM);

  if ((l <= ThreshL) && (m <= ThreshM) && (r <= ThreshR))
  {
    handleCW();
  }
  else if ((l <= ThreshL) && (m <= ThreshM))
  {
    handleCW();
  }
  else if ((r <= ThreshR) && (m <= ThreshM))
  {
    handleCCW();
  }
  else if ((r <= ThreshR))
  {
    handleCW();
  }
  else if ((l <= ThreshL))
  {
    handleCCW();
  }
  else if ((m <= ThreshM))
  {
    handleMove();
  }
  else
  {
    handleMove();
    state = MOVE;
    moveState = MOVE_FORWARD;
  }
}

void checkGlobalEvents(void)
{
  // if (TestCalibrateTimer())
  //   respToCalibrateTimer();
  // if (TestLimit())
  //   respToLimit();
}

// void testStatusBtn(void)
// {
//   int status = digitalRead(testStatusBtn);
//   if (status)
//   {
//     if (state != STOP)
//     {
//       state = STOP;
//     }
//     else
//     {
//       if (digitalRead(toggleBtn))
//       {
//         mode = 1; // LED IND
//       }
//       else
//       {
//         mode = 0; // LED IND
//       }
//       state = CALIBRATE_BLACK;
//     }
//   }
// }

// uint8_t TestLimit(void)
// {
//   return (uint8_t)digitalRead(limitPin);
// }

// void respToLimit(void)
// {
//   moveState = MOVE_BACKWARD;
// }

// uint8_t TestCalibrateTimer(void)
// {
//   return (uint8_t)calibrateTimer.check();
// }

// void respToCalibrateTimer(void)
// {
//   if (state == CALIBRATE_BLACK)
//   {
//     state = CALIBRATE_WHITE;
//     handleStop();
//   }
// }

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
  // analogWrite(enablePinB, 100);
  // analogWrite(enablePinA, 100);
  digitalWrite(dirPin1A, LOW);
  digitalWrite(dirPin2A, HIGH);

  digitalWrite(dirPin1B, LOW);
  digitalWrite(dirPin2B, HIGH);
}

void handleMoveBackward(void)
{
  // analogWrite(enablePinB, 100);
  // analogWrite(enablePinA, 100);
  digitalWrite(dirPin1A, HIGH);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, HIGH);
  digitalWrite(dirPin2B, LOW);
}

void handleStop(void)
{
  // analogWrite(enablePinB, 100);
  // analogWrite(enablePinA, 100);
  digitalWrite(dirPin1A, LOW);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, LOW);
  digitalWrite(dirPin2B, LOW);
}

void handleCW(void)
{
  digitalWrite(dirPin1A, HIGH);
  digitalWrite(dirPin2A, LOW);

  digitalWrite(dirPin1B, LOW);
  digitalWrite(dirPin2B, HIGH);
}

void handleCCW(void)
{
  digitalWrite(dirPin1A, LOW);
  digitalWrite(dirPin2A, HIGH);

  digitalWrite(dirPin1B, HIGH);
  digitalWrite(dirPin2B, LOW);
}

// void handleCCW(void)
// {
//   analogWrite(enablePinB, 250);
// }

// void handleCW(void)
// {
//   analogWrite(enablePinA, 250);
// }
