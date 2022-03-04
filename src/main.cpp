#include <Arduino.h>
#include <Metro.h>
#include "Robot.h"

#define CALIB_TIME_INTERVAL 1000
#define START_TIME_INTERVAL 2000
#define TEST true
#define DIAGNOSTIC false

Metro passLineTimer = Metro(800);
Metro passLineTimer2 = Metro(500);
Metro rotateTimer = Metro(60);
Metro fullturn = Metro(2000);


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
  PASS_L_2,
  PIVOT_L_2, // spin
  TO_FORK_DOWN,
  PASS_FORK_DOWN,
  PIVOT_FORK_DOWN, // spin
  TO_UNLOAD,
    UNLOAD_BACK,
  PIVOT_UNLOAD, // spin
  UNLOAD,
  EXIT_UNLOAD, // spin or what not
  TO_FORK_UP,
  PIVOT_FORK_UP, // spin
  TO_T,
  PIVOT_T, // spin
  TO_LOAD, // to idle
  NADA,
  TURN_AROUND,
  BACKINTOIT,
  GOALIL,
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
    22, // junction left
    23, // junction right
};

const byte servoPin = 10;

bool test = true;

Robot robot = Robot(leftMotor, rightMotor, controls, sensors, servoPin);

void checkGlobalEvents(void);

void setup()
{
  pinMode(14, INPUT);
  state = IDLE;
  passLineTimer.reset();
  passLineTimer2.reset();
  fullturn.reset();
  // robot.RaiseServo();
}
int count = 0;
void loop()
{
  robot.run();
  

  // if (!robot.calibrated)
  // {
  //   // robot.runDiagnostic();
  //   // robot.GetValues();
  //   robot.Calibrate();
  // }
  // else
  // {
  //   robot.run();
  //   // robot.Start();
  //   // Serial.println(robot._adjustToggle);
  //   // robot.GetValues();
  //   // robot.GetLineValues();
  //   // robot.GetStringReadings();
  // }
  Serial.println(state);
  checkGlobalEvents();
  switch (state)
  {
  case NADA:
    robot.Stop();
    robot.GetStringJReadings();
    break;
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
    break;
  case TO_L_2:
    robot.Follow();
    robot.GetJunctionReadings();
    Serial.println(robot.juncVals);
    robot.GetStringJReadings();
    break;
  case PASS_L_2:
    robot.MoveForward();
    Serial.println(state);
    break;
  case PIVOT_L_2:
    robot.GetReadings();
    robot.GetJunctionReadings();
    robot.MoveCW(); // for red
    break;
  case TO_FORK_DOWN:
    robot.GetReadings();
    robot.GetJunctionReadings();
    robot.Follow();
    break;
  case PASS_FORK_DOWN:
    robot.GetReadings();
    robot.GetJunctionReadings();
    robot.MoveForward();
    break;
  case PIVOT_FORK_DOWN:
    robot.GetReadings();
    robot.GetJunctionReadings();
    robot.MoveCCW();
    break;
  case TO_UNLOAD:
    robot.GetJunctionReadings();
    robot.Follow();
    break;
  case UNLOAD_BACK:
    robot.GetJunctionReadings();
    robot.MoveBackward();
    break;
  case PIVOT_UNLOAD:
    robot.GetJunctionReadings();
    robot.MoveCW();
    break;
  case UNLOAD:
    robot.Stop();
    robot.LowerServo();
    break;
  case TURN_AROUND:
  // robot.RaiseServo();
    // robot.SetSpeed(50);
    robot.MoveCCW();
    break;
  case BACKINTOIT:
    robot.MoveBackward();
    break;
  case GOALIL:
    robot.Follow();
    break;
  }
}

void checkGlobalEvents()
{
  if (robot.calibrated && state == IDLE)
  {
    state = LOAD;
    // Serial.println("CLAIBRATED");
    robot.calibrated = false;
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
  if ((uint8_t)passLineTimer.check() && state == PASS_LOAD_LINE)
  {
    state = TO_L_2;
  }
  if (robot.juncVals == 2 && state == TO_L_2)
  { // for red so check for right
    state = PASS_L_2;
    passLineTimer2.reset();
  }
  if (state == PASS_L_2 && (uint8_t)passLineTimer2.check())
  {
    state = PIVOT_L_2;
    passLineTimer2.reset();
  }
  if (state == PIVOT_L_2 && (robot.juncVals > 0) && (robot.lineVals < 7 ) && (uint8_t)passLineTimer2.check())
  { // detects bofa
    state = TO_FORK_DOWN;
    passLineTimer2.reset();
  }
  if (state == TO_FORK_DOWN && (robot.juncVals < 3) && (uint8_t)passLineTimer2.check())
  {
    state = PASS_FORK_DOWN;
    passLineTimer2.reset();
  }
  if (state == PASS_FORK_DOWN && (uint8_t)passLineTimer2.check())
  {
    state = PIVOT_FORK_DOWN;
    rotateTimer.reset();
  }
  if (state == PIVOT_FORK_DOWN && (robot.juncVals > 0) && (robot.lineVals < 7) && (uint8_t)rotateTimer.check())
  {
    state = GOALIL;
    // state = TURN_AROUND;
    passLineTimer2.reset();
  }
  if(state == GOALIL && (uint8_t)passLineTimer2.check()){
    state = TURN_AROUND;
    passLineTimer2.reset();
    robot.Stop();
  }
    if(state == TURN_AROUND && (robot.WThreshM > robot.BThreshM) && (uint8_t)passLineTimer2.check())
  {
    state = BACKINTOIT;
  }
  // if(state == TURN_AROUND && (uint8_t)fullturn.check()) //&& (robot.WThreshM > robot.BThreshM)
  // {
  //   state = NADA;
  // }


  // if (state == TO_UNLOAD && (robot.juncVals < 3) && (uint8_t)passLineTimer.check()) { // detected a line
  //   state = NADA;
  //   // state = UNLOAD_BACK;
  //   // passLineTimer2.reset();
  // }
  // if (state == UNLOAD_BACK && (uint8_t) passLineTimer2.check()) {
  //   state = PIVOT_UNLOAD;
  //   passLineTimer2.reset();
  // }
  // if (state == PIVOT_UNLOAD && ((uint8_t) passLineTimer2.check())) {
  //   state = UNLOAD;
  //   passLineTimer.reset();
  // }
  
  // if (state == UNLOAD && ((uint8_t) passLineTimer.check())) {
  //   state = NADA;
  //   passLineTimer.reset();
  // }
  // if (state == EXIT_LOAD && ((uint8_t) passLineTimer.check())) {
  //   state = NADA;
  // } // stop here for now
}
