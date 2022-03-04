#include "Arduino.h"
#include "Robot.h"
#include <Metro.h>
#include <Servo.h>

Robot::Robot(vexMotor leftMotor, vexMotor rightMotor, Controls controls, Sensors sensors, const byte servoPin)
{
    _leftMotor = leftMotor;
    _rightMotor = rightMotor;
    _controls = controls;
    _sensors = sensors;
    pinMode(_sensors.leftSensor, INPUT);
    pinMode(_sensors.middleSensor, INPUT);
    pinMode(_sensors.rightSensor, INPUT);

    pinMode(_sensors.leftJunction, INPUT);
    pinMode(_sensors.rightJunction, INPUT);

    pinMode(_leftMotor.dir1, OUTPUT);
    pinMode(_leftMotor.dir2, OUTPUT);
    pinMode(_leftMotor.enablePin, OUTPUT);
    pinMode(_rightMotor.dir1, OUTPUT);
    pinMode(_rightMotor.dir2, OUTPUT);
    pinMode(_rightMotor.enablePin, OUTPUT);

    pinMode(_controls.redLED, OUTPUT);
    pinMode(_controls.blueLED, OUTPUT);
    pinMode(_controls.orangeLED, OUTPUT);

    pinMode(_controls.statusBtn, INPUT_PULLUP);
    pinMode(_controls.toggleBtn, INPUT_PULLUP);
    pinMode(_controls.limitPin, INPUT);
    pinMode(_controls.backLimitPin, INPUT);

    SetSpeed(SPEED);
    scooper.attach(servoPin);
    LowerServo();

    _adjustTimer.reset();
}

void Robot::run(void)
{
    lVal = analogRead(_sensors.leftSensor);
    mVal = analogRead(_sensors.middleSensor);
    rVal = analogRead(_sensors.rightSensor);

    if (digitalRead(_controls.toggleBtn) == HIGH)
    {
        mode = 0;
        digitalWrite(_controls.redLED, HIGH);
        digitalWrite(_controls.blueLED, LOW);
    }
    else
    {
        mode = 1;
        digitalWrite(_controls.redLED, LOW);
        digitalWrite(_controls.blueLED, HIGH);
    }
}

void Robot::runDiagnostic(void)
{
    _diag = true;
    Serial.println("RUNNING DIAGNOSTIC...");
    Serial.println(_leftMotor.dir1);
    digitalWrite(_controls.redLED, HIGH);
    delay(1000);
    digitalWrite(_controls.blueLED, HIGH);
    delay(1000);
    digitalWrite(_controls.orangeLED, HIGH);
    delay(1000);
    while (digitalRead(_controls.statusBtn) == HIGH)
    {
    }
    digitalWrite(_controls.redLED, LOW);
    delay(500);
    digitalWrite(_controls.blueLED, LOW);
    delay(500);
    digitalWrite(_controls.orangeLED, LOW);
    delay(500);
    while (digitalRead(_controls.limitPin) == HIGH)
    {
    }
    while (digitalRead(_controls.backLimitPin) == LOW)
    {
    }
    digitalWrite(_controls.orangeLED, HIGH);
    delay(1000);
    MoveMotorForward(LEFT);
    delay(1000);
    MoveMotorBackward(LEFT);
    delay(1000);
    StopMotor(LEFT);
    delay(1000);

    MoveMotorForward(RIGHT);
    delay(1000);
    MoveMotorBackward(RIGHT);
    delay(1000);
    StopMotor(RIGHT);
    delay(1000);

    MoveForward();
    delay(1000);
    MoveBackward();
    delay(1000);
    MoveCW();
    delay(1000);
    MoveCCW();
    delay(1000);
    Stop();
    delay(1000);

    RaiseServo();
    delay(500);
    LowerServo();
    delay(500);
}

void Robot::CalibrateBlack(void)
{
    Serial.println("CALIBRATE BLACK");
    _BlackL = analogRead(_sensors.leftSensor);
    _BlackM = analogRead(_sensors.middleSensor);
    _BlackR = analogRead(_sensors.rightSensor);

    _BlackLJ = analogRead(_sensors.leftJunction);
    _BlackRJ = analogRead(_sensors.rightJunction);
}

void Robot::CalibrateWhite(void)
{
    Serial.println("CALIBRATE WHITE");
    _WhiteL = analogRead(_sensors.leftSensor);
    _WhiteM = analogRead(_sensors.middleSensor);
    _WhiteR = analogRead(_sensors.rightSensor);

    _WhiteLJ = analogRead(_sensors.leftJunction);
    _WhiteRJ = analogRead(_sensors.rightJunction);
}

void Robot::_setThresholds(void)
{
    Serial.println("GETTING THRESHOLD");
    _ThreshL = (_BlackL + _WhiteL) / 2;
    _ThreshM = (_BlackM + _WhiteM) / 2;
    _ThreshR = (_BlackR + _WhiteR) / 2;

    WThreshL = (_WhiteL + _ThreshL) / 2;
    WThreshM = (_WhiteM + _ThreshM) / 2;
    WThreshR = (_WhiteR + _ThreshR) / 2;

    BThreshL = (_BlackL + _ThreshL) / 2;
    BThreshM = (_BlackM + _ThreshM) / 2;
    BThreshR = (_BlackR + _ThreshR) / 2;

    _ThreshLJ = (_BlackL + _WhiteL) / 2;
    _ThreshRJ = (_BlackR + _WhiteR) / 2;

    WThreshLJ = (_WhiteL + _ThreshL) / 2;
    WThreshRJ = (_WhiteR + _ThreshR) / 2;

    BThreshLJ = (_BlackL + _ThreshL) / 2;
    BThreshRJ = (_BlackR + _ThreshR) / 2;
}

void Robot::_runAdjustTimer(void)
{
    if (_adjustToggle)
    {
        if (!_timerSet)
        {
            _adjustTimer.reset();
            _timerSet = true;
        }
        else
        {
            if ((uint8_t)_adjustTimer.check())
            {
                _timerSet = false;
                _adjustToggle = false;
            }
        }
    }
    else
    {
        if (!_timerSet)
        {
            _moveTimer.reset();
            _timerSet = true;
        }
        else
        {
            if ((uint8_t)_moveTimer.check())
            {
                _timerSet = false;
                _adjustToggle = true;
            }
        }
    }
}

void Robot::_button1()
{
    CalibrateBlack();
    Serial.printf("%d, %d, %d\n", _BlackL, _BlackM, _BlackR);
}

void Robot::_button2()
{
    CalibrateWhite();
    Serial.printf("%d, %d, %d\n", _WhiteL, _WhiteM, _WhiteR);
    _setThresholds();
    Serial.printf("%d, %d, %d\n", _ThreshL, _ThreshM, _ThreshR);
}

void Robot::_button3()
{
    _move = true;
    digitalWrite(_controls.orangeLED, HIGH);
    // GetReadings();
    // Move();
}

void Robot::_button4()
{
    _move = false;
    digitalWrite(_controls.orangeLED, LOW);
    Stop();
}

void Robot::GetReadings(void)
{
    if ((lVal >= WThreshL))
    {
        lineVals |= 4;
    }
    if ((mVal >= WThreshM))
    {
        lineVals |= 2;
    }
    if ((rVal >= WThreshR))
    {
        lineVals |= 1;
    }

    if ((lVal <= BThreshL))
    {
        lineVals &= 3;
    }
    if ((mVal <= BThreshM))
    {
        lineVals &= 5;
    }
    if ((rVal <= BThreshR))
    {
        lineVals &= 6;
    }
}

void Robot::GetStringReadings(void)
{
    GetReadings();

    if (lineVals == 7)
    {
        Serial.println("111");
    }
    else if (lineVals == 6)
    {
        Serial.println("110");
    }
    else if (lineVals == 5)
    {
        Serial.println("101");
    }
    else if (lineVals == 4)
    {
        Serial.println("100");
    }
    else if (lineVals == 3)
    {
        Serial.println("011");
    }
    else if (lineVals == 2)
    {
        Serial.println("010");
    }
    else if (lineVals == 1)
    {
        Serial.println("001");
    }
    else
    {
        Serial.println("000");
    }
}

void Robot::GetValues(void)
{
    Serial.printf("%d, %d, %d\n", lVal, mVal, rVal);
}

void GetJunctionReadings(void) {

}

void GetJunctionValues(void) {

}


void Robot::LowerServo(void)
{
    scooper.write(LOWER_ANGLE);
}

void Robot::RaiseServo(void)
{
    scooper.write(RAISE_ANGLE);
}

bool Robot::TestLimitSwitch(void)
{
    if (digitalRead(_controls.limitPin) == LOW) {
        return true;
    } else {
        return false;
    }
}

bool Robot::TestBackLimitSwitch(void)
{
    if (digitalRead(_controls.backLimitPin) == HIGH) {
        return true;
    } else {
        return false;
    }
}

bool Robot::TestJunction(void)
{
    GetJunctionReadings();
    if (lineVals == 0 || lineVals == 1 || lineVals == 6 || lineVals == 5) {
        return true;
    } else {
        return false;
    }
}


void Robot::MoveMotorForward(Motor_t motor)
{
    if (motor == LEFT)
    {
        if (_diag)
            Serial.println("LEFT F");
        digitalWrite(_leftMotor.dir1, LOW);
        digitalWrite(_leftMotor.dir2, HIGH);
    }
    else
    {
        if (_diag)
            Serial.println("RIGHT F");
        digitalWrite(_rightMotor.dir1, LOW);
        digitalWrite(_rightMotor.dir2, HIGH);
    }
}

void Robot::MoveMotorBackward(Motor_t motor)
{
    if (motor == LEFT)
    {
        if (_diag)
            Serial.println("LEFT B");
        digitalWrite(_leftMotor.dir1, HIGH);
        digitalWrite(_leftMotor.dir2, LOW);
    }
    else
    {
        if (_diag)
            Serial.println("RIGHT B");
        digitalWrite(_rightMotor.dir1, HIGH);
        digitalWrite(_rightMotor.dir2, LOW);
    }
}

void Robot::StopMotor(Motor_t motor)
{
    if (motor == LEFT)
    {
        if (_diag)
            Serial.println("LEFT STOP");
        digitalWrite(_leftMotor.dir1, LOW);
        digitalWrite(_leftMotor.dir2, LOW);
    }
    else
    {
        if (_diag)
            Serial.println("RIGHT STOP");
        digitalWrite(_rightMotor.dir1, LOW);
        digitalWrite(_rightMotor.dir2, LOW);
    }
}

void Robot::MoveForward(void)
{
    Serial.println("FORWARD");
    MoveMotorForward(LEFT);
    MoveMotorForward(RIGHT);
}

void Robot::MoveBackward(void)
{
    Serial.println("BACKWARD");
    MoveMotorBackward(LEFT);
    MoveMotorBackward(RIGHT);
}

void Robot::Stop(void)
{
    Serial.println("STOP");
    StopMotor(LEFT);
    StopMotor(RIGHT);
}

void Robot::MoveCCW(void)
{
    Serial.println("CCW");
    MoveMotorForward(LEFT);
    MoveMotorBackward(RIGHT);
}

void Robot::MoveCW(void)
{
    Serial.println("CW");
    MoveMotorBackward(LEFT);
    MoveMotorForward(RIGHT);
}

void Robot::SetSpeed(int newSpeed)
{
    SetMotorSpeed(LEFT, newSpeed);
    SetMotorSpeed(RIGHT, newSpeed);
}

void Robot::Start(void)
{
    if (digitalRead(_controls.statusBtn) == LOW)
    {
        if (_calib == 0)
        {
            _button1();
            _calib++;
        }
        else if (_calib == 1)
        {
            _button2();
            _calib++;
        }
        else if (_calib == 2)
        {
            _button3();
            _calib++;
        }
        else
        {
            _button4();
            _calib = 0;
        }
        delay(500);
    }
    if (_move)
    {
        // GetReadings();

        Follow();
    }
}

void Robot::Calibrate(void)
{
    if (digitalRead(_controls.statusBtn) == LOW)
    {
        if (_calib == 0)
        {
            _button1();
            _calib++;
        }
        else if (_calib == 1)
        {
            _button2();
            _calib++;
        }
        else if (_calib == 2)
        {
            _button3();
            _calib++;
        }
        else
        {
            _button4();
            _calib = 0;
            calibrated = false;
        }
        delay(500);
    }
    if (_move)
    {
        calibrated = true;
    }
}

void Robot::Follow(void)
{
    GetReadings();

    if (lineVals == 7)
    {
        Serial.println("NO MAN'S LAND");
        // Serial.println("111");
        SetSpeed(SPEED);
        Move();
        // Stop();
    }
    else if (lineVals == 6)
    {
        SetSpeed(SPEED);
        // Serial.println("110");
        MoveCW();
        // _runAdjustTimer();
        // if (_adjustToggle)
        // {
        //     MoveCW();
        // }
        // else
        // {
        //     Move();
        // }
    }

    else if (lineVals == 3)
    {
        SetSpeed(SPEED);
        // Serial.println("011");
        MoveCCW();
        // _runAdjustTimer();
        // if (_adjustToggle)
        // {
        //     MoveCCW();
        // }
        // else
        // {
        //     Move();
        // }
    }

    else if (lineVals == 1)
    {
        // Serial.println("001");
        SetSpeed(SPEED);
        Move();
        // MoveCCW();
        // _runAdjustTimer();
        // if (_adjustToggle)
        // {
        //     MoveCCW();
        // }
        // else
        // {
        //     Move();
        // }
    }
    else if (lineVals == 4)
    {
        // Serial.println("100");
        SetSpeed(SPEED);
        // MoveCW();
        Move();
        // _runAdjustTimer();
        // if (_adjustToggle)
        // {
        //     MoveCW();
        // }
        // else
        // {
        //     Move();
        // }
    }
    else if (lineVals == 2)
    {
        // SetSpeed(MAX_SPEED);
        Serial.println("JUNCTION");
        Move();
        // Serial.println("010");
        // Stop();
    }
    else if (lineVals == 5)
    {
        // Serial.println("101");
        SetSpeed(SPEED);
        Move();
    }
    else
    {
        // Serial.println("000");
        Serial.println("LINE");
        Move();
        // SetSpeed(MAX_SPEED);
        // Stop();
    }
}

void Robot::SetMotorSpeed(Motor_t motor, int newSpeed)
{
    if (motor == LEFT)
    {
        if (_leftMotor.speed != newSpeed)
        {
            _leftMotor.speed = newSpeed;
            analogWrite(_leftMotor.enablePin, newSpeed);
        }
    }
    else
    {
        if (_rightMotor.speed != newSpeed)
        {
            _rightMotor.speed = newSpeed;
            analogWrite(_rightMotor.enablePin, newSpeed);
        }
    }
}

void Robot::Move(void)
{
    if (_forwards)
    {
        MoveForward();
    }
    else
    {
        MoveBackward();
    }
}