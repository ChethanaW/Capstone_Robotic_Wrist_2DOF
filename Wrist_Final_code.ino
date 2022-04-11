
/*
Created on: 28th of December 2021
Created by: Chethana Wickramasinghe

 */

// libraries used
#include <AccelStepper.h>
#include <FlexCAN_T4.h>

// initializing constants
/*
without gears: 
The steps per revolution for motor we purchased is 200 (360 deg/1.8 step angle)

With gears for pitch:
Requires motors to move in opposite directions.
There is 1 driven and 1 driving gear, therefore the gear ratio for pitch is 40/20 =~ 2 
The steps per revolution for pitch = steps per revolution without gear * ratio = 200 * 2 =~ 400

With gears for roll:
Requires motors to move in same directions.
There are 2 driven and 2 driving gear, therefore the gear ratio for roll is 40/20 * 28/16 =~ 3.5
The steps per revolution for pitch = steps per revolution without gear * ratio = 200 * 3.5 =~ 700
*/

const int stepsPerRevolution_pitch = 400;
const int stepsPerRevolution_roll = 712;


const int max_speed = 150; // in rpm
const int min_speed = 0; // in rpm
const int max_pitch_angle = 90; // in degrees
const int min_pitch_angle = 0; // in degrees

double sections = 0;
int dir =0;
double tmp_angle =0.0;

struct {
    double roll;
    double pitch;
} angles;

struct {
    double roll;
    double pitch;
} target_steps;

// for CAN bus use in future
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
int wrist_id = 509;
int data[8] = {}; 
static CAN_message_t msg;
int data1 = 0;
int data2 = 0;
int data3 = 0;
int data4 = 0;

// serial inputs that mimic CAN bus
/*

input is the function ID
  input = 1 -> Continous rotation ( FUNC_ID =1)
  input = 2 -> Going to a specific angle ( FUNC_ID =2)
  input = 3 -> Report location ( FUNC_ID =3)
  input = 4 -> Reset ( FUNC_ID =4)
  input = 5 -> Report hard stops ( FUNC_ID =5) // will not be an incoming value

input2 corresponds to data2 column
input3 corresponds to data3 column
input4 corresponds to data4 column
input5 corresponds to data5 column

refer to the CAN bus wrist interface protocal documentation for more detail

*/

int input; // function ID
int input2; // data1
int input3; // data2
int input4; // data3
int input5; // data4

// initialize the accelstepper library on pins 8 through 11:
AccelStepper right_motor(1, 5, 4);
AccelStepper left_motor(1, 3, 2);

void setup() {
  // initialize the serial port:
  Serial.begin(9600);

  // set the CAN bus
  can1.begin();
  can1.setBaudRate(250000);

  // initialize constant at setup time
  angles.roll = 0;
  angles.pitch = 0;
  target_steps.roll = 0;
  target_steps.pitch = 0;
  right_motor.setMaxSpeed((max_speed*stepsPerRevolution_roll)/60); // speed in sps
  right_motor.setCurrentPosition(0);
  right_motor.setAcceleration((max_speed*stepsPerRevolution_roll)/60); 
  left_motor.setMaxSpeed((max_speed*stepsPerRevolution_roll)/60); // speed in sps
  left_motor.setCurrentPosition(0);
  right_motor.setAcceleration((max_speed*stepsPerRevolution_roll)/60); 
}

void loop() {

  while (Serial.available()>0) { // check if there is a value in serial buffer

    // grabbing inputs from serial
    input = Serial.parseInt();
    Serial.println("input1");
    Serial.println(input); //funct
  
    input2 = Serial.parseInt();
    Serial.println("input2");
    Serial.println(input2); //data1
  
    input3 = Serial.parseInt();
    Serial.println("input3");
    Serial.println(input3); //data2
  
    input4 = Serial.parseInt();
    Serial.println("input4");
    Serial.println(input4); //data3
  
    input5 = Serial.parseInt();
    Serial.println("input5");
    Serial.println(input5); //data4
    delay(2000);

    /*
    Sample code to use once the CAN bus is connected
    can1.read(msg);
    if (msg.id == wrist_id ){
      for ( uint8_t i = 0; i < 8; i++ ) {
        data[i] = msg.buf[i];
      }
  
      switch(data[0]){
        case 1: // continous rotation
          data1 = data[1];
          data2 = data[2];
          data3 = data[3];
          break;
        case 2:  // Going to a specific angle
          data1 = data[1];
          data2 = data[2];
          data3 = data[3];
          data3 = data[4];
          break;
        case 3:  // report angle
          data1 = data[1];
          data2 = data[2];
          data3 = data[3];
          data3 = data[4];
          break;
        case 4:
          break;
        case 5:
          break;
        default:
          break;
      }
      
    }
    */

    // add code to check if the inputs are valid ------------------------------------ //

    
    
    switch(input){
        case 1: // requesting a continous rotation

          //set position for both motors
          right_motor.setCurrentPosition(target_steps.roll);
          left_motor.setCurrentPosition(target_steps.roll);

          if (input3 <= max_speed){ // check if the speed is in the range
          
            if (input2 == 1){ // requesting clockwise motion
              Serial.println("clockwise");

              // update roll position
              target_steps.roll = target_steps.roll + stepsPerRevolution_roll*input4;
              
              while (right_motor.currentPosition() != (int)target_steps.roll){
                /*
                 setSpeed() function requires input in sps, therefore the following convertion is done
                 sps -> steps per second
                 rpm -> revolutions per minnute
                 spr -> steps per revolution
                 
                 sps = (rpm * spr)/60
                 */
                 
                right_motor.setSpeed((input3*stepsPerRevolution_roll)/60); 
                left_motor.setSpeed((input3*stepsPerRevolution_roll)/60);
                right_motor.runSpeed();
                left_motor.runSpeed();
              }
  

            }else{ // requesting counter clock wise motion
              Serial.println("counter clockwise");

              // update roll position
              target_steps.roll = target_steps.roll - stepsPerRevolution_roll*input4;
              
              while (right_motor.currentPosition() != (int)target_steps.roll ){
                right_motor.setSpeed(-(input3*stepsPerRevolution_roll)/60); 
                left_motor.setSpeed(-(input3*stepsPerRevolution_roll)/60);
                right_motor.runSpeed();
                left_motor.runSpeed();
              }

            }
            Serial.print("Done: Currently at angle = ");
            angles.roll = (target_steps.roll/stepsPerRevolution_roll)*360.0; 
            Serial.print(angles.roll);
            Serial.print(" deg and steps = ");
            Serial.print(target_steps.roll);
            delay(2000);
            
          }else{
            Serial.println("Hard stop: speed must be less than 150 rpm");
            Serial.print("Currently at roll angle = ");
            Serial.print(angles.roll);
            Serial.print(" deg and steps = ");
            Serial.println(target_steps.roll);
            Serial.print("and currently at pitch angle = ");
            Serial.print(angles.pitch);
            Serial.print(" deg and steps = ");
            Serial.print(target_steps.pitch);
          }
          break;
        case 2:  // requesting to go to a specific angle
          // default speed used is 200 sps
          
          if (input3 == 1){ // requesting roll motion
            right_motor.setCurrentPosition(target_steps.roll);
            left_motor.setCurrentPosition(target_steps.roll);
          }else{ // requesting pitch motion
             right_motor.setCurrentPosition(target_steps.pitch);
            left_motor.setCurrentPosition(target_steps.pitch);
          }

          // check the requesting motion direction (clockwise or counterclockwise)
          if (input5 == 1){ // requesting clock wise motion
            dir = 1;
          }else{ // requesting counter clock wise motion
            dir = -1;
          }
          
          if (input2 == 1){// requesting relative motion  
            sections = 360.000/input4;
          }else{ //requesting absolute motion  
            if(input3 == 1){// requesting roll motion
              if (angles.roll > input4*dir){ //current angle > inputangle*direction_sign
                  tmp_angle = angles.roll - input4*dir;
                  dir = -1;
               }else{
                  tmp_angle = input4*dir - angles.roll;
                  dir = 1;
               }
            }else if (input3 == 2){ // requesting pitch motion
              if (angles.pitch > input4*dir){ //current angle > inputangle*direction_sign
                  tmp_angle = angles.pitch - input4*dir;
                  dir = -1;
               }else{
                  tmp_angle = input4*dir - angles.pitch;
                  dir = 1;
               }
            }
            sections = 360.000/tmp_angle;
          }
          
          if (input3 == 1){ // requesting roll motion
            if (dir == 1){  // requesting clockwise motion
              target_steps.roll = target_steps.roll + stepsPerRevolution_roll/sections;
            }else{
              target_steps.roll = target_steps.roll - stepsPerRevolution_roll/sections;
            }
            
            Serial.println("Request to specific roll angle");
            while (right_motor.currentPosition()!= (int)target_steps.roll ){
              right_motor.setSpeed(200*dir); 
              left_motor.setSpeed(200*dir);
              right_motor.runSpeed();
              left_motor.runSpeed();
            }

            Serial.print("Done: Currently at roll angle = ");
            angles.roll = ((float)target_steps.roll/stepsPerRevolution_roll)*360.0;
            Serial.print(angles.roll);
            Serial.print(" deg and steps = ");
            Serial.println(target_steps.roll);
            
            delay(3000);
          }else{ // requesting pitch motion

            if ( abs(input4) <= 90){ // check if the pitch angle is in range
              if (dir == 1){  // requesting clockwise motion
                target_steps.pitch = target_steps.pitch + stepsPerRevolution_pitch/sections;
              }else{
                target_steps.pitch = target_steps.pitch - stepsPerRevolution_pitch/sections;
              }
              
              Serial.println("Request to specific pitch angle");
              while (right_motor.currentPosition()!= (int)target_steps.pitch ){
                right_motor.setSpeed(200*dir); /////////// we define a speed // (14*(stepsPerRevolution/sections))/60
                left_motor.setSpeed(200*-dir);
                right_motor.runSpeed();
                left_motor.runSpeed();
              }

              Serial.print("Done: Currently at pitch angle = ");
              angles.pitch = ((float)target_steps.pitch/stepsPerRevolution_pitch)*360.0;
              Serial.println(angles.pitch);
              Serial.print(" deg and steps = ");
              Serial.println(target_steps.pitch);
            }else{
              Serial.println("Hard stop: pitch angle must be between +90 to -90 degrees");
              Serial.print("Currently at roll angle = ");
              Serial.print(angles.roll);
              Serial.print(" deg and steps = ");
              Serial.println(target_steps.roll);
              Serial.print("and currently at pitch angle = ");
              Serial.print(angles.pitch);
              Serial.print(" deg and steps = ");
              Serial.println(target_steps.pitch);
            }
            
            delay(3000);
          }
          break;
        case 3:  // requestnig to report angle
          Serial.println("Request to report position");
          Serial.print("Currently at roll angle = ");
          Serial.print(angles.roll);
          Serial.print(" deg and steps = ");
          Serial.println(target_steps.roll);
          Serial.print("and currently at pitch angle = ");
          Serial.print(angles.pitch);
          Serial.print(" deg and steps = ");
          Serial.println(target_steps.pitch);
          delay(3000);

          break;
        case 4: // requesting reset
          // default speed used is 100 sps for roll and 50 sps for pitch
          
          if (input2 == 1){ // requesting to reset roll
            Serial.println("Request to reset");
            right_motor.setCurrentPosition(target_steps.roll);
            left_motor.setCurrentPosition(target_steps.roll);
            
            if (target_steps.roll > 0 ){ // check if the current postion is in the positive direction or not
              target_steps.roll = 0;
              while (right_motor.currentPosition()!= (int)target_steps.roll ){
                right_motor.setSpeed(-100); 
                left_motor.setSpeed(-100);
                right_motor.runSpeed();
                left_motor.runSpeed();
              }
            }else{
              target_steps.roll = 0;
              while (right_motor.currentPosition()!= (int)target_steps.roll ){
                right_motor.setSpeed(100); 
                left_motor.setSpeed(100);
                right_motor.runSpeed();
                left_motor.runSpeed();
              }
            }

            Serial.print("Done: Currently at roll angle = ");
            angles.roll = ((float)target_steps.roll/stepsPerRevolution_roll)*360.0;
            Serial.print(angles.roll);
            Serial.print(" deg and steps = ");
            Serial.println(target_steps.roll);
        
            delay(3000);
          }
          
          if(input3 == 1){ // requesting to reset pitch
            Serial.println("Request to pitch");
            right_motor.setCurrentPosition(target_steps.pitch);
            left_motor.setCurrentPosition(target_steps.pitch);

            if (target_steps.pitch > 0 ){ // check if the current postion is in the positive direction or not
              target_steps.pitch = 0;
              while (right_motor.currentPosition()!= (int)target_steps.pitch ){
                right_motor.setSpeed(-50); 
                left_motor.setSpeed(50);
                right_motor.runSpeed();
                left_motor.runSpeed();
              }
            }else{
              target_steps.pitch = 0;
              while (right_motor.currentPosition()!= (int)target_steps.pitch ){
                right_motor.setSpeed(50); 
                left_motor.setSpeed(-50);
                right_motor.runSpeed();
                left_motor.runSpeed();
              }
            }

            Serial.print("Done: Currently at pitch angle = ");
            angles.pitch = ((float)target_steps.pitch/stepsPerRevolution_pitch)*360.0;
            Serial.print(angles.pitch);
            Serial.print(" deg and steps = ");
            Serial.println(target_steps.pitch);
            
            delay(3000);
          }
          break;
        case 5:
          break;
        default:
          break;
      }
  }
  
}
