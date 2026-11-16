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

// Motor Driver 2: NEMA 17 Stepper Motor 
const int stepsPerRevolution = 200;

// FIXED: Pin order updated to match your documentation (IN1=3, IN2=6, IN3=2, IN4=12)
Stepper nemaMotor(stepsPerRevolution, 3, 6, 2, 12);

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

  // Set a smooth, reliable RPM for the NEMA 17
  nemaMotor.setSpeed(60);

  Serial.println("SYSTEM BOOTED & READY - Pin Order Fixed");
}

void loop() {
  // Check if an IR signal is received
  if (IrReceiver.decode()) {
    unsigned long cmd = IrReceiver.decodedIRData.command;
    
    Serial.print("Received Command: 0x");
    Serial.println(cmd, HEX);

    // If the OK button is pressed, run the movement sequence and spin for 10 seconds
    if (cmd == OK_COMMAND) {
      Serial.println("Executing: Robot Movement Sequence...");
      executeMovementSequence();

      // --- 10 SECONDS CLOCKWISE SPIN ---
      Serial.println("Spinning Stepper Clockwise for 10 seconds...");
      spinForSeconds(10.0, 1); // 1 = Clockwise
      delay(500);

      // --- TO CHANGE TO COUNTER-CLOCKWISE (CCW), JUST CHANGE 1 TO -1 BELOW ---
      // Serial.println("Spinning Stepper Counter-Clockwise for 10 seconds...");
      // spinForSeconds(10.0, -1); // -1 = Counter-Clockwise
      // delay(500);
    } 

    Serial.println("Finished command, resuming IR...");
    IrReceiver.resume();
  }
}

// Function to spin the stepper for a specific number of seconds
// seconds: duration, direction: 1 for Clockwise, -1 for Counter-Clockwise
void spinForSeconds(float seconds, int direction) {
  int currentRpm = 60; // Must match nemaMotor.setSpeed() above
  int stepsPerSecond = (currentRpm * stepsPerRevolution) / 60;
  int totalSteps = (int)(stepsPerSecond * seconds);
  
  int batchSize = 10; // Move in small batches for smooth rotation

  for (int i = 0; i < totalSteps; i += batchSize) {
    int stepsToTake = min(batchSize, totalSteps - i);
    nemaMotor.step(stepsToTake * direction);
    delay(5); // Tiny yield delay for driver switching
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