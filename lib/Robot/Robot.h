/*
    Robot.h - Library for ME210 Robot
*/
#ifndef Robot_h
#define Robot_h

#include "Arduino.h"
#include "Metro.h"
#include <Servo.h>

#define SPEED 40
#define SLOW_SPEED 30
#define FAST_SPEED 100
#define MAX_SPEED 255
#define LOWER_ANGLE 133
#define RAISE_ANGLE 220


typedef enum
{
    LEFT,
    RIGHT
} Motor_t;

struct vexMotor
{
    byte enablePin = 0;
    byte dir1 = 0;
    byte dir2 = 0;
    int speed = 0;
};
typedef struct vexMotor VexMotor;

struct sensors
{
    byte leftSensor = 0;
    byte middleSensor = 0;
    byte rightSensor = 0;
    byte leftJunction = 0;
    byte rightJunction = 0;
};
typedef struct sensors Sensors;

struct controls
{
    byte statusBtn = 0;
    byte toggleBtn = 0;
    byte limitPin = 0;
    byte redLED = 0;
    byte blueLED = 0;
    byte orangeLED = 0;
    byte backLimitPin = 0;
};
typedef struct controls Controls;

class Robot {
    public:
        Robot(vexMotor leftMotor, vexMotor rightMotor, Controls controls, Sensors sensors, const byte servoPin);
        void run(void);
        // one time things
        void runDiagnostic(void);
        void CalibrateBlack(void);
        void CalibrateWhite(void);
        void Calibrate(void);

        // individual motor actions
        void MoveMotorForward(Motor_t motor);
        void MoveMotorBackward(Motor_t motor);
        void StopMotor(Motor_t motor);
        void SetMotorSpeed(Motor_t motor, int newSpeed);

        // does action to both motors
        void Follow(void);
        void Move(void);
        void MoveForward(void);
        void MoveBackward(void);
        void Stop(void);
        void MoveCW(void);
        void MoveCCW(void);
        void SetSpeed(int newSpeed);

        void Start(void);
        void GetJunctionReadings(void);
        void GetReadings(void);
        void GetStringReadings(void);
        void GetValues(void);
        void GetJunctionValues(void);

        // servo
        Servo scooper;
        void LowerServo(void);
        void RaiseServo(void);

        // event checking
        bool TestLimitSwitch(void);
        bool TestBackLimitSwitch(void);
        bool TestJunction(void);

        int WThreshL;
        int WThreshM;
        int WThreshR;

        int BThreshL;
        int BThreshM;
        int BThreshR;

        // junction thresholds
        int WThreshLJ;
        int WThreshRJ;

        int BThreshLJ;
        int BThreshRJ;

        int lVal;
        int mVal;
        int rVal;

        byte lineVals = 0;
        bool _adjustToggle = true;
        bool calibrated = false;

        // To-Do
        // red-blue
        // RaiseServo
        // LowerServo
        // checkEvents
        // - limit switch
        // - junctions
        //  - specifc junctions?
        
    private:
        Sensors _sensors;
        Controls _controls;
        vexMotor _rightMotor;
        vexMotor _leftMotor;

        Metro _adjustTimer = Metro(300);
        Metro _moveTimer = Metro(100);
        
        // line sensors b/w values
        int _BlackL = 100;
        int _BlackM = 100;
        int _BlackR = 100;

        int _WhiteL = 800;
        int _WhiteM = 800;
        int _WhiteR = 800;

        int _ThreshL;
        int _ThreshM;
        int _ThreshR;

        // junction b/w values
        int _BlackLJ = 100;
        int _BlackRJ = 100;

        int _WhiteLJ = 800;
        int _WhiteRJ = 800;

        int _ThreshLJ;
        int _ThreshRJ;

        int _slowSpeed = 40;
        int _fastSpeed = 100;

        byte _calib; // calibrate variable
        byte mode = 0;
        bool _move = false;
        

        void _setThresholds(void);
        void _runAdjustTimer(void);
        void _button1(void);
        void _button2(void);
        void _button3(void);
        void _button4(void);
        
        bool _forwards = true;
        bool _adjusted = false;
        bool _movedAway = false;
        bool _adjusting =  false;
        bool _timerSet = false;
        bool _diag = false;
};

#endif