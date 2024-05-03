#include <Servo.h>
#include <NewPing.h>

// Motor control pins
const int LeftMotorForward = 2;
const int LeftMotorBackward = 3;
const int RightMotorForward = 4;
const int RightMotorBackward = 5;

// Buzzer pin
const int speakerPin = 7;

// Leds pins
const int whiteLed = 10;
const int blueLed = 11;

//############ NEW ################
// Battery level pins
const int batteryPin = A0;
const int greenLedPin = A5;
const int yellowLedPin = A4;
const int redLedPin = A3;

// IR sensors
int IRSensor1 = 9;
int IRSensor2 = 8;

boolean randomStop = random(2);
//##################################

// Ultrasonic Sensor pins
#define trig_pin A1
#define echo_pin A2

#define maximum_distance 200
boolean goesForward = false;
int distance = 100;

boolean turnLeftNext = true;

NewPing sonar(trig_pin, echo_pin, maximum_distance);

// Servos
Servo leftArm;
Servo rightArm;
Servo head;

int leftArmPos = 90;
int rightArmPos = 90;
int currentHeadPosition = 90;

unsigned long armTimer = 0;
unsigned long headTimer = 0;
unsigned long delayDuration = 600;

int armSequenceIndexL = 0;
int armSequenceL[] = {90, 60, 90, 90, 120, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
                      120, 90, 90, 60, 120, 90, 60, 90, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int armSequenceIndexR = 0;
int armSequenceR[] = {90, 60, 90, 90, 90, 180, 160, 180, 160, 180, 160, 180, 160, 90, 90, 
                      90, 90, 120, 90, 90, 90, 90, 90, 120, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180};

int headSequenceIndex = 0;
int headSequence[] = {90, 90, 90, 50, 50, 50, 90, 90, 90, 90, 90, 90, 90, 90, 150, 
                      150, 150, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90};

boolean helloPlayed = false;


void setup() {
  pinMode(RightMotorForward, OUTPUT);
  pinMode(LeftMotorForward, OUTPUT);
  pinMode(LeftMotorBackward, OUTPUT);
  pinMode(RightMotorBackward, OUTPUT);

  pinMode(speakerPin, OUTPUT);

  pinMode(whiteLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  leftArm.attach(12);
  rightArm.attach(13);
  head.attach(6);

  pinMode(IRSensor1, INPUT);
  pinMode(IRSensor2, INPUT);

  Serial.begin(9600);

  delay(2000);

  if (!helloPlayed) {
    playHello();
    helloPlayed = true;
  }
}

void loop() {
  //############ NEW ################
  checkBatteryLevel();
  int sensorStatus1 = digitalRead(IRSensor1);
  int sensorStatus2 = digitalRead(IRSensor2);
  //#################################

  distance = readPing();
  
  leftArm.write(leftArmPos);
  rightArm.write(rightArmPos);
  head.write(currentHeadPosition);

  digitalWrite(whiteLed, LOW);
  digitalWrite(blueLed, HIGH);


  //If obstacle detected within 20 centi
  if (distance <= 50) {
    digitalWrite(blueLed, LOW);
    digitalWrite(whiteLed, HIGH);

    playUhOh(); //"Uh-oh"

    moveStop();
    delay(600);
    moveBackward();
    delay(600);
    moveStop();
    delay(400);

    //While obstacle still detected -> turn right in place
    while (distance <= 60) {
      if (turnLeftNext) {
        turnLeft();
      } else {
        turnRight();
      }
      delay(500);
      moveStop();
      delay(400);
      distance = readPing();
    }

    digitalWrite(whiteLed, LOW);
    digitalWrite(blueLed, HIGH);
    moveStop(); 
    delay(2100);

    //Once obstacle-free space is found -> move forward
    moveForward();
    delay(500);

    //Toggle the turning direction for the next encounter
    toggleTurnDirection();
    
  } else if ((sensorStatus1 == LOW && sensorStatus2 == HIGH) || (sensorStatus1 == HIGH && sensorStatus2 == LOW)
              || (sensorStatus1 == LOW && sensorStatus2 == LOW)) {
    moveStop();
    delay(300);
    while ((sensorStatus1 == LOW && sensorStatus2 == HIGH) || (sensorStatus1 == HIGH && sensorStatus2 == LOW)) {
      if (sensorStatus1 == LOW && sensorStatus2 == HIGH) {
        moveStop();
        moveToPosition(head, 20);
        delay(300);
        turnLeft();
        delay(600);
        moveForward();
        delay(300);
        moveStop();
        delay(500);
        moveToPosition(head, 90);
        delay(150);
      } else if (sensorStatus1 == HIGH && sensorStatus2 == LOW) {
        moveStop();
        moveToPosition(head, 160);
        delay(300);
        turnRight();
        delay(600);
        moveForward();
        delay(300);
        moveStop();
        delay(500);
        moveToPosition(head, 90);
        delay(150);
      } else if (sensorStatus1 == LOW || sensorStatus2 == LOW) {
        moveStop();
        moveToPosition(head, 90);
        delay(600);
      }
      sensorStatus1 = digitalRead(IRSensor1);
      sensorStatus2 = digitalRead(IRSensor2);
    }
    delay(1000);
  } else {
    moveForward();

    //Move the arms according to the sequence
    if (millis() - armTimer >= delayDuration) {
      leftArmPos = armSequenceL[armSequenceIndexL];
      rightArmPos = armSequenceR[armSequenceIndexR];
      moveToPosition(leftArm, leftArmPos); // Move left arm to position
      moveToPosition(rightArm, rightArmPos); // Move right arm to position
      armSequenceIndexL = (armSequenceIndexL + 1) % (sizeof(armSequenceL) / sizeof(armSequenceL[0]));
      armSequenceIndexR = (armSequenceIndexR + 1) % (sizeof(armSequenceR) / sizeof(armSequenceR[0]));
      armTimer = millis();
    }

    //Move the head according to the sequence
    if (millis() - headTimer >= delayDuration) {
      currentHeadPosition = headSequence[headSequenceIndex];
      moveToPosition(head, currentHeadPosition); // Move head to position
      headSequenceIndex = (headSequenceIndex + 1) % (sizeof(headSequence) / sizeof(headSequence[0]));
      headTimer = millis();
    }
  }
}

int readPing() {
  delay(70);
  int cm = sonar.ping_cm();
  if (cm == 0) {
    cm = 250; // Max dist
  }
  return cm;
}

void moveStop() {
  digitalWrite(RightMotorForward, LOW);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorBackward, LOW);
  digitalWrite(LeftMotorBackward, LOW);
}

void moveForward() {
  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(RightMotorForward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorBackward, LOW);
}

void moveBackward() {
  digitalWrite(LeftMotorBackward, HIGH);
  digitalWrite(RightMotorBackward, HIGH);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorForward, LOW);
}

void turnRight() {
  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(RightMotorBackward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorForward, LOW);
}

void turnLeft() {
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorBackward, LOW);
  digitalWrite(LeftMotorBackward, HIGH);
  digitalWrite(RightMotorForward, HIGH);
}

void toggleTurnDirection() {
  turnLeftNext = !turnLeftNext;
}


//############ NEW ################
//Convert ADC reading to voltage
float adcToVoltage(int adcValue) {
  float voltage = adcValue * (5.0 / 1023.0);
  return voltage;
}

//Check battery level and set LED
void checkBatteryLevel() {
  int adcValue = analogRead(batteryPin);
  float batteryVoltage = adcToVoltage(adcValue);

  if (batteryVoltage > 4.7) {
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(yellowLedPin, LOW);
    digitalWrite(redLedPin, LOW);
  } else if (batteryVoltage > 4.2) {
    digitalWrite(greenLedPin, LOW);
    digitalWrite(yellowLedPin, HIGH);
    digitalWrite(redLedPin, LOW);
  } else {
    digitalWrite(greenLedPin, LOW);
    digitalWrite(yellowLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
  }
}

void moveToPosition(Servo servo, int targetPosition) {
  int currentPosition = servo.read();
  if (currentPosition < targetPosition) {
    for (int i = currentPosition; i <= targetPosition; i++) {
      servo.write(i);
      delay(21);
    }
  } else {
    for (int i = currentPosition; i >= targetPosition; i--) {
      servo.write(i);
      delay(21);
    }
  }
}
//######################################

void playUhOh() {
  tone(speakerPin, 523, 250); // plays C5 (U sound)
  delay(300);
  tone(speakerPin, 415, 500); // plays G#4 (O sound)
  delay(500);
}

void playHello() {
  tone(speakerPin, 262, 250); // plays C4 (W sound)
  delay(350);
  tone(speakerPin, 330, 250); // plays E4 (A sound)
  delay(350);
  tone(speakerPin, 392, 250); // plays G4 (L sound)
  delay(350);
  tone(speakerPin, 523, 500); // plays C5 (L sound)
  delay(250);
  tone(speakerPin, 523, 500); // plays C5 (L sound)
  delay(550);
}