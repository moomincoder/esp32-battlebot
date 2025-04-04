#include <PS4Controller.h>
#include <ESP32Servo.h>
#include <Arduino.h>

// Motor control pins
const int ENA = 32; // Left motor PWM
const int IN1 = 25;
const int IN2 = 26;

const int ENB = 33; // Right motor PWM
const int IN3 = 27;
const int IN4 = 14;

const int ESC_PIN = 13; // Brushless ESC pin

// LEDC PWM channels
#define CH_LEFT  0
#define CH_RIGHT 1

// Deadzone threshold
const int DEADZONE = 10;

Servo esc;

void setup() {
  Serial.begin(115200);

  // Motor direction pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // PWM setup (Core 2.x compatible)
  ledcSetup(CH_LEFT, 1000, 8);
  ledcAttachPin(ENA, CH_LEFT);

  ledcSetup(CH_RIGHT, 1000, 8);
  ledcAttachPin(ENB, CH_RIGHT);

  // ESC setup
  esc.setPeriodHertz(50);
  esc.attach(ESC_PIN, 1000, 2000);
  esc.writeMicroseconds(1000); // Start with idle signal

  // PS4 controller
  PS4.begin("78:2b:46:d6:02:0d");
  Serial.println("Waiting for PS4 controller...");
}

void loop() {
  if (PS4.isConnected()) {
    int leftY = PS4.LStickY();
    int rightY = PS4.RStickY();

    // Apply deadzone
    leftY = (abs(leftY) < DEADZONE) ? 0 : leftY;
    rightY = (abs(rightY) < DEADZONE) ? 0 : rightY;

    int leftSpeed = map(leftY, -128, 127, 255, -255);
    int rightSpeed = map(rightY, -128, 127, 255, -255);

    controlMotor(CH_LEFT, IN1, IN2, leftSpeed);
    controlMotor(CH_RIGHT, IN3, IN4, rightSpeed);

    // If left bumper (L1) is pressed, set ESC to half power
    if (PS4.L1()) {
      esc.writeMicroseconds(2000); // Set ESC to half power
      Serial.println("ESC set to half power");
    } else {
      esc.writeMicroseconds(1000); // Set ESC to idle
    }
  }
}

void controlMotor(int channel, int in1, int in2, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    ledcWrite(channel, speed);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(channel, -speed);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(channel, 0);
  }
}
