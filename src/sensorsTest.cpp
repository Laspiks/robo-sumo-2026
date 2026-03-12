#include <Wire.h>
#include <Servo.h>

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

Servo esc;


const int MPU = 0x68;
float accX, accY, accZ;
float gyroZ;

float angleZ = 0;
float tiltX = 0;


unsigned long lastTime;

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


bool readDipSwichPin(uint8_t PIN) {
    long sum = 0;

    for (int i = 0; i < 10; i++) {
        sum += analogRead(PIN);
        delay(2);
    }

    if ((sum / 10) > 1000) {
        return 1;
    } else {
        return 0;
    }

}

void setup_gyro() {
    Wire.begin();

    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    lastTime = millis();
}

void setup() {
    Serial.begin(9600);
    
    pinMode(LEFT_IR, INPUT);
    pinMode(LEFT_CENTER_IR, INPUT);
    pinMode(CENTER_IR, INPUT);
    pinMode(RIGHT_CENTER_IR, INPUT);
    pinMode(RIGHT_IR, INPUT);

    esc.attach(ESC_PIN, 1000, 2000);
    esc.write(180);
    delay(2000);
    esc.write(0);
    delay(1000);

}

void loop() {
    Serial.print("L: "); Serial.print(!digitalRead(LEFT_IR));
    Serial.print(" LC: "); Serial.print(!digitalRead(LEFT_CENTER_IR));
    Serial.print(" C: "); Serial.print(!digitalRead(CENTER_IR));
    Serial.print(" RC: "); Serial.print(!digitalRead(RIGHT_CENTER_IR));
    Serial.print(" R: "); Serial.println(!digitalRead(RIGHT_IR));
}