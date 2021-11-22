#include <Arduino.h>

#include "plotter.h"
#include "parser.h"

// Whiteboard
const float boardWidth = 1970.0;    // mm
const float boardHeight = 1185.0;   // mm

// Delay of 500 µs in sps: 2000Hz
const float baseVelocity = 1000.0;  // Hz

// Auxiliary function to repeat char
char *repeatString(char c, int num) {
  static char res[255];
  num = num < 255 ? num : 255;
  for (int i = 0; i < num; i++) {
    res[i] = c;
  }
  return res;
}

Point::Point(float x, float y) {
  this->x = x;
  this->y = y;
}

// Calculate the length of the string at
// the point (x, y) with the law of Pythagoras
float *Point::getStrings() {
  // Keyword `static` makes sure that the array
  // is not deleted (out of scope) after the return statement
  static float position[2];
  position[0] = sqrt(pow(boardWidth/2 + this->x, 2) + pow(boardHeight - this->y, 2));
  position[1] = sqrt(pow(boardWidth/2 - this->x, 2) + pow(boardHeight - this->y, 2));
  return position;
}

// `<path>`: https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
int Plotter::executeSVG(SVG svg) {
  // Store the start coordinates to return later
  Point start = this->pos;

  float x1, y1, x2, y2, x3, y3;
  // Retrieve the vector with all of the commands + coordinates
  // std::vector<std::pair<char, std::vector<float> > > res = svg.followPath();
  std::vector<std::pair<char, std::vector<float> > > res = svg.actions;

  for (int i = 0; i < res.size(); i++) {
    // Get the next element
    std::pair<char, std::vector<float> > c = res[i];
    
    // Logging
    // // Clear line with 100 whitespaces
    // char whitespaces[110];
    // strcat(whitespaces, "\r");
    // strcat(whitespaces, repeatString(' ', 100));
    // Serial.printf("%s", whitespaces);
    Serial.printf("--> %c ", c.first);
    for (int i = 0; i < c.second.size(); i += 1) {
      Serial.printf("%f ", c.second[i]);
    }

    // Evaluate
    switch (c.first) {
      // Move to
      case 'm':
      case 'M':
         x1 = c.second[0] + start.x;
         y1 = c.second[1] + start.y;
        // Relative coordinates
        if (c.first == 'm') {
          x1 += this->pos.x - start.x;
          y1 += this->pos.y - start.y;
        }
        this->pen.penUp();
        this->moveTo(Point(x1, y1));
        break;

      // Line to
      case 'l':
      case 'L':
         x1 = c.second[0] + start.x;
         y1 = c.second[1] + start.y;
        // Relative coordinates
        if (c.first == 'l') {
          x1 += this->pos.x - start.x;
          y1 += this->pos.y - start.y;
        }
        this->pen.penDown();
        this->splitMove(Point(x1, y1));
        // this->moveTo(Point(x1, y1));
        break;

      // Horizontal line
      case 'h':
      case 'H':
         x1 = c.second[0] + start.x;
         y1 = this->pos.y;
        // Relative coordinates
        if (c.first == 'h') {
          x1 += this->pos.x;
        }
        this->pen.penDown();
        this->splitMove(Point(x1, y1));
        // this->moveTo(Point(x1, y1));
        break;

      // Vertical line
      case 'v':
      case 'V':
         x1 = this->pos.x;
         y1 = c.second[0] + start.y;
        // Relative coordinates
        if (c.first == 'v') {
          y1 += this->pos.y - start.y;
        }
        this->pen.penDown();
        this->splitMove(Point(x1, y1));
        // this->moveTo(Point(x1, y1));
        break;

      // Close path
      case 'z':
      case 'Z':
        // Point `start` saved in the beginning
        this->pen.penUp();
        this->moveTo(start);
        break;

      // Quadratic bézier curve
      case 'q':
      case 'Q':
         x1 = c.second[0] + start.x;
         y1 = c.second[1] + start.y;
         x2 = c.second[2] + start.x;
         y2 = c.second[3] + start.y;
        // Relative coordinates
        if (c.first == 'q') {
          x1 += this->pos.x - start.x;
          y1 += this->pos.y - start.y;
          x2 += this->pos.x - start.x;
          y2 += this->pos.y - start.y;
        }
        this->pen.penDown();
        this->bezierQuadratic(Point(x1, y1), Point(x2, y2));
        break;

      // Quadratic bézier curve (continuation)
      // https://www.inf.ed.ac.uk/teaching/courses/cg/d3/bezierJoin.html
      case 't':
      case 'T':
        // `P_1 = 2 * P_0 - P_old_1`
         x1 = 2.0 * this->pos.x - res[i - 1].second[0] + start.x;
         y1 = 2.0 * this->pos.y - res[i - 1].second[1] + start.y;
         x2 = c.second[0] + start.x;
         y2 = c.second[1] + start.y;
        // Relative coordinates
        if (c.first == 't') {
          x1 += this->pos.x;
          y1 += this->pos.y;
          x2 += this->pos.x;
          y2 += this->pos.y;
        }
        this->pen.penDown();
        this->bezierQuadratic(Point(x1, y1), Point(x2, y2));
        break;

      // Cubic bézier curve
      case 'c':
      case 'C':
         x1 = c.second[0] + start.x;
         y1 = c.second[1] + start.y;
         x2 = c.second[2] + start.x;
         y2 = c.second[3] + start.y;
         x3 = c.second[4] + start.x;
         y3 = c.second[5] + start.y;
        // Relative coordinates
        if (c.first == 'c') {
          x1 += this->pos.x - start.x;
          y1 += this->pos.y - start.y;
          x2 += this->pos.x - start.x;
          y2 += this->pos.y - start.y;
          x3 += this->pos.x - start.x;
          y3 += this->pos.y - start.y;
        }
        this->pen.penDown();
        this->bezierCubic(Point(x1, y1), Point(x2, y2), Point(x3, y3));
        break;

      // Cubic bézier curve (continuation)
      // https://www.inf.ed.ac.uk/teaching/courses/cg/d3/bezierJoin.html
      case 's':
      case 'S':
        // `P_1 = 2 * P_0 - P_old_2`
         x1 = 2.0 * this->pos.x - res[i - 1].second[2] + start.x;
         y1 = 2.0 * this->pos.y - res[i - 1].second[3] + start.y;
         x2 = c.second[0] + start.x;
         y2 = c.second[1] + start.y;
         x3 = c.second[2] + start.x;
         y3 = c.second[3] + start.y;
        // Relative coordinates
        if (c.first == 's') {
          x1 += this->pos.x - start.x;
          y1 += this->pos.y - start.y;
          x2 += this->pos.x - start.x;
          y2 += this->pos.y - start.y;
          x3 += this->pos.x - start.x;
          y3 += this->pos.y - start.y;
        }
        this->pen.penDown();
        this->bezierCubic(Point(x1, y1), Point(x2, y2), Point(x3, y3));
        break;

      default:
        break;
    }
  }

  return 0;
}