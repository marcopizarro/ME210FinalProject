#include <Arduino.h>
#include <Metro.h>
#include "Robot.h"

#define CALIB_TIME_INTERVAL 1000
#define START_TIME_INTERVAL 2000
#define TEST true
#define DIAGNOSTIC false


Metro passLineTimer = Metro(1000);

bool turn90 = false;
bool shortT = false;
bool shorterT = false;

typedef enum
{
  IDLE,
  LOAD,
  EXIT_LOAD,
  PASS_LOAD_LINE,
  TO_L_2,
  PIVOT_L_2, // spin
  TO_FORK_DOWN,
  PIVOT_FORK_DOWN, // spin
  TO_UNLOAD,
  PIVOT_UNLOAD, // spin
  UNLOAD,
  EXIT_UNLOAD, // spin or what not
  TO_FORK_UP,
  PIVOT_FORK_UP, // spin
  TO_T,
  PIVOT_T, // spin
  TO_LOAD, // to idle
} States_t;

States_t state;

VexMotor leftMotor = {
    16, // enable
    18, // dir1
    17,
};

VexMotor rightMotor = {
    9, // enable
    7, // dir1
    8,
};

Controls controls = {
    5,  // status
    6,  // toggle
    15, // limit for now
    4,  // red
    3,  // blue
    2,  // orange
    14, // limit back
};

Sensors sensors = {
    22, // left
    21, // middle
    23, // right
    0,  // junction left
    0,  // junction right
};

const byte servoPin = 10;

const bool test = false;

Robot robot = Robot(leftMotor, rightMotor, controls, sensors, servoPin);

void checkGlobalEvents(void);

void setup()
{
  pinMode(14, INPUT);
  state = IDLE;
}
int count = 0;
void loop()
{
  if (test)
  {
    robot.runDiagnostic();
  }
  else
  {
    robot.run();
    // robot.Start();
    // Serial.println(robot._adjustToggle);
    // robot.GetValues();
    // robot.GetStringReadings();
  }

  Serial.println(state);
  checkGlobalEvents();
  switch (state)
  {
  case IDLE:
    robot.Calibrate();
    break;
  case LOAD:
    robot.MoveBackward();
    break;
  case EXIT_LOAD:
    robot.MoveForward();
    break;
  case PASS_LOAD_LINE:
    robot.MoveForward();
  case TO_L_2:
    robot.Follow();
    break;
  case PIVOT_L_2:
    robot.MoveCW(); // for red
    break;
  case TO_FORK_DOWN:
    robot.Follow();
  case PIVOT_FORK_DOWN:
    robot.MoveCCW();
    break;
  case TO_UNLOAD:
    robot.Follow();
    break;
  case PIVOT_UNLOAD:
    robot.MoveCW();
    break;
  case UNLOAD:
    robot.LowerServo();
  }
}

void checkGlobalEvents()
{
  if (robot.calibrated && state == IDLE)
  {
    state = LOAD;
    // Serial.println("CLAIBRATED");
  }
  if (robot.TestLimitSwitch() && state == LOAD)
  {
    // Serial.println("HIT SWITCH");
    state = EXIT_LOAD;
    robot.calibrated = false;
    robot.RaiseServo();
    // skipLine.reset();
  }
  if (robot.TestJunction() && state == EXIT_LOAD)
  {
    robot.Stop();
    state = PASS_LOAD_LINE;
    passLineTimer.reset();
  }
  if (passLineTimer.check() && state == PASS_LOAD_LINE) {
    state = TO_L_2;
  }
  if (robot.juncVals & 0 &&) { // for red
  
  }





  if (turn90Timer.check())
  {
    turn90 = true;
  }
  if (shortTimer.check())
  {
    shortT = true;
  }
  if (shorterTimer.check())
  {
    shorterT = true;
  }
  if (state == EXIT_OPP && turn90)
  {
    state = EXIT_LOAD;
    turn90Timer.reset();
    turn90 = false;
  }
  if (state == EXIT_LOAD && robot.TestJunction() && turn90)
  {
    state = TO_T;
    shortTimer.reset();
    shortT = false;
  }
  if (state == TO_T && shortT)
  {
    state = CROSS_OVER;
    shortTimer.reset();
    shortT = false;
  }
  if (state == CROSS_OVER && robot.TestJunction() && shortT)
  {
    state = PIVOT_T;
    shorterTimer.reset();
    shorterT = false;
  }
  if (state == PIVOT_T && shorterT)
  {
    state = TO_LOAD;
  }
  if (state == TO_LOAD && robot.TestJunction())
  {
    state = TO_FORK;
  }
}
