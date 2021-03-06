/*
  Make a set of motors move to sound.
  Uses an Arduino Uno paired with the Adafruit Motor Shield v2.
  Written with the larger Big Mouth Billy Bass (or any animatronic) in mind.
   
   Modified by Joe Villegas on July 2018:
   -Defined all motors
   -Set variables (Ex: speed and sensitivity) for easier tweaking
   -Added support for the third motor in some Basses (Disable/Enable third motor use)
   -Included serial printouts for debugging and audio reading monitoring

   August 2018:
   -Added frantic mode (Maximum speed of motor movement, requires lots of power)
   
   Original information:
   Created by Donald Bell, Maker Project Lab (2016).
   Based on Sound to Servo by Cenk Ã–zdemir (2012)
   and DCMotorTest by Adafruit
*/

// Libraries
#include <Wire.h>
#include <QueueArray.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

//* SETUP (!!! This is the usually the best and probably only place you need to adjust!)
boolean legacyFish = true; //For older models that incorporate three motors
//* Motor speed variables
int motorDelay = 0; //Delay between sound readings and motor movements
//* Range: 1 - 255. Lower makes most movement.
int mouthMotorSpeed = 100;
int headMotorSpeed = 100;
int tailMotorSpeed = 25;
// Funsies (ENABLE AT RISK (make sure you have power)
boolean modeFrantic = false;
int logDelay = 10;
//* SETUP finished

//Base system variables
int afmsFreq = 1600; //Default is 1600 (1.6kHz)
int audioSensitivity = 1023;

//Audio threshold variables
boolean headMovementEnabled = true; //TODO: head motor is unable to move the head itself (reglue?)
int staticThreshold = 1;            //To leave out any unwanted movement due to static
int mouthAudioThreshold = 10;
int tailAudioThreshold = 25;

//* ---- This section below don't touch

//Handled all by system (Best to not touch)
// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Select which 'port' M1, M2, M3 or M4.
Adafruit_DCMotor *mouthMotor = AFMS.getMotor(1);
Adafruit_DCMotor *headMotor = AFMS.getMotor(2);
Adafruit_DCMotor *tailMotor = AFMS.getMotor(3);

// Some other Variables we need
int SoundInPin = A1;
int LedPin = 12; //in case you want an LED to activate while mouth moves

boolean audioDetected = false;
int audioLastDetected = 0;
QueueArray<int> audioReadingQueue;
boolean bodyMoved;

int speedFrantic = 254;
int ping = 0;

//* ---- This section above don't touch ---

//The setup routine runs once when you press reset:
void setup()
{
  //Pre-setup tweaks

  if (modeFrantic == true)
  {
    motorDelay = 0;
    mouthMotorSpeed = speedFrantic;
    headMotorSpeed = speedFrantic;
    tailMotorSpeed = speedFrantic;
    Serial.println("hello");
  }

  Serial.begin(9600); // set up Serial library at 9600 bps
  Serial.println("sup b");

  AFMS.begin(afmsFreq);
  if (afmsFreq == 1600)
  {
    Serial.println("Adafruit Motor Shield booted");
  }
  else
  {
    Serial.println("AFMS enabled with frequency of " + String(afmsFreq));
  }

  // Set the speed to start, from 0 (off) to 255 (max speed)

  //mouth motor
  mouthMotor->setSpeed(mouthMotorSpeed);
  Serial.println("Mouth motor speed set to " + String(mouthMotorSpeed) + ".");
  mouthMotor->run(FORWARD); // turn on motor
  Serial.println("Mouth motor connected, attempted to move");

  mouthMotor->run(RELEASE);
  pinMode(SoundInPin, INPUT);
  pinMode(LedPin, OUTPUT);

  //head motor
  headMotor->setSpeed(headMotorSpeed);
  Serial.println("Head motor speed set to " + String(headMotorSpeed) + ".");
  headMotor->run(FORWARD); // turn on motor
  Serial.println("Head motor connected, attempted to move");

  headMotor->run(RELEASE);
  pinMode(SoundInPin, INPUT);

  //tail motor
  if (legacyFish == true)
  {
    tailMotor->setSpeed(tailMotorSpeed);
    Serial.println("Tail motor speed set to " + String(tailMotorSpeed) + ".");
    tailMotor->run(FORWARD); // turn on motor
    Serial.println("Tail motor connected, attempted to move");

    tailMotor->run(RELEASE);
  }

  Serial.println("Billy Bass is GO!");
}

void loop()
{
  uint8_t i;

  //Reading values from analog pin
  int sensorValue = analogRead(SoundInPin);
  sensorValue = map(sensorValue, 0, 256, 0, audioSensitivity); //Sets the range of audio readings

  if (ping >= logDelay)
  {
    Serial.println("Audio - Input value: " + String(sensorValue));
    ping = 0;
  }
  else
  {
    ping++;
  }

  //int LEDValue = map(sensorValue,0,512,0,255); // we Map another value of this for LED that can be a integer betwen 0..255
  //int MoveDelayValue = map(sensorValue,0,255,0,sensorValue);  // note normally the 512 is 1023 because of analog reading should go so far, but I changed that to get better readings.

  //Passive movement (Moves the head when it detects any audio, and moves back when it becomes silent for a while)
  if (headMovementEnabled)
  {
    //Gathering history
    audioReadingQueue.enqueue(sensorValue);
    //Serial.println("QUEUE SIZE: " + String(audioReadingQueue.count()));     //DEBUG
    if (audioReadingQueue.count() == 50)
    {
      if (audioReadingQueue.peek() > staticThreshold)
      {
        audioLastDetected = 0;
        audioDetected = true;
      }
      else
      {
        audioLastDetected++;
        audioDetected = false;
      }
      audioReadingQueue.pop();
      //Serial.println("Audio - Last detected " + String(audioLastDetected) + " steps ago");      //DEBUG
    }
    //Moving head according to specific scenario
    if (audioLastDetected < 50 && bodyMoved == false)
    {
      headMotor->run(FORWARD);
      for (i = headMotorSpeed; i < 255; i++)
      {
        headMotor->setSpeed(i);
      }
      headMotor->setSpeed(0);
      bodyMoved = true;

      Serial.println("I'm ALIVEEEEE");
    }
    else if (audioLastDetected < 50)
    {

      //hold position
    }
    else if (bodyMoved == true)
    {
      headMotor->run(BACKWARD);
      for (i = headMotorSpeed; i < 255; i++)
      {
        headMotor->setSpeed(i);
      }
      headMotor->setSpeed(0);
      bodyMoved = false;

      Serial.println("Imma be sleepin");
    }
    else
    {

      //hold position
    }
  }

  //Active movement
  if (sensorValue > mouthAudioThreshold)
  {
    delay(motorDelay);
    // now move the motor
    mouthMotor->run(FORWARD);
    for (i = mouthMotorSpeed; i < 255; i++)
    {
      mouthMotor->setSpeed(i);
    }

    analogWrite(LedPin, sensorValue); //Brightens LED according to immediate audio strength

    mouthMotor->run(RELEASE);
  }

  if (sensorValue > tailAudioThreshold)
  {
    delay(motorDelay);
    // now move the motor
    tailMotor->run(BACKWARD);
    for (i = tailMotorSpeed; i < 255; i++)
    {
      tailMotor->setSpeed(i);
    }

    tailMotor->run(RELEASE);
  }

  // Done.
  // turn off the led again.
  analogWrite(LedPin, 0);
  // and this repeats all the time.
}
