#include <Ps3Controller.h>
#include <ESP32Servo.h>  // by Kevin Harrington

#define steeringServoPin 23
#define mastTiltServoPin 22
#define cabLights 32
#define auxLights 33

#define mastMotor0 25  // Used for controlling auxiliary attachment movement
#define mastMotor1 26  // Used for controlling auxiliary attachment movement
#define auxAttach0 18  // Used for controlling auxiliary attachment movement
#define auxAttach1 17  // Used for controlling auxiliary attachment movement

#define leftMotor0 21   // Used for controlling the left motor movement
#define leftMotor1 19   // Used for controlling the left motor movement
#define rightMotor0 33  // Used for controlling the right motor movementc:\Users\JohnC\Desktop\SOLIDWORKS Connected.lnk
#define rightMotor1 32  // Used for controlling the right motor movement

Servo steeringServo;
Servo mastTiltServo;

int servoDelay = 0;
int lightSwitchTime = 0;

float adjustedSteeringValue = 86;
float adjustedThrottleValue = 0;
float steeringAdjustment = 1;
int steeringTrim = 0;

int mastTiltValue = 90;
int mastTilt = 0;


bool lightsOn = false;
bool moveMastTiltServoDown = false;
bool moveMastTiltServoUp = false;
bool hardLeft;
bool hardRight;


void notify() {
  //--------------- Digital D-pad button events --------------
  if (Ps3.event.button_down.up) {
    Serial.println("Started pressing the up button");
    mastTilt = 1;
  }
  if (Ps3.event.button_down.down) {
    Serial.println("Started pressing the down button");
    mastTilt = 2;
  }
  if (Ps3.event.button_up.up) {
    Serial.println("Released the up button");
    mastTilt = 0;
  }
  if (Ps3.event.button_up.down) {
    Serial.println("Released the down button");
    mastTilt = 0;
  }

  //---------------- Analog stick value events ---------------
  if (abs(Ps3.event.analog_changed.stick.lx) + abs(Ps3.event.analog_changed.stick.ly) > 2) {
    // Serial.print("Moved the left stick:");
    // Serial.print(" x=");
    // Serial.print(Ps3.data.analog.stick.lx, DEC);
    // Serial.print(" y=");
    // Serial.print(Ps3.data.analog.stick.ly, DEC);
    // Serial.println();
    int LYValue = Ps3.data.analog.stick.ly * 2;
    processThrottle(LYValue);
  }

  if (abs(Ps3.event.analog_changed.stick.rx) + abs(Ps3.event.analog_changed.stick.ry) > 2) {
    //  Serial.print("Moved the right stick:");
    // Serial.print(" x=");
    // Serial.print(Ps3.data.analog.stick.rx, DEC);
    // Serial.print(" y=");
    // Serial.print(Ps3.data.analog.stick.ry, DEC);
    // Serial.println();
    int RXValue = (Ps3.data.analog.stick.rx);
    adjustedSteeringValue = 90 - (RXValue / 3);
    int RYValue = (Ps3.data.analog.stick.ry);
    steeringServo.write(adjustedSteeringValue + steeringTrim);

    if (adjustedSteeringValue > 100) {
      steeringAdjustment = ((200 - adjustedSteeringValue) / 100);
    } else if (adjustedSteeringValue < 80) {
      steeringAdjustment = ((200 - (90 + (90 - adjustedSteeringValue))) / 100);
    }
    processThrottle(adjustedThrottleValue);

    if (RYValue > 100 || RYValue < -100) {
      moveMotor(mastMotor0, mastMotor1, RYValue);
    } else {
      moveMotor(mastMotor0, mastMotor1, 0);
    }
  }
  //------------------------shoulder buttons events ----------------
  if (Ps3.event.button_down.r1) {
    if (steeringTrim < 20) {
      steeringTrim = steeringTrim + 2;
      steeringServo.write(adjustedSteeringValue + steeringTrim);
      delay(50);
    }
  }
  if (Ps3.event.button_down.l1) {
    if (steeringTrim > -20) {
      steeringTrim = steeringTrim - 2;
      steeringServo.write(adjustedSteeringValue + steeringTrim);
      delay(50);
    }
  }
  //------------------------trigger buttons events ----------------
  if (Ps3.event.button_down.l2) {
    hardLeft = true;
    processThrottle(adjustedThrottleValue);
    delay(10);
    Serial.println("Started pressing the left trigger button");
  }
  if (Ps3.event.button_up.l2) {
    hardLeft = false;
    processThrottle(adjustedThrottleValue);
    delay(10);
    Serial.println("Released the left trigger button");
  }
  if (Ps3.event.button_down.r2) {
    hardRight = true;
    processThrottle(adjustedThrottleValue);
    delay(10);
    Serial.println("Started pressing the right trigger button"); 
  }
  if (Ps3.event.button_up.r2) {
    hardRight = false;
    processThrottle(adjustedThrottleValue);
    delay(10);
    Serial.println("Released the right trigger button");
  }

  //------------------------ Joystick Button events ----------------
  if (Ps3.event.button_down.r3) {
    if ((millis() - lightSwitchTime) > 200) {
      if (lightsOn) {
        digitalWrite(auxAttach0, LOW);
        digitalWrite(auxAttach1, LOW);
        lightsOn = false;
      } else {
        digitalWrite(auxAttach0, HIGH);
        digitalWrite(auxAttach1, LOW);
        lightsOn = true;
      }
      lightSwitchTime = millis();
    }
    Serial.println("Started pressing the right stick button");
  }
  if (servoDelay >= 5) {
    if (mastTilt == 1 && mastTiltValue >= 10 && mastTiltValue < 170) {
      mastTiltValue = mastTiltValue + 1;
      mastTiltServo.write(mastTiltValue);
      servoDelay = 0;
    }
    if (mastTilt == 2 && mastTiltValue <= 170 && mastTiltValue > 10) {
      mastTiltValue = mastTiltValue - 1;
      mastTiltServo.write(mastTiltValue);
      servoDelay = 0;
    }
  }
  servoDelay++;
}
void processThrottle(int throttleValue) {
  adjustedThrottleValue = throttleValue;
  int deadZoneValue = 60;
  if (adjustedThrottleValue > deadZoneValue || adjustedThrottleValue < -deadZoneValue) {
    if (hardRight) {
      moveMotor(rightMotor0, rightMotor1, -1 * (adjustedThrottleValue * steeringAdjustment));
    } else if (hardLeft) {
      moveMotor(leftMotor0, leftMotor1, -1 * (adjustedThrottleValue * steeringAdjustment));
    } else if (adjustedSteeringValue > 100) {
      moveMotor(leftMotor0, leftMotor1, adjustedThrottleValue * steeringAdjustment);
      moveMotor(rightMotor0, rightMotor1, adjustedThrottleValue);
    } else if (adjustedSteeringValue < 80) {
      moveMotor(leftMotor0, leftMotor1, adjustedThrottleValue);
      moveMotor(rightMotor0, rightMotor1, adjustedThrottleValue * steeringAdjustment);
    } else {
      moveMotor(leftMotor0, leftMotor1, adjustedThrottleValue);
      moveMotor(rightMotor0, rightMotor1, adjustedThrottleValue);
    }
  } else {
    moveMotor(leftMotor0, leftMotor1, 0);
    moveMotor(rightMotor0, rightMotor1, 0);
  }
}

void moveMotor(int motorPin0, int motorPin1, int velocity) {
  if (velocity > 15) {
    analogWrite(motorPin0, velocity);
    analogWrite(motorPin1, LOW);
  } else if (velocity < -15) {
    analogWrite(motorPin0, LOW);
    analogWrite(motorPin1, (-1 * velocity));
  } else {
    analogWrite(motorPin0, 0);
    analogWrite(motorPin1, 0);
  }
}

void onConnect() {
  Serial.println("Connected.");
}

void setup() {

  Serial.begin(115200);

  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.begin("8c:7c:b5:fc:3b:39");
  Serial.println("Ready.");

  pinMode(auxAttach0, OUTPUT);
  pinMode(auxAttach1, OUTPUT);
  digitalWrite(auxAttach0, LOW);
  digitalWrite(auxAttach1, LOW);
  pinMode(leftMotor0, OUTPUT);
  pinMode(leftMotor1, OUTPUT);
  pinMode(rightMotor0, OUTPUT);
  pinMode(rightMotor1, OUTPUT);
  pinMode(mastMotor0, OUTPUT);
  pinMode(mastMotor1, OUTPUT);


  steeringServo.attach(steeringServoPin);
  steeringServo.write(adjustedSteeringValue);

  mastTiltServo.attach(mastTiltServoPin);
  mastTiltServo.write(mastTiltValue);
}
void loop() {
  if (!Ps3.isConnected())
    return;
  delay(500);
}
