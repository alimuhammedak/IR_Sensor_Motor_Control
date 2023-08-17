#include <Arduino.h>
#define DECODE_NEC
#include <IRremote.hpp>
// #define IR_SENSOR 5 // Define IR sensor pin


// Motor Pins
int Motor1In1 = 9;
int Motor1In2 = 10;

// Motor Direction Control
bool openControl = false; // 0 = close, 1 = open // alıcıyı açmak için
byte readCode;

// IR Control
const int IR_sensor = 5; // Define IR sensor pin
IRrecv IR(IR_sensor); // Create IRrecv object to receive signals
long decode; // Variable for storing the code
byte repeatDecode = 0; // Variable for storing the repeat code
int repeatedZeros = 0;
int openRead; // Code to start

//Starting and Closing Messages
String openMessage = "Open";
String closeMessage = "Clos";

// Motor Speed Control
byte motorIn1SpeedMax = 250;
byte motorIn2SpeedMax = 250;
byte motorIn1SpeedMin = 0;
byte motorIn2SpeedMin = 0;
byte motorIn1SpeedDefault = 100;
byte motorIn2SpeedDefault = 100;
byte motorIn1Speed;
byte motorIn2Speed;
byte motorStop = 0;
byte motorDirection;

// IR Buttons
#define button_OnOff -1286807808 // Open-Close
#define button_0  0          // Repeat
#define button_1 -1420501248 // FORWARD
#define button_2 -384377088 // BACK
#define button_3 -367665408 // STOP
#define button_4 -1353654528 // Speed ++
#define button_5 -317530368 // Speed --
#define button_6 -300818688 // Default Speed
#define button_Undfined 66 // Undefined

// decode_results results; // Eski kullanım. Şimdiki kullanım IR.decodedIRData.decodedRawData

void Close()
{
  openControl = !openControl; // 0 = close, 1 = open // alıcıyı kapatmak için
  Serial.println("Closing");
}

byte RepeatDecodeControl()
{
  repeatedZeros++;
  if (repeatedZeros > 3)
  {
    delay(100);
    Serial.print("repeatDecode: ");
    Serial.println(repeatDecode);
    return repeatDecode; // If the repeat code is received 3 times, the code is returned
  }  
  Serial.print("repeatedZeros: ");
  Serial.println(repeatedZeros);
  return button_Undfined;
}

int Read()
{
  do
  {
    if(IR.decode()){ // If a message is received, this will be 1
      decode = IR.decodedIRData.decodedRawData; // Get the value of the IR reading
      IR.resume(); // Receive the next value
      delay(250);

      switch (decode)
      {
        case button_0: // Repeat
          return 0;

        case button_1: // FORWARD
          Serial.println("1");
          return 1;

        case button_2: // BACK
          Serial.println("2");
          return 2;
          
        case button_3: // STOP
          Serial.println("3");
          return 3;

        case button_4: // Speed ++
          Serial.println("4");
          return 4;

        case button_5: // Speed --
          Serial.println("5");
          return 5;

        case button_6: // Default Speed
          Serial.println("6");
          return 6;

        case button_OnOff: // Open-Close
          Serial.println("7");
          return 7;
        
        default: return button_Undfined;
      
      }
    }
  }while (!IR.available());

}

void OpenController()
{
  while (true)
  {
    openRead = Read();
    if (openRead == 7)
    {
      openControl = !openControl;
      Serial.println("Opening");
      break;
    }
  }
}

void MotorDirectionControl(byte direction)
{
  switch (direction)
  {
    case 1: //Forward

      if (motorDirection == 1) 
      {
        Serial.println("Forward"); 
        analogWrite(Motor1In1, motorIn1Speed);
        Serial.println(motorIn1Speed);
        analogWrite(Motor1In2, motorIn2SpeedMin);
        Serial.println(motorIn2SpeedMin);
        delay(100);
      }
      else
      {
        motorIn1Speed = motorIn1SpeedDefault;
        motorDirection = 1;
        Serial.println("Forward"); 
        analogWrite(Motor1In1, motorIn1Speed);
        Serial.println(motorIn1Speed);
        analogWrite(Motor1In2, motorIn2SpeedMin);
        Serial.println(motorIn2SpeedMin);
        delay(100);
      }
      break;

    case 2: //Back

      if (motorDirection == 2)
      {
        Serial.println("Back");
        analogWrite(Motor1In1, motorIn1SpeedMin);
        analogWrite(Motor1In2, motorIn2Speed);
        delay(1000);
      }
      else
      {
        motorIn2Speed = motorIn2SpeedDefault;
        motorDirection = 2;
        Serial.println("Back");
        analogWrite(Motor1In1, motorIn1SpeedMin);
        analogWrite(Motor1In2, motorIn2Speed);
        delay(1000);}
      break;
      
    case 3: //Stop

      motorDirection = 3;
      Serial.println("Stop");
      analogWrite(Motor1In1, motorStop);
      analogWrite(Motor1In2, motorStop);
      delay(1000);
      break;
  }  
}

void MotorSpeedControl(byte speed)
{
  switch (speed)
  {
    case 4: 
      Serial.println("Speed ++");
      if(motorDirection == 1)
      {
        if(motorIn1Speed < motorIn1SpeedMax)
        {
          motorIn1Speed += 50;
          MotorDirectionControl(1); // Forward
        }
      }
      else if(motorDirection == 2)
      {
        if(motorIn2Speed < motorIn2SpeedMax){
        motorIn2Speed += 50;
        MotorDirectionControl(2); // Back
        }
      }
      break;
    case 5:
      Serial.println("Speed --");

      if(motorDirection == 1)
      {
        if(motorIn1Speed > motorIn1SpeedMin)
        {
          motorIn1Speed -= 50;
          MotorDirectionControl(1); // Forward
        }
      }
      else if(motorDirection == 2)
      {
        if(motorIn2Speed > motorIn2SpeedMin){
        motorIn2Speed -= 50;
        MotorDirectionControl(2); // Back
        }
      }
      break;
    case 6:
      Serial.println("Speed 3");
      motorIn1Speed = motorIn1SpeedMax;
      break;
  }
}

void setup() {
  pinMode(13,OUTPUT);// LED
  pinMode(Motor1In1,OUTPUT); // Motor1In1
  pinMode(Motor1In2,OUTPUT); // Motor1In2
  pinMode(IR_sensor, INPUT); // Set IR sensor pin as input

  Serial.begin(9600);// Start the serial monitor

  digitalWrite(13,LOW); // Turn off the LED
  
  IR.enableIRIn();// Start the receiver
  IR.blink13(true); // Blink the LED when a IR signal is received
}

void loop() {

  motorIn1Speed = motorIn1SpeedDefault;
  motorIn2Speed = motorIn2SpeedDefault;

  OpenController();
  delay(1000);
  Serial.println(openMessage);
  
  while (openControl)
  {
    readCode = Read();

    if (readCode == 0) readCode = RepeatDecodeControl(); // Repeat Control
    else repeatedZeros = 0; // Repeat Control Reset

    if(readCode > 0 and readCode <= 3)
    {
      repeatDecode = readCode;
      MotorDirectionControl(readCode); // Forward, Back, Stop
    }

    else if(readCode >= 4 and readCode < 7){
      repeatDecode = readCode;
      MotorSpeedControl(readCode); // Speed ++, Speed --, Default Speed
    } 
      
    else if(readCode == 7) // Close
    {
      Close();
      delay(1000);
      repeatDecode = 0;
      MotorDirectionControl(3); // Stop
      Serial.println(closeMessage);
      break;
    }
  }

}
