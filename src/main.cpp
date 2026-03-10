#include <Arduino.h>
#include <Servo.h>
#include <DFRobot_BMI160.h>
#include <Wire.h>

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


int startLeftEdge = 0;
int startRightEdge = 0;

void waitButtonPressAndRelease() {
  while (digitalRead(BUTTON_PIN) == HIGH) {
    // wait button
  }

  delay(25);

  while (digitalRead(BUTTON_PIN) == LOW) {
    // wait until released
  }
}

void LeftMotor(int speed) {
  if (speed > 0) {
    analogWrite(LEFT_MOTOR_PIN_F, speed);
    analogWrite(LEFT_MOTOR_PIN_B, 0);

  } else if (speed < 0) {
    analogWrite(LEFT_MOTOR_PIN_F, 0);
    analogWrite(LEFT_MOTOR_PIN_B, speed * -1);

  } else {
    analogWrite(LEFT_MOTOR_PIN_F, 0);
    analogWrite(LEFT_MOTOR_PIN_B, 0);

  }
}

void RightMotor(int speed) {
  if (speed > 0) {
    analogWrite(RIGHT_MOTOR_PIN_F, speed);
    analogWrite(RIGHT_MOTOR_PIN_B, 0);

  } else if (speed < 0) {
    analogWrite(RIGHT_MOTOR_PIN_F, 0);
    analogWrite(RIGHT_MOTOR_PIN_B, speed * -1);

  } else {
    analogWrite(RIGHT_MOTOR_PIN_F, 0);
    analogWrite(RIGHT_MOTOR_PIN_B, 0);

  }
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
    uint32_t detectTimestamp = millis();
    LeftMotor(0);
    RightMotor(200);
    while (!isRightEdge()) {
      if (millis() - detectTimestamp > 500) {
        break;
      }
    }

    LeftMotor(0);
    RightMotor(0);
    delay(100);

    LeftMotor(-100);
    RightMotor(-100);
    delay(100);
    
    while (isLeftEdge()) {
      LeftMotor(-170);
      RightMotor(-170);
      
    }
    return;
  } 
  else if (isRightEdge()) {
    uint32_t detectTimestamp = millis();
    LeftMotor(200);
    RightMotor(0);
    while (!isLeftEdge()) {
      if (millis() - detectTimestamp > 500) {
        break;
      }
    }
    LeftMotor(0);
    RightMotor(0);
    delay(100);
    
    LeftMotor(-100);
    RightMotor(-100);
    delay(100);
    
    while (isLeftEdge()) {
      LeftMotor(-170);
      RightMotor(-170);
      
    }
    return;
  }

  if (!digitalRead(CENTER_IR)) {
    for (int i = 0; i < MAX_ATTACK_SPEED; i++) {
      LeftMotor(i);
      RightMotor(i);
      delay(1);  
    }
    while (!digitalRead(CENTER_IR) && (!isLeftEdge() && !isRightEdge()));
  }

  else if (!digitalRead(LEFT_CENTER_IR)) {
    uint32_t attackTimestamp = millis();
    LeftMotor(0);
    RightMotor(150);
    while (digitalRead(CENTER_IR)) {
      if (millis() - attackTimestamp > 500) {
        break;
      }
    }
    
  }
  else if (!digitalRead(RIGHT_CENTER_IR)) {
    uint32_t attackTimestamp = millis();
    LeftMotor(150);
    RightMotor(0);
    while (digitalRead(CENTER_IR)) {
      if (millis() - attackTimestamp > 500) {
        break;
      }
    }
    

  }
  else {
    LeftMotor(-50);
    RightMotor(50);
  }
}