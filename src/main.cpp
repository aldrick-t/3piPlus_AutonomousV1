//===============================
// 3pi+ Auto Roaming
// All in one functionality test platform.
// By: aldrick-t, DaniDRG04
// Version: 1.1.2 (April 2024)
//
//===============================


#include <Arduino.h>
#include <Pololu3piPlus32U4.h>
#include <string.h>
#include <Pololu3piPlus32U4IMU.h>
#include <Wire.h>
 
using namespace Pololu3piPlus32U4;
 
// IMU imu;
 
OLED display;
Buzzer buzzer;
ButtonA buttonA;
ButtonB buttonB;
ButtonC buttonC;
LineSensors lineSensors;
BumpSensors bumpSensors;
Motors motors;
Encoders encoders;

//Global Variables
int motorSpeed = 80;
int motorSpeedRev = motorSpeed/1.25;
int motorSpeedTurn = motorSpeed/2;
signed long encCountsL = encoders.getCountsAndResetLeft();
signed long encCountsR = encoders.getCountsAndResetRight();
bool bumpLeft = false;
bool bumpRight = false;
uint16_t lineSensVals[5];

//Two chevrons pointing up.
const char forwardArrows[] PROGMEM = {
  0b00000,
  0b00100,
  0b01010,
  0b10001,
  0b00100,
  0b01010,
  0b10001,
  0b00000,
};

//Fully lit up character matrix.
const char fullBlock[] PROGMEM = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
};

//Back arrow. (return arrow)
const char backArrow[] PROGMEM = {
  0b00000,
  0b00010,
  0b00001,
  0b00101,
  0b01001,
  0b11110,
  0b01000,
  0b00100,
};

//Two chevrons pointing down.
const char reverseArrows[] PROGMEM = {
  0b00000,
  0b10001,
  0b01010,
  0b00100,
  0b10001,
  0b01010,
  0b00100,
  0b00000,
};

const char perSec[] PROGMEM = {
  0B00000,
  0B00010,
  0B00100,
  0B01011,
  0B10100,
  0B00011,
  0B00001,
  0B00110
};

const char reload[] PROGMEM = {
  0B00000,
  0B10110,
  0B11001,
  0B11101,
  0B00001,
  0B10001,
  0B01110,
  0B00000
};

//Menu display declarations
char mainMenu(char);
char opMenu(char);
int settings(int vel);
void about();

//Settings function declarations
int speed(int);
void lineSensorsSet(int);
void bumpSensorsSet(int);
void encodersSet(int);
void motorsSet(int);
void inertialSet(int);
void feedbackSet(int);

//Operation modes declarations
void turtleAuto();
void doubtEvents();
void straightLine();
void setCircuit();

//Conversion functions
float tick2cm(int);
float tick2deg(int);

void setup() {
  //loads custom characters to memory
  display.loadCustomCharacter(forwardArrows, 1);
  display.loadCustomCharacter(reverseArrows, 2);
  display.loadCustomCharacter(backArrow, 7);
  display.loadCustomCharacter(fullBlock, 3);
  display.loadCustomCharacter(perSec, 4);
  display.loadCustomCharacter(reload, 5);

  display.setLayout21x8();
  display.noAutoDisplay();
  display.clear();

  bumpSensors.calibrate();
}

void loop() {
  char mode = 0;
  int vel = motorSpeed;
  
  while (true) {
    //update settings variables
    vel = motorSpeed;
    //menu switch
    switch (mode) {
    case 0:
      mode = mainMenu(mode);
      break;
    case 1:
      //Operation menu
      mode = opMenu(mode);
      break;
    case 2:
      //Settings menu
      mode = settings(vel);
      break;
    case 3:
      //About menu
      about();
      mode = 0;
      break;
    case 11:
      //Turtle Auto mode
      turtleAuto();
      mode = 1;
      break;
    case 12:
      //Doubt events
      doubtEvents();
      mode = 1;
      break;
    case 13:
      //Set Distance
      straightLine();
      mode = 1;
      break;
    case 14:
      //4 Segment Circuit
      setCircuit();
      mode = 1;
      break;
    case 21:
      //Motor Speed
      motorSpeed = speed(vel);
      mode = 2;
      break;
    case 22:
      //Line Sensors
      lineSensorsSet(vel);
      mode = 2;
      break;
    case 23:
      //Bump Sensors
      bumpSensorsSet(vel);
      mode = 2;
      break;
    case 24:
      //Encoders
      encodersSet(vel);
      mode = 2;
      break;
    case 25:
      //Motors
      motorsSet(vel);
      mode = 2;
      break;  
    case 26:
      //Inertial
      inertialSet(vel);
      mode = 2;
      break;
    case 27:
      //Feedback
      feedbackSet(vel);
      mode = 2;
      break;
     
    default:
      break;
    }
 }

}

char mainMenu(char mode) {
  //main menu
  display.clear();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("3pi+ Auto Roaming    ");
  display.gotoXY(0,5);
  display.print("Start              :A");
  display.gotoXY(0,6);
  display.print("Settings           :B");
  display.gotoXY(0,7);
  display.print("About              :C");
  display.display();

  while(true) {
    if(buttonA.getSingleDebouncedPress()) {
      mode = 1;
      break;
    }
    else if(buttonB.getSingleDebouncedPress()) {
      mode = 2;
      break;
    }
    else if(buttonC.getSingleDebouncedPress()) {
      mode = 3;
      break;
    }
  }

  return mode;
}

char opMenu(char mode) {
  display.clear();
  display.noInvert();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Operation Modes:     ");
  display.gotoXY(0,2);
  display.print("                   >>");
  display.gotoXY(0,5);
  display.print("Next               :A");
  display.gotoXY(0,6);
  display.print("Select             :B");
  display.gotoXY(0,7);
  display.print("Back\7              :C");
  //display.display();

  int setting = 0;
  const char* settings[] = {"Turtle Full Auto  ", "Doubt Events      ", "Set Distance      ",  "4 Seg. Circuit    "};
  while(true) {
    //display.gotoXY(0,2);
    //display.print("                   ");
    display.gotoXY(0,2);
    //display.print();
    display.print(settings[setting]);
    display.display();
    if (buttonA.getSingleDebouncedPress()){
      setting++;
      if (setting == 4) setting = 0;
    }
    else if (buttonB.getSingleDebouncedPress()){
      mode = setting + 11;
      break;
    }
    if(buttonC.getSingleDebouncedPress()) {
      mode = 0;
      break;
    }
  }
  return mode;
}

int settings(int mode) {
  display.clear();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Settings:            ");
  display.gotoXY(0,2);
  display.print("                   >>");
  display.gotoXY(0,5);
  display.print("Next               :A");
  display.gotoXY(0,6);
  display.print("Select             :B");
  display.gotoXY(0,7);
  display.print("Back\7              :C");
  display.display();

  int setting = 0;
  String settings[] = {"Motor Speed ", "Line Sensors", "Bump Sensors", "Encoders    ", "Motors      ", "Inertial    ", "Feedback    "};
  while(true) {
    display.gotoXY(0,2);
    display.print(settings[setting]);
    display.gotoXY(0,2);
    display.displayPartial(2, 0, 23);
    if (buttonA.getSingleDebouncedPress()){
      setting++;
      if (setting == 7) setting = 0;
    }
    else if (buttonB.getSingleDebouncedPress()){
      mode = setting + 21;
      break;
    }
    if(buttonC.getSingleDebouncedPress()) {
      mode = 0;
      break;
    }
  }
  return mode;
}

int speed(int vel){
  //velocity
  //display velocity menu
  display.clear();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Motor Speed:         ");
  display.gotoXY(0,6);
  display.print(" A        B        C ");
  display.gotoXY(0,7);
  display.print(" -        +        \7 ");
  display.display();
  while(true){
    //vel edit
    //reduce by 20 until 0
    if (buttonA.getSingleDebouncedPress() && vel > 0){
      vel = vel - 20;  
    //increase by 20 to limit
    } 
    else if (buttonB.getSingleDebouncedPress() && vel < 160){
      vel = vel + 20;
    }
    //print vel value
    display.gotoXY(0,2);
    display.print("Min");
    display.gotoXY(18,2);
    display.print("Max");
    display.gotoXY(0,3);
    display.print(" 0 ");
    display.gotoXY(18,3);
    display.print("160");
    display.display();
    display.gotoXY(9,3);
    display.print(vel);
    display.print(" ");
    display.gotoXY(0,2);
    display.displayPartial(2, 0, 23);
    
    motors.setSpeeds(vel, vel);
    //option exit
    if (buttonC.getSingleDebouncedPress()){
      motors.setSpeeds(0, 0);
      motorSpeed = vel;
      return vel;
    }
  }
  motors.setSpeeds(0, 0);
  return vel;
}

void lineSensorsSet(int sens) {
  bool emitterToggle = false;
  lineSensors.calibrate();
  //lineSensors.emittersOn();

  display.clear();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Line Sens:           ");
  display.gotoXY(0,1);
  display.print("IR Emitters:         ");
  display.gotoXY(0,2);
  display.print("    2    3    4      ");
  display.gotoXY(0,3);
  display.print("1                   5");
  display.gotoXY(0,5);
  display.print("Next               :A");
  display.gotoXY(0,6);
  display.print("Toggle Emitters    :B");
  display.gotoXY(0,7);
  display.print("Back\7              :C");

  while(true) {
    lineSensors.readCalibrated(lineSensVals);

    display.gotoXY(10,0);
    display.print(" Calibrated");
    display.gotoXY(0,4);
    display.print(lineSensVals[0]);
    display.print("    ");
    display.gotoXY(4,3);
    display.print(lineSensVals[1]);
    display.print("    ");
    display.gotoXY(9,3);
    display.print(lineSensVals[2]);
    display.print("    ");
    display.gotoXY(14,3);
    display.print(lineSensVals[3]);
    display.print("    ");
    display.gotoXY(17,4);
    display.print(lineSensVals[4]);
    display.print("    ");
    display.display();

    if(emitterToggle) {
      lineSensors.emittersOn();
      display.gotoXY(13,1);
      display.print("On ");
    } 
    else if(!emitterToggle) {
      lineSensors.emittersOff();
      display.gotoXY(13,1);
      display.print("Off");
    }
    if (buttonA.getSingleDebouncedPress()) {
      break;
    }
    else if(buttonB.getSingleDebouncedPress()) {
      emitterToggle = !emitterToggle;
      delay(100);
    }
    if(buttonC.getSingleDebouncedPress()) {
      lineSensors.emittersOff();
      break;
    }
  }
  

}

void bumpSensorsSet(int sens) { 
  bumpLeft = false;
  bumpRight = false;

  bumpSensors.calibrate();
  display.clear();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Bump Sensors:        ");
  display.gotoXY(0,2);
  display.print("  L               R  ");
  display.gotoXY(0,7);
  display.print("Back\7              :C");

  while(true) {
    bumpSensors.read();
    if(bumpSensors.leftIsPressed()) {
      bumpLeft = true;
    } else bumpLeft = false;
    if(bumpSensors.rightIsPressed()) {
      bumpRight = true;
    } else bumpRight = false;
    
    if(bumpSensors.leftChanged()) {
      display.gotoXY(2,4);
      display.print("\3");
      //delay(100);
    } else {
      display.gotoXY(2,4);
      display.print(" ");
    }
    if(bumpSensors.rightChanged()) {
      display.gotoXY(18,4);
      display.print("\3");
      //delay(100);
    } else {
      display.gotoXY(18,4);
      display.print(" ");
    }

    if (bumpLeft) {
      display.gotoXY(2,3);
      display.print("\3");
    } else {
      display.gotoXY(2,3);
      display.print(" ");
    }
    if (bumpRight) {
      display.gotoXY(18,3);
      display.print("\3");
    } else {
      display.gotoXY(18,3);
      display.print(" ");
    }
    display.displayPartial(3, 0, 23);

    if(buttonC.getSingleDebouncedPress()) {
      break;
    }
  }
}

void encodersSet(int sens) {
  encCountsL = encoders.getCountsAndResetLeft();
  encCountsR = encoders.getCountsAndResetRight();

  display.clear();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Motor Encoders:      ");
  display.gotoXY(0,2);
  display.print("L              R     ");

  display.gotoXY(0,5);
  display.print("Reset Counts       :A");
  display.gotoXY(0,6);
  display.print("Drive Motors\1      :B");
  display.gotoXY(0,7);
  display.print("Back\7              :C");

  while(true) {
    encCountsL = encoders.getCountsLeft();
    encCountsR = encoders.getCountsRight();
    if(encCountsL > 32767 || encCountsL < -32767) {
      encCountsL = encoders.getCountsAndResetLeft();
    }
    if(encCountsR > 32767 || encCountsR < -32767) {
      encCountsR = encoders.getCountsAndResetRight();
    }
    if(buttonA.getSingleDebouncedPress()) {
      encCountsL = encoders.getCountsAndResetLeft();
      encCountsR = encoders.getCountsAndResetRight();
    }
    if(buttonB.isPressed()) {
      motors.setSpeeds(motorSpeed, motorSpeed);
      encCountsL = encoders.getCountsLeft();
      encCountsR = encoders.getCountsRight();
    } else {
      motors.setSpeeds(0, 0);
    }

    display.gotoXY(0,3);
    display.print(encCountsL);
    display.print("     ");
    display.gotoXY(16,3);
    display.print(encCountsR);
    display.print("     ");

    // display.gotoXY(0,4);
    // if(encoders.checkErrorLeft()) {
    //   display.print("Error");
    // } else {
    //   display.print("     ");
    // }
    // display.gotoXY(16,4);
    // if(encoders.checkErrorRight()) {
    //   display.print("Error");
    // } else {
    //   display.print("     ");
    // }

    display.gotoXY(0,4);
    display.print(tick2cm(encCountsL));
    display.print("cm");
    display.gotoXY(14,4);
    display.print(tick2cm(encCountsR));
    display.print("cm");
    

    display.displayPartial(3, 0, 23);

    if(buttonC.getSingleDebouncedPress()) {
      break;
    }
  }
}

void motorsSet(int sens) {

}

void inertialSet(int sens) {
  display.clear();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Inertial Sensors:    ");
  display.gotoXY(0,2);
  display.print("Accelerometer        ");
  display.gotoXY(0,3);

}

void feedbackSet(int sens) {

}

void about() {
  display.clear();
  display.gotoXY(0,0);
  display.print("3pi+ Auto Roaming    ");
  display.gotoXY(0,1);
  display.print("Version: 1.1.2       ");
  display.gotoXY(0,2);
  display.print("All in one functiona-");
  display.gotoXY(0,3);
  display.print("lity test platform.  ");
  display.gotoXY(0,7);
  display.print("Back\7              :C");
  display.display();

  while(true){
    if(buttonC.getSingleDebouncedPress()) {
      break;
    }
  }
}

void turtleAuto() {
  display.clear();
  display.setLayout11x4();
  display.gotoXY(2,1);
  display.print("Turtle");
  display.gotoXY(1,2);
  display.print(" Full Auto ");
  display.invert();
  display.display();
  delay(2500);

  display.clear();
  display.noInvert();
  display.gotoXY(0,0);
  display.print("Roaming... ");
  display.gotoXY(0,3);
  display.print(" C to STOP ");
  display.display();

  //FSD System
  int avoidCount = 0;
  while(true) {
    //Start Roam
    int encLocL = encoders.getCountsAndResetLeft();
    int encLocR = encoders.getCountsAndResetRight();
    bumpSensors.read();
    lineSensors.calibrate();
    lineSensors.emittersOn();
    lineSensors.readCalibrated(lineSensVals);

    //LEFT ONLY collision redirect (Rev + Turn Right)
    if(bumpSensors.leftIsPressed() && !bumpSensors.rightIsPressed()) {
      ledRed(1);
      motors.setSpeeds(0, 0);
      encLocL = encoders.getCountsAndResetLeft();
      encLocR = encoders.getCountsAndResetRight();
      ledYellow(1);
      while (encLocL > -100 && encLocR > -100) {
        motors.setSpeeds(-motorSpeedRev, -motorSpeedRev);
        encLocL = encoders.getCountsLeft();
        encLocR = encoders.getCountsRight();
      }
      encLocL = encoders.getCountsAndResetLeft();
      encLocR = encoders.getCountsAndResetRight();
      while (encLocR > -200) {
        motors.setSpeeds(0, -motorSpeedTurn);
        encLocL = encoders.getCountsLeft();
        encLocR = encoders.getCountsRight();
      }
      motors.setSpeeds(0, 0);
      avoidCount++;
    } 
    //RIGHT ONLY collision redirect (Rev + Turn Left)
    else if(bumpSensors.rightIsPressed() && !bumpSensors.leftIsPressed()) {
      ledRed(1);
      motors.setSpeeds(0, 0);
      encLocL = encoders.getCountsAndResetLeft();
      encLocR = encoders.getCountsAndResetRight();
      ledYellow(1);
      while (encLocL > -100 && encLocR > -100) {
        motors.setSpeeds(-motorSpeedRev, -motorSpeedRev);
        encLocL = encoders.getCountsLeft();
        encLocR = encoders.getCountsRight();
      }
      encLocL = encoders.getCountsAndResetLeft();
      encLocR = encoders.getCountsAndResetRight();
      while (encLocL > -200) {
        motors.setSpeeds(-motorSpeedTurn, 0);
        encLocL = encoders.getCountsLeft();
        encLocR = encoders.getCountsRight();
      }
      motors.setSpeeds(0, 0);
      avoidCount++;  
    }
    //BOTH collision redirect (2xRev + 90Turn Right)
    else if(bumpSensors.leftIsPressed() && bumpSensors.rightIsPressed()) {
      ledRed(1);
      motors.setSpeeds(0, 0);
      encLocL = encoders.getCountsAndResetLeft();
      encLocR = encoders.getCountsAndResetRight();
      ledYellow(1);
      while (encLocL > -200 && encLocR > -200) {
        motors.setSpeeds(-motorSpeedRev, -motorSpeedRev);
        encLocL = encoders.getCountsLeft();
        encLocR = encoders.getCountsRight();
      }
      encLocL = encoders.getCountsAndResetLeft();
      encLocR = encoders.getCountsAndResetRight();
      while (encLocL < 270 || encLocR > -270) {
        motors.setSpeeds(motorSpeedTurn, -motorSpeedTurn);
        encLocL = encoders.getCountsLeft();
        encLocR = encoders.getCountsRight();
      }
      motors.setSpeeds(0, 0);
      avoidCount++;
    }
    //No Progress / Corner (Rev + 180Spin Right)
    if (avoidCount >= 3) {
      motors.setSpeeds(0, 0);
      encLocL = encoders.getCountsAndResetLeft();
      encLocR = encoders.getCountsAndResetRight();
      while (encLocL > -200 && encLocR > -200) {
        motors.setSpeeds(-motorSpeedRev, -motorSpeedRev);
        encLocL = encoders.getCountsLeft();
        encLocR = encoders.getCountsRight();
      }
      encLocL = encoders.getCountsAndResetLeft();
      encLocR = encoders.getCountsAndResetRight();
      while (encLocL < 540) {
        motors.setSpeeds(motorSpeedTurn, -motorSpeedTurn);
        encLocL = encoders.getCountsLeft();
        encLocR = encoders.getCountsRight();
      }
      motors.setSpeeds(0, 0);
      avoidCount = 0;
    }

    //Edge Detection (Rev + 90Turn Right)
    if(lineSensVals[0] > 650 && lineSensVals[1] > 650 && lineSensVals[2] > 650 && lineSensVals[3] > 650 && lineSensVals[4] > 650) {
      
      for(int i=0; i < 5; i++) {
        lineSensors.readCalibrated(lineSensVals);
        if(lineSensVals[i] > 650) {
          motors.setSpeeds(0, 0);
          encLocL = encoders.getCountsAndResetLeft();
          encLocR = encoders.getCountsAndResetRight();
          while (encLocL > -600 && encLocR > -600) {
            motors.setSpeeds(-motorSpeedRev, -motorSpeedRev);
            encLocL = encoders.getCountsLeft();
            encLocR = encoders.getCountsRight();
          }
          encLocL = encoders.getCountsAndResetLeft();
          encLocR = encoders.getCountsAndResetRight();
          motors.setSpeeds(0, 0);
          switch (i) {
          case 0:
            encLocL = encoders.getCountsAndResetLeft();
            encLocR = encoders.getCountsAndResetRight();
            while (encLocL < 120) {
              motors.setSpeeds(motorSpeedTurn, -motorSpeedTurn);
              encLocL = encoders.getCountsLeft();
              encLocR = encoders.getCountsRight();
            }
            motors.setSpeeds(0, 0);
            break;
          case 1 || 3:
            encLocL = encoders.getCountsAndResetLeft();
            encLocR = encoders.getCountsAndResetRight();
            while (encLocL < 320) {
              motors.setSpeeds(motorSpeedTurn, -motorSpeedTurn);
              encLocL = encoders.getCountsLeft();
              encLocR = encoders.getCountsRight();
            }
            motors.setSpeeds(0, 0);
            break;
          case 2:
            encLocL = encoders.getCountsAndResetLeft();
            encLocR = encoders.getCountsAndResetRight();
            while (encLocL < 260) {
              motors.setSpeeds(motorSpeedTurn, -motorSpeedTurn);
              encLocL = encoders.getCountsLeft();
              encLocR = encoders.getCountsRight();
            }
            motors.setSpeeds(0, 0);
            break;
          case 4:
            encLocL = encoders.getCountsAndResetLeft();
            encLocR = encoders.getCountsAndResetRight();
            while (encLocR < 120) {
              motors.setSpeeds(-motorSpeedTurn, motorSpeedTurn);
              encLocL = encoders.getCountsLeft();
              encLocR = encoders.getCountsRight();
            }
            break;
          default:
            break;
          }
        }
        motors.setSpeeds(0, 0);
        //avoidCount++;
      }
    }  

    //No Anomalies (Forward)
    if (!bumpSensors.leftIsPressed() && !bumpSensors.rightIsPressed()) {
      ledRed(0);
      ledYellow(0);
      motors.setSpeeds(motorSpeed, motorSpeed);
      if (encLocL > 2000 || encLocR > 2000) {
        avoidCount = 0;
      }
    }


    //Stop Roam
    if(buttonC.getSingleDebouncedPress()) {
      motors.setSpeeds(0, 0);
      break;
    }
  }
}

void doubtEvents() { 

}

void straightLine() {
  display.clear();
  display.noInvert();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Set Distance:  Config");
  display.gotoXY(0,1);
  display.print("Speed:               ");
  display.gotoXY(0,2);
  display.print("Distance:            ");
  display.gotoXY(0,3);
  display.print("Direction:           ");
  display.gotoXY(0,6);
  display.print(" A        B        C ");
  display.gotoXY(0,7);
  display.print(" -       SET       + ");
  

  int dist = 0;
  bool dir = true;
  int speed = 0;
  int speedSet = 0;
  int modeLoc = 0;
  int8_t deltaTime = 50; //time in ms
  double prevTime = 0; 
  float distCurrent = 0;
  //float encLocL = 0;
  //float encLocR = 0;
  float velCurrent = 0;
  float distTotal = 0;
  
  while(modeLoc != 4) {
    switch (modeLoc) {
    case 0:
      display.gotoXY(0,0);
      display.print("Set Distance:  Config");
      display.gotoXY(0,6);
      display.print(" A        B        C ");
      display.gotoXY(0,7);
      display.print(" -       SET       + ");
      while(true) {
        display.gotoXY(12,1);
        display.print("->");
        display.gotoXY(15,1);
        display.print(speed);
        display.print(" ");
        display.gotoXY(18,1);
        display.print("cm\4");
        if(buttonC.getSingleDebouncedPress() && speed < 150) {
          speed = speed + 15;
        }
        else if(buttonA.getSingleDebouncedPress() && speed > 0) {
          speed = speed - 15;
        }
        else if(buttonB.getSingleDebouncedPress()) {
          // speed = calculo
          speedSet = (speed * 400)/150;
          display.gotoXY(0,1);
          display.print("Speed:         ");
          modeLoc++;
          break;
        }
      }
      break;
    case 1:
      while(true) {
        display.gotoXY(12,2);
        display.print("->");
        display.gotoXY(15,2);
        display.print(dist);
        display.print("   ");
        display.gotoXY(19,2);
        display.print("cm");
        if(buttonC.getSingleDebouncedPress() && dist < 9999) {
          dist = dist + 20;
        }
        else if(buttonA.getSingleDebouncedPress() && dist > 0) {
          dist = dist - 20;
        }
        else if(buttonB.getSingleDebouncedPress()) {
          display.gotoXY(0,2);
          display.print("Distance:      ");
          modeLoc++;
          break;
        }
      }
      break;
    case 2:
      while(true) {
        display.gotoXY(0,7);
        display.print("\1/\2      SEL        ");
        display.gotoXY(12,3);
        display.print("->");
        display.gotoXY(15,3);
        if(dir) {
          display.print("FWD \1");
        } else {
          display.print("REV \2");
        }
        if(buttonA.getSingleDebouncedPress()) {
          dir = !dir;
        }
        else if(buttonB.getSingleDebouncedPress()) {
          display.gotoXY(0,3);
          display.print("Direction:     ");
          modeLoc++; //consider either new case or exit case and run prog block
          break;
        }
      }
      break;
    case 3:
      //display.clear();
      display.gotoXY(14,0);
      display.print("       ");
      display.gotoXY(16,0);
      display.print(" in 3");
      delay(1000);
      display.gotoXY(16,0);
      display.print(" in 2");
      delay(1000);
      display.gotoXY(16,0);
      display.print(" in 1");
      delay(1000);
      display.gotoXY(0,0);
      display.print("Set Distance: Running");

      while(modeLoc != 4) {
        if(millis() - prevTime >= deltaTime) {
          prevTime = millis();
          encCountsL = encoders.getCountsAndResetLeft();
          encCountsR = encoders.getCountsAndResetRight();

          distCurrent = (tick2cm(encCountsL) + tick2cm(encCountsR))/2;
          velCurrent = distCurrent / (deltaTime/1000.0);
          if (dir) {
            distTotal += distCurrent;
          } else {
            distTotal -= distCurrent;
          }
          //consider moving motor logic block here
          display.gotoXY(0,4);
          display.print("Velocity: ");
          if (dir) {
            display.print(velCurrent);
          } else {
            display.print(-velCurrent);
          }
          display.print("cm\4");
          display.print("  ");
          display.gotoXY(0,5);
          display.print("Distance: ");
          if (dir) {
            display.print(distTotal);
          } else {
            display.print(-distTotal);
          }
          display.print("cm");
          display.print("  ");

          if((dist - distTotal) >= 0) {
            if (dir) {
              motors.setSpeeds(speedSet, speedSet);
            } else {
              motors.setSpeeds(-speedSet, -speedSet);
            }
          } else {
            motors.setSpeeds(0, 0);
            display.gotoXY(0,0);
            display.print("Set Distance:   Done!");
            display.gotoXY(0,7);
            display.print("          \5        \7 ");
          }
        }
        if(buttonB.getSingleDebouncedPress()) {
          motors.setSpeeds(0, 0);
          distTotal = 0;
          modeLoc = 0;
          break;
        }
        else if(buttonC.getSingleDebouncedPress()) {
          motors.setSpeeds(0, 0);
          modeLoc = 4;
          break;
        }
      }
    default:
      break;
    }
  }
}

void setCircuit() {
  display.clear();
  display.noInvert();
  display.setLayout21x8();
  display.gotoXY(0,0);
  display.print("Set 4 Seg. Circuit:  ");
  display.gotoXY(0,6);
  display.print(" A        B        C ");
  display.gotoXY(0,7);
  display.print(" -       SET       + ");
  
  int modeLoc = 0;

  short int distSet = 0;
  char minSpeed_set = 0;
  char maxSpeed_set = 0;
  short int angleSet = 0;

  //int8_t deltaTime = 50; //time in ms
  //double prevTime = 0;
  //float period = 0.05;

  float distCurrent = 0;
  float distTotal = 0;
  float posError = 0;

  //int encLocL = 0;
  //int encLocR = 0;

  //float velCurrent = 0;
  int speed = 0;

  //float angleTotal = 0;
  float angleCurrent = 0;
  float angleError = 0;
  float angleZero = 0;
  float angle = 0;

  float kp_dist = 3;
  float kp_angle = 1.5;

  float maxSpeed_cms = 60;
  float minSpeed_cms = 10;
  char bumpIdent = 0; //0 = none, 1 = left, 2 = right

  int circuitSegment[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};


  while(modeLoc != 10) {
    switch (modeLoc) {
    case 0: //Max Speed
      display.gotoXY(0,2);
      display.print("Max Speed:            ");
      display.gotoXY(0,3);
      display.print("Min Speed:           ");
      while(true) {
        display.gotoXY(10,2);
        display.print("->");
        display.print(maxSpeed_cms);
        display.print("cm\4");
        display.print("  ");
        display.gotoXY(10,3);
        display.print("  ");
        display.print(minSpeed_cms);
        display.print("cm\4");
        display.print("  ");
        if(buttonC.getSingleDebouncedPress() && maxSpeed_cms < 150) {
          maxSpeed_cms = maxSpeed_cms + 5;
        }
        else if(buttonA.getSingleDebouncedPress() && maxSpeed_cms > minSpeed_cms) {
          maxSpeed_cms = maxSpeed_cms - 5;
        }
        else if(buttonB.getSingleDebouncedPress()) {
          maxSpeed_set = (speed * 400)/150;
          display.gotoXY(0,2);
          display.print("Max Speed:  ");
          modeLoc++;
          break;
        }
      }
      break;
    case 1: //Min Speed
      while(true) {
        display.gotoXY(10,3);
        display.print("->");
        display.print(minSpeed_cms);
        display.print("cm\4");
        display.print("  ");
        //display.gotoXY(18,3);
        if(buttonC.getSingleDebouncedPress() && minSpeed_cms < maxSpeed_cms) {
          minSpeed_cms = minSpeed_cms + 5;
        }
        else if(buttonA.getSingleDebouncedPress() && minSpeed_cms > 5) {
          minSpeed_cms = minSpeed_cms - 5;
        }
        else if(buttonB.getSingleDebouncedPress()) {
          // speed = calculo
          minSpeed_set = (speed * 400)/150;
          display.gotoXY(0,3);
          display.print("Min Speed:  ");
          modeLoc++;
          break;
        }
      }
      break;
    case 2: //Brake Gain
      display.gotoXY(0,0);
      display.print("Set Circuit:         ");
      display.gotoXY(0,2);
      display.print("Brake Gain:          ");
      display.gotoXY(0,3);
      display.print("Turn Gain:           ");  
      while(true) {
        display.gotoXY(11,2);
        display.print("->");
        display.print(kp_dist);
        display.print("  ");
        if (buttonC.getSingleDebouncedPress() && kp_dist < 10) {
          kp_dist = kp_dist + 0.5;
        }
        else if (buttonA.getSingleDebouncedPress() && kp_dist > 0) {
          kp_dist = kp_dist - 0.5;
        }
        else if (buttonB.getSingleDebouncedPress()) {
          display.gotoXY(0,2);
          display.print("Brake Gain:  ");
          modeLoc++;
          break;
        }
      }
      break;
    case 3: //Turn Gain
      while(true) {
        display.gotoXY(11,3);
        display.print("->");
        display.print(kp_angle);
        display.print("  ");
        if (buttonC.getSingleDebouncedPress() && kp_angle < 5) {
          kp_angle = kp_angle + 0.5;
        }
        else if (buttonA.getSingleDebouncedPress() && kp_angle > 0) {
          kp_angle = kp_angle - 0.5;
        }
        else if (buttonB.getSingleDebouncedPress()) {
          display.gotoXY(0,3);
          display.print("Turn Gain:   ");
          modeLoc++;
          break;
        }
      }
      break;
    case 4: //Circuit Segment Set
      display.gotoXY(0,0);
      display.print("Set Circuit:         ");
      display.gotoXY(0,1);
      display.print("Segment 1:           ");
      display.gotoXY(0,2);
      display.print("Distance:            ");
      display.gotoXY(0,3);
      display.print("Angle:               ");

      for (int seg = 0; seg < 4; seg++) {
        switch (seg) {
        case 0:
          display.gotoXY(0,1);
          display.print("Segment ONE:         ");
          break;
        case 1:
          display.gotoXY(0,1);
          display.print("Segment TWO:         ");
          break;
        case 2:
          display.gotoXY(0,1);
          display.print("Segment THREE:       ");
          break;
        case 3:
          display.gotoXY(0,1);
          display.print("Segment FOUR:        ");
          break;
        }
        while(true) {
          display.gotoXY(10,2);
          display.print("->");
          display.print(distSet);
          display.print("cm    ");
          if(buttonC.getSingleDebouncedPress() && distSet < 9999) {
            distSet = distSet + 20;
          }
          else if(buttonA.getSingleDebouncedPress() && distSet > 0) {
            distSet = distSet - 20;
          }
          else if(buttonB.getSingleDebouncedPress()) {
            circuitSegment[seg][0] = distSet;
            display.gotoXY(0,2);
            display.print("Distance:   ");
            
            //modeLoc++;
            break;
          }
        }
        while(true) {
          display.gotoXY(10,3);
          display.print("->");
          display.print(angleSet);
          display.print("deg   ");
          if (seg == 3) {
            display.gotoXY(0,7);
            display.print(" -       RUN       + ");
          }
          if(buttonC.getSingleDebouncedPress() && angleSet < 360) {
            angleSet = angleSet + 15;
          }
          else if(buttonA.getSingleDebouncedPress() && angleSet > -360) {
            angleSet = angleSet - 15;
          }
          else if(buttonB.getSingleDebouncedPress()) {
            circuitSegment[seg][1] = angleSet;
            display.gotoXY(0,3);
            display.print("Angle:      ");
            //modeLoc++;
            break;
          }
        }
      }
      modeLoc++;
      break;
      
    case 5:
      display.gotoXY(14,0);
      display.print("       ");
      display.gotoXY(16,0);
      display.print(" in 3");
      delay(1000);
      display.gotoXY(16,0);
      display.print(" in 2");
      delay(1000);
      display.gotoXY(16,0);
      display.print(" in 1");
      delay(1000);
      display.gotoXY(0,0);
      display.print("Set Circuit: Running");

      for (int seg = 0; seg < 4; seg++) {
        modeLoc = 0;
        switch (modeLoc) {
        case 0:
          while(true) {
            encCountsL = encoders.getCountsAndResetLeft();
            encCountsR = encoders.getCountsAndResetRight();
            bumpLeft = bumpSensors.leftIsPressed();
            bumpRight = bumpSensors.rightIsPressed();

            distCurrent = (tick2cm(encCountsL) + tick2cm(encCountsR))/2;
            //velCurrent = distCurrent / (deltaTime/1000.0);
            distTotal += distCurrent;
            angleCurrent = angleCurrent + 1.6 * ((tick2deg(encCountsL) - tick2deg(encCountsR))/9);

            posError = circuitSegment[seg][0] - distTotal;
            //angleError = circuitSegment[seg][1] - angleCurrent;

            if (posError > 0) {
              speed = kp_dist * posError;
            } else {
              speed = 0;
              modeLoc++;
            }
            if(speed > maxSpeed_set) {
              speed = maxSpeed_set;
            } else if(speed < minSpeed_set) {
              speed = maxSpeed_set;
            }

            //Angle Calc for Straight Path
            angleError = angleZero - angleCurrent;
            // Angle control rule
            angle = kp_angle * angleError;

            if (angle < 1) {
              motors.setSpeeds(-speed, speed);
            }
            else if (angle > 1) {
              motors.setSpeeds(speed, -speed);
            }
            else {
              motors.setSpeeds(0, 0);
            }

            int segInt = seg + 1;
            display.gotoXY(0,1);
            display.print("Segment: " + (segInt));
            display.gotoXY(0,2);
            display.print("Distance: ");
            display.print(distTotal);
            display.print("cm");
            display.print("  ");
            display.gotoXY(0,3);
            display.print("Angle:    ");
            display.print(angleCurrent);
            display.print("° ");
            display.print("  ");
            display.gotoXY(0,4);
            display.print("Straight Path...     ");

            if (bumpLeft || bumpRight) {
                modeLoc == 20;
                while (modeLoc == 20){
                  motors.setSpeeds(0, 0);
                  display.gotoXY(0,0);
                  display.print("Set Circuit:   PAUSED");
                  display.gotoXY(0,4);
                  display.print("Hit Obstacle!        ");
                  display.gotoXY(0,5);
                  display.print("Set Action:          ");
                  display.gotoXY(0,6);
                  display.print(" A                 C ");
                  display.gotoXY(0,7);
                  display.print("CONTINUE        RESET");

                  if(buttonA.getSingleDebouncedPress()) {
                    modeLoc = 0;
                    if (bumpLeft) {
                      bumpIdent = 1;
                    } else if (bumpRight) {
                      bumpIdent = 2;
                    }
                    break;
                  }
                  else if(buttonC.getSingleDebouncedPress()) {
                    modeLoc = 10;
                    break;
                  }
                }
              } 
          }
          break;
        case 1:
          encCountsL = encoders.getCountsAndResetRight();
          encCountsR = encoders.getCountsAndResetLeft();
          
          angleError = circuitSegment[seg][1] - angleCurrent;
          // Angle control rule
          angle = kp_angle * angleError;

          if (angle < 1) {
            motors.setSpeeds(-speed, speed);
          }
          else if (angle > 1) {
            motors.setSpeeds(speed, -speed);
          }
          else {
            motors.setSpeeds(0, 0);
          }
          break;
        }
      
      }
      break;
    default:
      break;
    }
  }
}

float tick2cm(int ticks) {
  float cm;
  cm = ticks * (1.0/12.0) * (1.0/29.86) * ((3.1*3.1416)/1);
  return cm;
}

float tick2deg(int ticks) {
  float deg;
  deg = ticks * (1.0/12.0) * (1.0/29.86) * (360);
  return deg;
}




  // //velocity calc
  // //calc pos error
  // posError = distSet - distTotal;
  // //control rule
  // speed = kp * posError;

  // if(speedSet > maxSpeed_set) {
  //   speedSet = maxSpeed_cms;
  // } else if(speed < minSpeed_set) {
  //   speedSet = minSpeed_cms;
  // }

  // if(posError > 0) {
  //   motors.setSpeeds(speedSet, speedSet);
  // } else {
  //   motors.setSpeeds(0, 0);
  // }

  // //angle calc
  // angleCurrent = angleCurrent + 1.6 * ((tick2deg(encCountsR) - tick2deg(encCountsL))/9);

  // // angle error calc
  // angleError = angleSet - angleCurrent;
  // // control rule
  // angleSet = kp * angleError;

  // if (angleSet > 0) {
  //   motors.setSpeeds(-speedSet, speedSet);
  // } else {
  //   motors.setSpeeds(speedSet, -speedSet);
  // }

  // if (angleError < 1) {
  //   motors.setSpeeds(-speed, speed);
  // }
  // else if (angleError > 1) {
  //   motors.setSpeeds(speed, -speed);
  // }
  // else {
  //   motors.setSpeeds(0, 0);
  // }
