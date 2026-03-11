#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>


// Motors
#define LEFT_MOTOR_PIN_F 11
#define LEFT_MOTOR_PIN_B 10
#define RIGHT_MOTOR_PIN_F 6
#define RIGHT_MOTOR_PIN_B 3

// Sensors
#define LEFT_IR 7
#define RIGHT_IR 2
#define LEFT_CENTER_IR 9
#define RIGHT_CENTER_IR A1
#define CENTER_IR 4

#define EDGE_LEFT  A0
#define EDGE_RIGHT A2


// Control
#define DIP1 A7
#define DIP2 A6
#define DIP3 A3
#define BTN 8
#define ESC_PIN 5

// Settings
#define MAX_ATTACK_SPEED 255
#define SEARCH_SPEED 120
#define CENTER_TURN_SPEED 200
#define EDGE_TURN_SPEED 200
#define EDGE_BACK_SPEED -100
#define EDGE_BACK_RECOVER_SPEED -170


#define EDGE_THRESHOLD 100
#define CENTER_TURN_TIMEOUT_MS 500
#define EDGE_TURN_TIMEOUT_MS 500




#define LEFT 0
#define RIGHT 1
int startLeftEdge = 0;
int startRightEdge = 0;
int searchDir = LEFT;
const int MPU = 0x68;
float accX, accY, accZ;
float gyroZ;
float angleZ = 0;
float tiltX = 0;
unsigned long lastTime;
Servo esc;

void waitButtonPressAndRelease() {
  while (digitalRead(BTN) == HIGH) {
    // wait button
  }

  delay(25);

  while (digitalRead(BTN) == LOW) {
    // wait until released
  }
}

void setMotor(uint8_t pinForward, uint8_t pinBackward, int speed) {
  if (speed > 0) {
    analogWrite(pinForward, speed);
    analogWrite(pinBackward, 0);
  } else if (speed < 0) {
    analogWrite(pinForward, 0);
    analogWrite(pinBackward, -speed);
  } else {
    analogWrite(pinForward, 0);
    analogWrite(pinBackward, 0);
  }
}

void LeftMotor(int speed) {
  setMotor(LEFT_MOTOR_PIN_F, LEFT_MOTOR_PIN_B, speed);
}

void RightMotor(int speed) {
  setMotor(RIGHT_MOTOR_PIN_F, RIGHT_MOTOR_PIN_B, speed);
}

void robotControl(int leftSpeed, int rightSpeed) {
  LeftMotor(leftSpeed);
  RightMotor(rightSpeed);
}

void stopMotors() {
  robotControl(0, 0);
}

void CalibrateEdgeSensors() {
  startLeftEdge = analogRead(EDGE_LEFT);
  startRightEdge = analogRead(EDGE_RIGHT);
}

bool isLeftEdge() {
  return abs(analogRead(EDGE_LEFT) - startLeftEdge) > EDGE_THRESHOLD;
}

bool isRightEdge() {
  return abs(analogRead(EDGE_RIGHT) - startRightEdge) > EDGE_THRESHOLD;
}

void waitUntilEdgeClears(bool (*edgeDetected)()) {
  while (edgeDetected()) {
    robotControl(EDGE_BACK_RECOVER_SPEED, EDGE_BACK_RECOVER_SPEED);
  }
}

void handleEdge(bool leftEdge) {
  const uint32_t detectTimestamp = millis();
  if (leftEdge) {
    robotControl(0, EDGE_TURN_SPEED);
    while (!isRightEdge()) {
      if (millis() - detectTimestamp > EDGE_TURN_TIMEOUT_MS) {
        break;
      }
    }
  } else {
    robotControl(EDGE_TURN_SPEED, 0);
    while (!isLeftEdge()) {
      if (millis() - detectTimestamp > EDGE_TURN_TIMEOUT_MS) {
        break;
      }
    }
  }

  stopMotors();
  delay(100);

  robotControl(EDGE_BACK_SPEED, EDGE_BACK_SPEED);
  delay(100);

  if (leftEdge) {
    waitUntilEdgeClears(isLeftEdge);
  } else {
    waitUntilEdgeClears(isRightEdge);
  }
  stopMotors();
  delay(100);
}

void setup_gyro() {
    Wire.begin();

    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    lastTime = millis();
}

void updateGyro() {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B); 
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 14, true);

    int16_t ax = Wire.read() << 8 | Wire.read();
    int16_t ay = Wire.read() << 8 | Wire.read();
    int16_t az = Wire.read() << 8 | Wire.read();

    Wire.read(); Wire.read(); // пропускаем температуру

    int16_t gx = Wire.read() << 8 | Wire.read();
    int16_t gy = Wire.read() << 8 | Wire.read();
    int16_t gz = Wire.read() << 8 | Wire.read();

    accX = ax / 16384.0;
    accY = ay / 16384.0;
    accZ = az / 16384.0;

    gyroZ = gz / 131.0;

    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    lastTime = now;

    angleZ += gyroZ * dt;

    tiltX = atan2(accY, accZ) * 180 / PI;

    Serial.print("Yaw: ");
    Serial.print(angleZ);

    Serial.print("   TiltX: ");
    Serial.println(tiltX);
}


void setup() {
  Serial.begin(9600);
  pinMode(LEFT_IR, INPUT);
  pinMode(LEFT_CENTER_IR, INPUT);
  pinMode(CENTER_IR, INPUT);
  pinMode(RIGHT_CENTER_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  pinMode(EDGE_LEFT, INPUT);
  pinMode(EDGE_RIGHT, INPUT);

  pinMode(LEFT_MOTOR_PIN_F, OUTPUT);
  pinMode(LEFT_MOTOR_PIN_B, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN_F, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN_B, OUTPUT);
  
  pinMode(DIP1, INPUT);
  pinMode(DIP2, INPUT);
  pinMode(DIP3, INPUT);
  pinMode(BTN, INPUT_PULLUP);

  setup_gyro();
  // esc.attach(ESC_PIN, 1000, 2000);
  // esc.write(180);
  // delay(2000);
  // esc.write(0);
  // delay(1000);

  // esc.write(40);
  // for (int i = 0; i < 150; i++) {
  //   esc.write(i);
  //   delay(50);
  // }
  waitButtonPressAndRelease();
  CalibrateEdgeSensors();
  delay(2000);

  // angleZ = 0;
  // robotControl(-40, 40);
  // while (abs(angleZ) <= 90) {
  //   updateGyro();
  // }
  
  // stopMotors();
  // while (1) {};
  

}

void loop() {
  // Serial.print(analogRead(EDGE_LEFT)); Serial.print(" ");
  // Serial.println(analogRead(EDGE_RIGHT));

  // if (isLeftEdge()) {
  //   handleEdge(LEFT);
  //   return;
  // } 
  // else if (isRightEdge()) {
  //   handleEdge(RIGHT);
  //   return;
  // }

  if (!digitalRead(CENTER_IR)) {
    for (int i = 0; i < MAX_ATTACK_SPEED; i++) {
      robotControl(i, i);
      delay(1);  
    }
    // while (!digitalRead(CENTER_IR) && (!isLeftEdge() && !isRightEdge()));
    while (!digitalRead(CENTER_IR));

  }
  else if (!digitalRead(LEFT_CENTER_IR)) {
    searchDir = LEFT;
    const uint32_t attackTimestamp = millis();
    robotControl(0, CENTER_TURN_SPEED);
    while (digitalRead(CENTER_IR)) {
      if (millis() - attackTimestamp > CENTER_TURN_TIMEOUT_MS) {
        break;
      }
    }
    
  }
  else if (!digitalRead(RIGHT_CENTER_IR)) {
    searchDir = RIGHT;
    const uint32_t attackTimestamp = millis();
    robotControl(CENTER_TURN_SPEED, 0);
    while (digitalRead(CENTER_IR)) {
      if (millis() - attackTimestamp > CENTER_TURN_TIMEOUT_MS) {
        break;
      }
    }
    

  }
  else if (!digitalRead(LEFT_IR)) {
    searchDir = LEFT;
    robotControl(-255, 255);
    delay(140);
    stopMotors();
    delay(50);

  }
  else if (!digitalRead(RIGHT_IR)) {
    searchDir = RIGHT;
    robotControl(255, -255);
    delay(140);
    stopMotors();
    delay(50);

  }
  else {
    if (searchDir == LEFT)
      robotControl(-SEARCH_SPEED, SEARCH_SPEED);
    else 
      robotControl(SEARCH_SPEED, -SEARCH_SPEED);
  }
}
