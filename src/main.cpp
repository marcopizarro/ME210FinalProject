#include <Arduino.h>
#include <Metro.h>
#include "Robot.h"

#define CALIB_TIME_INTERVAL 1000
#define START_TIME_INTERVAL 2000
#define TEST true
#define DIAGNOSTIC false

Metro passLineTimer = Metro(500);
Metro passLineTimer2 = Metro(400);
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
    // ALIGN_TO_L2,
    TO_L_1,
    TO_L_2,
    PASS_L_2,
    PIVOT_L_2, // spin
    FIX_PIVOT_L_2,
    TO_FORK_DOWN_LINE,
    TO_FORK_DOWN_SLOW,
    REALIGN_FORK_DOWN,
    TO_FORK_DOWN,
    PIVOT_FORK_DOWN, // spin
    TO_UNLOAD,
    PIVOT_UNLOAD_TO_WHITE,
    PIVOT_UNLOAD, // spin
    CHECK_UNLOAD,
    REALIGN_UNLOAD,
    UNLOAD_BACK,
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
    robot.LowerServo();
    // robot.RaiseServo();
}
int count = 0;
void loop()
{
    robot.run();

    if (false)
    {
        // robot.runDiagnostic();
        // robot.GetValues();
        robot.runDiagnostic();
    }
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
        robot.SetMotorSpeed(LEFT, SPEED);
        robot.SetMotorSpeed(RIGHT, SPEED);
        robot.MoveBackward();
        // robot.MoveMotorBackward(RIGHT);
        // robot.MoveMotorBackward(LEFT);
        break;
    case EXIT_LOAD:
        robot.SetMotorSpeed(LEFT, SPEED);
        robot.SetMotorSpeed(RIGHT, SPEED);
        robot.MoveForward();
        // robot.MoveMotorForward(RIGHT);
        // robot.MoveMotorForward(RIGHT);

        break;
    case PASS_LOAD_LINE:
        robot.SetSpeed(SPEED);
        robot.MoveForward();
        break;
    case TO_L_1:
        robot.Follow();
        robot.GetJunctionReadings();
        robot.GetStringJReadings();
    case TO_L_2:
        robot.Follow();
        robot.GetJunctionReadings();
        robot.GetStringJReadings();
        break;
    case PASS_L_2:
        robot.MoveForward();
        Serial.println(state);
        break;
    case PIVOT_L_2:
        robot.SetSpeed(SLOW_SPEED + 2);
        robot.GetReadings();
        robot.GetJunctionReadings();
        robot.MoveCW(); // for red
        break;
    case FIX_PIVOT_L_2:
        robot.GetReadings();
        robot.GetJunctionReadings();
        robot.MoveCCW(); // for red
        break;
    case TO_FORK_DOWN_LINE:
        robot.SetSpeed(SLOW_SPEED);
        robot.GetReadings();
        robot.GetJunctionReadings();
        robot.MoveForward();
        robot.GetStringReadings();
        break;
    case TO_FORK_DOWN_SLOW:
        robot.GetReadings();
        robot.GetJunctionReadings();
        robot.Follow();
        break;
    case TO_FORK_DOWN:
        robot.SetSpeed(SPEED);
        robot.GetReadings();
        robot.GetJunctionReadings();
        robot.Follow();
        break;
    case REALIGN_FORK_DOWN:
        robot.GetReadings();
        robot.GetJunctionReadings();
        robot.MoveCW();
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
    case PIVOT_UNLOAD_TO_WHITE:
        robot.GetReadings();
        robot.GetJunctionReadings();
        robot.MoveCCW();
        break;
    case PIVOT_UNLOAD:
        robot.SetSpeed(SPEED);
        robot.GetJunctionReadings();
        robot.GetReadings();
        robot.MoveCCW();
        break;
    case CHECK_UNLOAD:
        robot.SetSpeed(MAX_SPEED);
        robot.Stop();
        break;
    case REALIGN_UNLOAD:
        robot.SetSpeed(SPEED);
        robot.MoveCW();
        break;
    case UNLOAD_BACK:
        robot.MoveBackward();
        robot.SetMotorSpeed(LEFT, SPEED + 60);
        robot.SetMotorSpeed(RIGHT, SPEED + 70);
        break;
    case UNLOAD:
        robot.Stop();
        robot.LowerServo();
        break;
    }
}

void checkGlobalEvents()
{
    if (robot.calibrated && state == IDLE)
    {
        state = LOAD;
        passLineTimer.reset();

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
    }
    if (!robot.TestJunction() && state == PASS_LOAD_LINE)
    {
        state = TO_L_1;
    }
    if (robot.juncVals == 2 && state == TO_L_1)
    { // for red so check for right
        // state = PASS_L_2;
        state = REALIGN_FORK_DOWN;
    }
    if (robot.juncVals == 1 && state == TO_L_1)
    { // for red so check for right
        // state = PASS_L_2;
        state = TO_L_2;
    }
    if (robot.juncVals == 2 && state == TO_L_2)
    { // for red so check for right
        // state = PASS_L_2;
        passLineTimer.reset();
        state = PIVOT_L_2;
    }
    if (state == PIVOT_L_2 && robot.juncVals == 1 && ((uint8_t)passLineTimer.check()))
    {
        state = TO_FORK_DOWN_LINE;
    }
    if (state == TO_FORK_DOWN_LINE && robot.juncVals != 1)
    {
        state = FIX_PIVOT_L_2;
    }
    if (state == FIX_PIVOT_L_2 && robot.juncVals == 1)
    {
        robot.Stop();
        state = TO_FORK_DOWN_LINE;
    }
    if (state == TO_FORK_DOWN_LINE && (robot.lineVals == 4 || robot.lineVals == 6 || robot.lineVals == 5 || robot.lineVals == 1))
    {
        robot.Stop();
        state = TO_FORK_DOWN_SLOW;
    }
    if (state == TO_FORK_DOWN_SLOW && (robot.lineVals == 5))
    {
        state = TO_FORK_DOWN;
    }
    if ((state == TO_FORK_DOWN_SLOW || state == TO_FORK_DOWN) && (robot.juncVals == 2))
    {
        state = REALIGN_FORK_DOWN;
    }
    if (state == REALIGN_FORK_DOWN && robot.lineVals == 1)
    {
        state = TO_FORK_DOWN_SLOW;
    }
    if (state == REALIGN_FORK_DOWN && robot.lineVals == 0)
    {
        state = PIVOT_FORK_DOWN;
    }
    if (state == TO_FORK_DOWN && (robot.juncVals == 0))
    {
        state = PIVOT_FORK_DOWN;
    }
    if (state == PIVOT_FORK_DOWN && robot.juncVals == 3 && (robot.lineVals == 1 || robot.lineVals == 5))
    {
        robot.Stop();
        state = TO_UNLOAD;
    }
    if (state == TO_UNLOAD && robot.juncVals < 3)
    {
        state = PIVOT_UNLOAD_TO_WHITE;
    }
    if (state == PIVOT_UNLOAD_TO_WHITE && robot.juncVals == 3 && robot.lineVals == 7)
    {
        state = PIVOT_UNLOAD;
        robot.Stop();
    }
    if (state == PIVOT_UNLOAD && (robot.juncVals == 0 || robot.juncVals == 1))
    {
        state = CHECK_UNLOAD;
    }
    if (state == CHECK_UNLOAD && robot.juncVals > 1)
    {
        state = REALIGN_UNLOAD;
    }
    if (state == CHECK_UNLOAD && (robot.juncVals == 0 || robot.juncVals == 1))
    {
        robot.Stop();
        state = UNLOAD_BACK;
        passLineTimer.reset();
    }
    if (state == REALIGN_UNLOAD && (robot.juncVals == 0 || robot.juncVals == 1))
    {
        state = UNLOAD_BACK;
        passLineTimer.reset();
    }
    if (state == UNLOAD_BACK && ((uint8_t)passLineTimer.check()))
    {
        state = UNLOAD;
        robot.Stop();
        passLineTimer2.reset();
    }
    if (state == UNLOAD && ((uint8_t)passLineTimer2.check()))
    {
        state = NADA;
    } // stop here for now
}
