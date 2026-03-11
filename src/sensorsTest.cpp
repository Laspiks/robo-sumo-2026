#include <Wire.h>

#define DIP1 A7
#define DIP2 A6
#define DIP3 A3
#define BTN  8

#define LEFT_IR          7
#define LEFT_CENTER_IR   9
#define CENTER_IR        4
#define RIGHT_CENTER_IR  A1
#define RIGHT            2
#define LEFT_QTR         A0
#define RIGHT_QTR        A2


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
    pinMode(RIGHT, INPUT);



}

void loop() {
    Serial.print("L: "); Serial.print(!digitalRead(LEFT_IR));
    Serial.print(" LC: "); Serial.print(!digitalRead(LEFT_CENTER_IR));
    Serial.print(" C: "); Serial.print(!digitalRead(CENTER_IR));
    Serial.print(" RC: "); Serial.print(!digitalRead(RIGHT_CENTER_IR));
    Serial.print(" R: "); Serial.println(!digitalRead(RIGHT));
    delay(10);
}