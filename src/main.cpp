/*
  ************************************************************************************
  * MIT License
  *
  * Copyright (c) 2025 Crunchlabs LLC (LabelMaker Code)

  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is furnished
  * to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
  * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *
  ************************************************************************************
*/
//////////////////////////////////////////////////
    //  LIBRARIES  //
//////////////////////////////////////////////////
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <ESP32Servo.h>
#include <stdio.h>
#include <math.h>
#include "WebServer.h"

// Replace with your network credentials
const char* ssid = "MyWifi";
const char* password = "myPassword";

WebServer server(ssid, password);

typedef struct {
    float x;
    float y;
} Point;

//////////////////////////////////////////////////
//  PINS AND PARAMETERS  //
//////////////////////////////////////////////////

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16x2 display

#define INIT_MSG "Initializing..." // Text to display on startup
#define PRINTING "    PRINTING    " // NOTE: this text must be 16 characters or LESS in order to fit on the screen correctly
#define MENU_CLEAR ":                " //this one clears the menu for editing
#define SCALE_VALUE 76  
#define NUMBER_OF_SEGMENTS 20

//text variables
int x_scale = SCALE_VALUE;//these are multiplied against the stored coordinate (between 0 and 4) to get the actual number of steps moved
int y_scale = SCALE_VALUE;//for example, if this is 230(default), then 230(scale) x 4(max coordinate) = 920 (motor steps)
int scale = x_scale;

// Stepper motor parameters
const int stepCount = 200;
const int stepsPerRevolution = 2048;

// initialize the stepper library for both steppers:
Stepper xStepper(stepsPerRevolution, 6, 8, 7, 9);
Stepper yStepper(stepsPerRevolution, 2, 4, 3, 5); 

int xPins[4] = {6, 8, 7, 9};  // pins for x-motor coils
int yPins[4] = {2, 4, 3, 5};    // pins for y-motor coils

//Servo
const int SERVO_PIN  = 13;
Servo servo;
int angle = 30; // the current angle of servo motor


// Creates states to store what the current menu and joystick states are
// Decoupling the state from other functions is good because it means the sensor / screen aren't hardcoded into every single action and can be handled at a higher level
enum State { Initializing, Waiting, Printing };
State currentState = Initializing;

boolean pPenOnPaper = false; // pen on paper in previous cycle
int lineCount = 0;

int xpos = 0;
int ypos = 0;
const int posS = 2;
const int posM = 7;
const int posL = 12;
IPAddress ip;

void displayIpAddress(IPAddress ip) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP Address: ");
  lcd.setCursor(0, 1);
  lcd.print(ip);
}

void plot(boolean penOnPaper) {  //used to handle lifting or lowering the pen on to the tape
  if (penOnPaper) {              //if the pen is already up, put it down
    angle = 80;
  } else {  //if down, then lift up.
    angle = 25;
  }
  servo.write(angle);                        //actuate the servo to either position.
  if (penOnPaper != pPenOnPaper) delay(50);  //gives the servo time to move before jumping into the next action
  pPenOnPaper = penOnPaper;                  //store the previous state.
}

void line(int newx, int newy, bool drawing) {
  //this function is an implementation of bresenhams line algorithm
  //this algorithm basically finds the slope between any two points, allowing us to figure out how many steps each motor should do to move smoothly to the target
  //in order to do this, we give this function our next X (newx) and Y (newy) coordinates, and whether the pen should be up or down (drawing)

  // int scaledY = newy * 3.5;
  // int scaledX = newx * 1.5;
  // int currentScaledY = ypos * 3.5;
  // int currentScaledX = xpos * 1.5;//2.3

  int scaledY = newy;
  int scaledX = newx / 2.3;
  int currentScaledY = ypos;
  int currentScaledX = xpos / 2.3;


  if (drawing < 2) {  //checks if we should be drawing and puts the pen up or down based on that.
    plot(drawing);    // dashed: 0= don't draw / 1=draw / 2... = draw dashed with variable dash width
  } else {
    plot((stepCount / drawing) % 2);  //can do dashed lines, but for now this isn't doing anything since we're only sending 0 or 1.
  }

  int i;
  long over = 0;

  long dx = scaledX - currentScaledX;  //calculate the difference between where we are (xpos) and where we want to be (newx)
  long dy = scaledY - currentScaledY;
  int dirx = dx > 0 ? -1 : 1;  //this is called a ternary operator, it's basically saying: if dx is greater than 0, then dirx = -1, if dx is less than or equal to 0, dirx = 1.
  int diry = dy > 0 ? 1 : -1;  //this is called a ternary operator, it's basically saying: if dy is greater than 0, then diry = 1, if dy is less than or equal to 0, diry = -1.
  //the reason one of these ^ is inverted logic (1/-1) is due to the direction these motors rotate in the system.

  dx = abs(dx);  //normalize the dx/dy values so that they are positive.
  dy = abs(dy);  //abs() is taking the "absolute value" - basically it removes the negative sign from negative numbers

  //the following nested If statements check which change is greater, and use that to determine which coordinate (x or y) get's treated as the rise or the run in the slope calculation
  //we have to do this because technically bresenhams only works for the positive quandrant of the cartesian coordinate grid,
  // so we are just flipping the values around to get the line moving in the correct direction relative to it's current position (instead of just up an to the right)
  if (dx > dy) {
    over = dx / 2;
    for (i = 0; i < dx; i++) {  //for however much our current position differs from the target,
      xStepper.step(dirx);      //do a step in that direction (remember, dirx is always going to be either 1 or -1 from the ternary operator above)

      // Serial.print("Xsteps: ");
      // Serial.print(dirx);
      // Serial.print("  ");

      over += dy;
      if (over >= dx) {
        over -= dx;

        // Serial.print("Ysteps: ");
        // Serial.println(diry);

        yStepper.step(diry);
      }
      //delay(1);
    }
  } else {
    over = dy / 2;
    for (i = 0; i < dy; i++) {
      yStepper.step(diry);
      // Serial.print("Ysteps: ");
      // Serial.print(diry);
      // Serial.print("  ");
      over += dx;
      if (over >= dy) {
        over -= dy;
        // Serial.print("Xsteps: ");
        // Serial.println(dirx);
        xStepper.step(dirx);
      }
      //delay(1);
    }
  }
  xpos = newx;  //store positions
  ypos = newy;  //store positions
}

void bezierCurve(Coord start, Coord control1, Coord control2, Coord end, int segments, Coord* outPoints) {
  for (int i = 0; i <= segments; i++) {
      float t = (float)i / segments;
      float u = 1.0f - t;

      float x = u*u*u * start.x +
                3 * u*u * t * control1.x +
                3 * u * t*t * control2.x +
                t*t*t * end.x;

      float y = u*u*u * start.y +
                3 * u*u * t * control1.y +
                3 * u * t*t * control2.y +
                t*t*t * end.y;

      outPoints[i].x = x;
      outPoints[i].y = y;
  }
}

void approximateEllipse(int centerX, int centerY, int width, int height, int segments, Coord* outPoints) {
  float angleStep = 2.0f * M_PI / segments;

  for (int i = 0; i <= segments; ++i) { // <= to close the ellipse loop
      float angle = i * angleStep;
      outPoints[i].x = centerX + (width / 2.0f) * cosf(angle);
      outPoints[i].y = centerY + (height / 2.0f) * sinf(angle);
  }
}

void drawLine(std::vector<Coord> coords) {

  int x1 = coords[0].x;
  int y1 = coords[0].y;
  int x2 = coords[1].x;
  int y2 = coords[1].y;

  if(!(x1 == xpos && y1 == ypos)) {

    Serial.print("Moving pen from: ");
    Serial.print(xpos);
    Serial.print(", ");
    Serial.print(ypos);
    Serial.print(" to: ");
    Serial.print(x1);
    Serial.print(", ");
    Serial.println(y1);

    line(x1, y1, 0);
    server.Loop();
  }

  Serial.print("Drawing to: ");
  Serial.print(x2);
  Serial.print(", ");
  Serial.println(y2);

  line(x2, y2, 1);

  Serial.println("Drawing complete");

  //xpos = x2;
  //ypos = y2;
}

void drawBezierCurve(std::vector<Coord> coords) {

  // Break beier down into individual lines
  Coord bezierPathPoints[NUMBER_OF_SEGMENTS + 1];
  bezierCurve(coords[0], coords[2], coords[3], coords[1], NUMBER_OF_SEGMENTS, bezierPathPoints);

  // Move to the start of the bezier curve if not already there
  if(!(coords[0].x == xpos && coords[0].y == ypos)) {
    line(coords[0].x, coords[0].y, 0);
    server.Loop();
  }

  // Draw each line
  for (int i = 0; i <= NUMBER_OF_SEGMENTS; i++) {
      line(bezierPathPoints[i].x, bezierPathPoints[i].y, 1);
      server.Loop();
  }

  // xpos = coords[1].x;
  // ypos = coords[1].y;
}

void drawEllipse(std::vector<Coord> coords) {

  int x = coords[0].x;
  int y = coords[0].y;
  int width = coords[1].x;
  int height = coords[1].y;
  int cx = x + (width / 2);
  int cy = y + (height / 2);

  float avgRadius = (width + height) / 4.0f;  // average of a and b
  int segments = static_cast<int>(avgRadius);
  if (segments < 12) segments = 12;  // minimum number of segments

  // Break beier down into individual lines
  Coord pathPoints[segments + 1];
  approximateEllipse(cx, cy, width, height, segments, pathPoints);

  // Move to the start of the bezier curve if not already there
  if(!(coords[0].x == xpos && coords[0].y == ypos)) {
    line(coords[0].x, coords[0].y, 0);
    server.Loop();
  }

  // Draw each line
  for (int i = 0; i <= segments; i++) {
      line(pathPoints[i].x, pathPoints[i].y, 1);
      server.Loop();
  }

  // xpos = coords[1].x;
  // ypos = coords[1].y;
}


void drawRectangle(std::vector<Coord> coords) {
  int x = coords[0].x;
  int y = coords[0].y;
  int width = coords[1].x;
  int height = coords[1].y;

  if(!(x == xpos && y == ypos)) {

    Serial.print("Moving pen from: ");
    Serial.print(xpos);
    Serial.print(", ");
    Serial.print(ypos);
    Serial.print(" to: ");
    Serial.print(x);
    Serial.print(", ");
    Serial.println(y);

    line(x, y, 0);
    server.Loop();
  }

  Serial.print("Drawing to: ");
  Serial.print(x + width);
  Serial.print(", ");
  Serial.println(y);

  line(x + width, y, 1);
  server.Loop();

  Serial.print("Drawing to: ");
  Serial.print(x + width);
  Serial.print(", ");
  Serial.println(y - height);

  line(x + width, y - height, 1);
  server.Loop();

  Serial.print("Drawing to: ");
  Serial.print(x);
  Serial.print(", ");
  Serial.println(y - height);

  line(x, y - height, 1);
  server.Loop();

  Serial.print("Drawing to: ");
  Serial.print(x);
  Serial.print(", ");
  Serial.println(y);

  line(x, y, 1);

  //xpos = x;
  //ypos = y;
}

void penUp() {  //singular command to lift the pen up
  servo.write(25);
}

void penDown() {  //singular command to put the pen down
  servo.write(80);
}

void releaseMotors() {
  for (int i = 0; i < 4; i++) {  //deactivates all the motor coils
    digitalWrite(xPins[i], 0);   //just picks each motor pin and send 0 voltage
    digitalWrite(yPins[i], 0);
  }
  plot(false);
}

void homeYAxis() {
  yStepper.step(-3000);  //lowers the pen holder to it's lowest position.
}


//////////////////////////////////////////////////
//  S E T U P  //
//////////////////////////////////////////////////
void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print(INIT_MSG);  // print start up message

  //pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);

  server.Setup();

  servo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  servo.write(angle);

  plot(false);  //servo to tape surface so pen can be inserted

  // set the speed of the motors
  yStepper.setSpeed(12);  // set first stepper speed (these should stay the same)
  xStepper.setSpeed(10);  // set second stepper speed (^ weird stuff happens when you push it too fast)

  penUp();      //ensure that the servo is lifting the pen carriage away from the tape
  homeYAxis();  //lower the Y axis all the way to the bottom

  ypos = 0;
  xpos = 0;

  releaseMotors();
  lcd.clear();
}

////////////////////////////////////////////////
//  L O O P  //
////////////////////////////////////////////////
void loop() {

  server.Loop();

  switch (currentState) {  //state machine that determines what to do with the input controls based on what mode the device is in

    case Initializing:
      ip = server.GetIPAddress();
      if (ip != 0) {
        displayIpAddress(ip);
        currentState = Waiting;
      }
      break;

    case Waiting:
      if(server.HaveShapeToPrint()) {
        currentState = Printing;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Printing...");
      }
      break;

    case Printing:

      if(server.HaveShapeToPrint()) {

        Serial.print("Shape Type: ");
        Serial.println(server.GetShapeType());

        // Get the coordinates from the server
        std::vector<Coord> coords = server.GetCoords();

        if(server.GetShapeType()[0] == 'X') {
          Serial.println("X received, stopping print.");
          currentState = Waiting;
          server.PrintingComplete();
          displayIpAddress(server.GetIPAddress());

          line(coords[0].x, 0, 0);  // move to new line
          xpos = 0;
          ypos = 0;    
          //yStepper.step(-2250);
          releaseMotors();
    
          break;
        }

        if(server.GetShapeType()[0] == 'D') {          
          currentState = Waiting;
          line(0, 3220, 0);  // move to new line
          penDown();
          server.PrintingComplete();
          break;
        }

        if(server.GetShapeType()[0] == 'U') {
          currentState = Waiting;
          penUp();
          line(20, 0, 0);
          server.PrintingComplete();
          displayIpAddress(server.GetIPAddress());    
          break;
        }


        switch (server.GetShapeType()[0]) {
          case 'L':
            drawLine(coords);
            break;

          case 'B':
            drawBezierCurve(coords);
            break;

          case 'R':
            drawRectangle(coords);
            break;

          case 'E':
            drawEllipse(coords);
            break;

          default:
            break;
        }  
        currentState = Waiting;
        server.PrintingComplete();
        Serial.println("Printing complete");
    
      }

      break;
  }
}