#include <Arduino.h>
#include <Servo.h>

// Motors
#define LEFT_MOTOR_PIN_F 11
#define LEFT_MOTOR_PIN_B 10
#define RIGHT_MOTOR_PIN_F 6
#define RIGHT_MOTOR_PIN_B 3

// Sensors
#define LEFT_IR 2
#define RIGHT_IR A0
#define LEFT_CENTER_IR 4
#define RIGHT_CENTER_IR 9
#define CENTER_IR 7

#define EDGE_LEFT A1
#define EDGE_RIGHT A2


// Control
#define BUTTON_PIN 0
#define ESC_PIN 5

// Settings
#define MAX_ATTACK_SPEED 200
#define EDGE_THRESHOLD 100
#define EDGE_TURN_SPEED 200
#define EDGE_BACK_SPEED -100
#define EDGE_BACK_RECOVER_SPEED -170
#define EDGE_TURN_TIMEOUT_MS 500
#define EDGE_SETTLE_MS 100
#define EDGE_REVERSE_MS 100
#define SEEK_LEFT_SPEED -50
#define SEEK_RIGHT_SPEED 50
#define CENTER_TURN_SPEED 150
#define CENTER_TURN_TIMEOUT_MS 500

int startLeftEdge = 0;
int startRightEdge = 0;
#define LEFT 0
#define RIGHT 1

void waitButtonPressAndRelease() {
  while (digitalRead(BUTTON_PIN) == HIGH) {
    // wait button
  }

  delay(25);

  while (digitalRead(BUTTON_PIN) == LOW) {
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
  delay(EDGE_SETTLE_MS);

  robotControl(EDGE_BACK_SPEED, EDGE_BACK_SPEED);
  delay(EDGE_REVERSE_MS);

  if (leftEdge) {
    waitUntilEdgeClears(isLeftEdge);
  } else {
    waitUntilEdgeClears(isRightEdge);
  }
}

Servo esc;

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
  CalibrateEdgeSensors();
}

void loop() {
  Serial.print(analogRead(EDGE_LEFT)); Serial.print(" ");
  Serial.println(analogRead(EDGE_RIGHT));

  if (isLeftEdge()) {
    handleEdge(LEFT);
    return;
  } 
  else if (isRightEdge()) {
    handleEdge(RIGHT);
    return;
  }

  if (!digitalRead(CENTER_IR)) {
    for (int i = 0; i < MAX_ATTACK_SPEED; i++) {
      robotControl(i, i);
      delay(1);  
    }
    while (!digitalRead(CENTER_IR) && (!isLeftEdge() && !isRightEdge()));
  }

  else if (!digitalRead(LEFT_CENTER_IR)) {
    const uint32_t attackTimestamp = millis();
    robotControl(0, CENTER_TURN_SPEED);
    while (digitalRead(CENTER_IR)) {
      if (millis() - attackTimestamp > CENTER_TURN_TIMEOUT_MS) {
        break;
      }
    }
    
  }
  else if (!digitalRead(RIGHT_CENTER_IR)) {
    const uint32_t attackTimestamp = millis();
    robotControl(CENTER_TURN_SPEED, 0);
    while (digitalRead(CENTER_IR)) {
      if (millis() - attackTimestamp > CENTER_TURN_TIMEOUT_MS) {
        break;
      }
    }
    

  }
  else {
    robotControl(SEEK_LEFT_SPEED, SEEK_RIGHT_SPEED);
  }
}
