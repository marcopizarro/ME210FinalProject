#include "Arduino.h"
#include "Robot.h"
#include <Metro.h>

Robot::Robot(vexMotor leftMotor, vexMotor rightMotor, Controls controls, Sensors sensors)
{
    _leftMotor = leftMotor;
    _rightMotor = rightMotor;
    _controls = controls;
    _sensors = sensors;
    pinMode(_sensors.leftSensor, INPUT);
    pinMode(_sensors.middleSensor, INPUT);
    pinMode(_sensors.rightSensor, INPUT);

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

    SetSpeed(SPEED);

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
}

void Robot::CalibrateBlack(void)
{
    Serial.println("CALIBRATE BLACK");
    _BlackL = analogRead(_sensors.leftSensor);
    _BlackM = analogRead(_sensors.middleSensor);
    _BlackR = analogRead(_sensors.rightSensor);
}

void Robot::CalibrateWhite(void)
{
    Serial.println("CALIBRATE WHITE");
    _WhiteL = analogRead(_sensors.leftSensor);
    _WhiteM = analogRead(_sensors.middleSensor);
    _WhiteR = analogRead(_sensors.rightSensor);
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
        vals |= 4;
    }
    if ((mVal >= WThreshM))
    {
        vals |= 2;
    }
    if ((rVal >= WThreshR))
    {
        vals |= 1;
    }

    if ((lVal <= BThreshL))
    {
        vals &= 3;
    }
    if ((mVal <= BThreshM))
    {
        vals &= 5;
    }
    if ((rVal <= BThreshR))
    {
        vals &= 6;
    }
}

void Robot::GetStringReadings(void)
{
    GetReadings();

    if (vals == 7)
    {
        Serial.println("111");
    }
    else if (vals == 6)
    {
        Serial.println("110");
    }
    else if (vals == 5)
    {
        Serial.println("101");
    }
    else if (vals == 4)
    {
        Serial.println("100");
    }
    else if (vals == 3)
    {
        Serial.println("011");
    }
    else if (vals == 2)
    {
        Serial.println("010");
    }
    else if (vals == 1)
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

void Robot::MoveCW(void)
{
    Serial.println("CW");
    MoveMotorForward(LEFT);
    MoveMotorBackward(RIGHT);
}

void Robot::MoveCCW(void)
{
    Serial.println("CCW");
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

void Robot::Follow(void)
{
    GetReadings();

    if (vals == 7)
    {
        // Serial.println("111");
        SetSpeed(SPEED);
        Move();
        // Stop();
    }
    else if (vals == 6)
    {
        Serial.println("JUNCTION");
        // Serial.println("110");
        if (!_adjusted && !_adjusting && !_movedAway)
        {
            _adjustTimer.reset();
            _adjusting = true;
        }
        else if (!_adjusted && _adjusting && !_movedAway)
        {
            SetMotorSpeed(RIGHT, SLOW_SPEED);
            SetMotorSpeed(LEFT, FAST_SPEED);
            MoveCW();
            if ((uint8_t)_adjustTimer.check())
            {
                _adjusted = true;
                _adjusting = false;
                _adjustTimer.reset();
            }
        }
        else if (_adjusted && !_adjusting && !_movedAway)
        {
            SetSpeed(SLOW_SPEED);
            Move();
            if ((uint8_t)_adjustTimer.check())
            {
                _adjusted = false;
                _adjusting = false;
                _movedAway = true;
            }
        }
        else
        {
            _movedAway = false; // dumb ik
        }
        // MoveCW();
    }

    else if (vals == 3)
    {
        
        Serial.println("JUNCTION");
        // Serial.println("011");
        if (!_adjusted && !_adjusting && !_movedAway)
        {
            _adjustTimer.reset();
            _adjusting = true;
        }
        else if (!_adjusted && _adjusting && !_movedAway)
        {
            SetMotorSpeed(RIGHT, FAST_SPEED);
            SetMotorSpeed(LEFT, SLOW_SPEED);
            MoveCCW();
            if ((uint8_t)_adjustTimer.check())
            {
                _adjusted = true;
                _adjusting = false;
                _adjustTimer.reset();
            }
        }
        else if (_adjusted && !_adjusting && !_movedAway)
        {   
            SetSpeed(SLOW_SPEED);
            Move();
            if ((uint8_t)_adjustTimer.check())
            {
                _adjusted = false;
                _adjusting = false;
                _movedAway = true;
            }
        }
        else
        {
            _movedAway = false; // dumb ik
        }

        // MoveCCW();
    }

    else if (vals == 1)
    {
        // Serial.println("001");
        SetSpeed(SPEED);
        Move();
    }
    else if (vals == 4)
    {
        // Serial.println("100");
        SetSpeed(SPEED);
        Move();
    }
    else if (vals == 2)
    {
        SetSpeed(10);
        Serial.println("JUNCTION");
        // Serial.println("010");
        Stop();
    }
    else if (vals == 5)
    {
        // Serial.println("101");
        SetSpeed(SPEED);
        Move();
    }
    else
    {
        // Serial.println("000");
        Serial.println("LINE");
        SetSpeed(10);
        Stop();
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