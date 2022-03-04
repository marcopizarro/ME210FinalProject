#include <Arduino.h>
#include <Metro.h>
#include "Robot.h"

#define CALIB_TIME_INTERVAL 1000
#define START_TIME_INTERVAL 2000
#define TEST true
#define DIAGNOSTIC false


Metro passLineTimer = Metro(1000);
Metro rotateTimer = Metro(2000);

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
    19, // left
    21, // middle
    20, // right
    22,  // junction left
    23,  // junction right
};

const byte servoPin = 10;

bool test = true;

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
  robot.run();
  if (!robot.calibrated)
  {
    // robot.runDiagnostic();
    // robot.GetValues();
    robot.Calibrate();
  }
  else
  {
    robot.run();
    // robot.Start();
    // Serial.println(robot._adjustToggle);
    // robot.GetValues();
    // robot.GetLineValues();
    robot.GetStringReadings();
  }

  // Serial.println(state);
  checkGlobalEvents();
  switch (state)
  {
  case IDLE:
    robot.Stop();
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
    robot.GetJunctionReadings();
    break;
  case PIVOT_L_2:
    robot.GetJunctionReadings();
    robot.MoveCW(); // for red
    break;
  case TO_FORK_DOWN:
    robot.GetJunctionReadings();
    robot.Follow();
  case PIVOT_FORK_DOWN:
    robot.GetJunctionReadings();
    robot.MoveCCW();
    break;
  case TO_UNLOAD:
    robot.GetJunctionReadings();
    robot.Follow();
    break;
  case PIVOT_UNLOAD:
    robot.GetJunctionReadings();
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
  if ((uint8_t) passLineTimer.check() && state == PASS_LOAD_LINE) {
    state = TO_L_2;
  }
  if ((robot.juncVals & 1) == 0 && state == TO_L_2) { // for red so check for right
    state = PIVOT_L_2;
  }
  if (state == PIVOT_L_2 && (robot.juncVals & 3)) { // detects bofa
    state = TO_FORK_DOWN;
  }
  if (state == TO_FORK_DOWN && (robot.juncVals & 3)) {
    state = PIVOT_FORK_DOWN;
  }
  if (state == PIVOT_FORK_DOWN && (robot.juncVals ^ 0)) {
    state = TO_UNLOAD;
  }
  if (state == TO_UNLOAD && (robot.juncVals > 0)) { // detected a line
    state = PIVOT_UNLOAD;
    rotateTimer.reset();
  }
  if (state == PIVOT_UNLOAD && ((uint8_t) rotateTimer.check())) {
    state = UNLOAD;
    passLineTimer.reset();
  }
  if (state == UNLOAD && ((uint8_t) passLineTimer.check())) {
    state = EXIT_LOAD;
  }
  if (state == EXIT_LOAD && ((uint8_t) passLineTimer.check())) {
    state = IDLE;
  } // stop here for now

}
