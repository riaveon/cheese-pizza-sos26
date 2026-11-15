#include <IRremote.hpp>
#include <Stepper.h>

// --- PIN DEFINITIONS & CONSTANTS ---
const int IR_RECEIVE_PIN = 11;

// Motor Driver 1: Four TT Gear Motors (Paired left/right)
const int in1 = 4;
const int in2 = 5;
const int in3 = 7;
const int in4 = 8;

// Ultrasonic Sensor Pins (HC-SR04)
const int trigPin = 9;
const int echoPin = 10;

// Motor Driver 2: NEMA 17 Stepper Motor (Wired to physical pins 2, 3, 6, 12)
const int stepsPerRevolution = 200;

// Optimized H-Bridge mapping order for Stepper.h with L298N
Stepper nemaMotor(stepsPerRevolution, 2, 3, 6, 12);

// IR Remote OK button command hex value
const unsigned long OK_COMMAND = 0x40;

// Obstacle stop distance threshold in centimeters
const int STOP_DISTANCE_CM = 5; 

void setup() {
  Serial.begin(9600);
  
  // Setup DC motor pins for the 4 TT motors
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Setup Ultrasonic pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Start IR Receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // Set a moderate, reliable RPM for the L298N driver
  nemaMotor.setSpeed(75);

  Serial.println("SYSTEM BOOTED & READY - Stepper Optimized");
}

void loop() {
  // Check if an IR signal is received
  if (IrReceiver.decode()) {
    unsigned long cmd = IrReceiver.decodedIRData.command;
    
    Serial.print("Received Command: 0x");
    Serial.println(cmd, HEX);

    // If the OK button is pressed, run the movement sequence and smooth stepper rotation
    if (cmd == OK_COMMAND) {
      Serial.println("Executing: Robot Movement Sequence...");
      executeMovementSequence();

      // Spin the NEMA 17 stepper smoothly in a full circle (one complete revolution)
      Serial.println("Spinning Stepper Smoothly in a Full Circle (Clockwise)...");
      spinSmoothCircle(stepsPerRevolution); 
      delay(400);

      // Spin back in the opposite direction
      Serial.println("Spinning Stepper Smoothly in a Full Circle (Counter-Clockwise)...");
      spinSmoothCircle(-stepsPerRevolution); 
      delay(400);
    } 

    Serial.println("Finished command, resuming IR...");
    IrReceiver.resume();
  }
}

// Custom wrapper function to break a full revolution into smaller, fluid increments
// This prevents the driver from missing steps or locking up under continuous load.
void spinSmoothCircle(int totalSteps) {
  int direction = (totalSteps > 0) ? 1 : -1;
  int absSteps = abs(totalSteps);
  int batchSize = 10; // Move in small batches for smoother execution

  for (int i = 0; i < absSteps; i += batchSize) {
    int stepsToTake = min(batchSize, absSteps - i);
    nemaMotor.step(stepsToTake * direction);
    // Tiny yield delay to let the L298N H-bridge transistors switch cleanly
    delay(5); 
  }
}

// Function to handle car movement using all 4 TT motors
void executeMovementSequence() {
  int calibrationTimeMs = 300; 

  Serial.println("-> Phase 1: Moving forward briefly...");
  moveMotorsForward();
  delay(calibrationTimeMs);

  Serial.println("-> Phase 2: Ultrasonic sensor active, looking for object...");
  
  // Keep moving forward until an object is closer than STOP_DISTANCE_CM
  while (getDistanceCm() > STOP_DISTANCE_CM) {
    moveMotorsForward();
    delay(50); 
  }

  stopMotors();
  Serial.println("-> Target reached! Motors OFF.");
}

// Drive all 4 TT motors forward simultaneously
void moveMotorsForward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

// Stop all 4 TT motors
void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// Measure distance using HC-SR04
long getDistanceCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distanceCm = duration * 0.034 / 2;

  return distanceCm;
}