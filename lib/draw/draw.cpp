#include <Arduino.h>

#include "draw.h"
#include "parser.h"

// Whiteboard
const float boardWidth = 1930.0;    // mm
const float boardHeight = 1170.0;   // mm
// Delay of 500 µs in rps: 2000Hz
const float baseVelocity = 2000.0;  // Hz

Point::Point(float x, float y) {
  this->x = x;
  this->y = y;
}

// Calculate the length of the string at
// the point (x, y) with the law of Pythagoras
float *Point::calculatePosition() {
  float newS1 = sqrt(pow(boardWidth/2 + this->x, 2) + pow(boardHeight - this->y, 2));
  float newS2 = sqrt(pow(boardWidth/2 - this->x, 2) + pow(boardHeight - this->y, 2));
  // Keyword `static` makes sure that the array
  // is not deleted (out of scope) after the return statement
  static float position[2] = { newS1, newS2 };
  return position;
}

// `<path>`: https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
int Plotter::executeSVG(SVG svg) {
  // Retrieve the vector with all of the commands + coordinates
  std::vector<std::pair<char, std::vector<float> > > res = svg.followPath();

  for (int i = 0; i < res.size(); i++) {
    // Get the next element
    std::pair<char, std::vector<float> > c = res[i];
    switch (c.first) {
      // Move to
      case 'm' | 'M':
        float x = c.second[0];
        float y = c.second[1];
        // Relative coordinates
        if (c.first == 'm') {
          x += this->pos.x;
          y += this->pos.y;
        }
        this->moveTo(Point(x, y));
        break;

      // Line to
      case 'l' | 'L':
        float x = c.second[0];
        float y = c.second[1];
        // Relative coordinates
        if (c.first == 'l') {
          x += this->pos.x;
          y += this->pos.y;
        }
        // this->penDown();
        this->moveTo(Point(x, y));
        break;

      // Horizontal line
      case 'h' | 'H':
        float x = c.second[0];
        float y = this->pos.y;
        // Relative coordinates
        if (c.first == 'h') {
          x += this->pos.x;
        }
        // this->penDown();
        this->moveTo(Point(x, y));
        break;

      // Vertical line
      case 'v' | 'V':
        float x = this->pos.x;
        float y = c.second[0];
        // Relative coordinates
        if (c.first == 'v') {
          y += this->pos.y;
        }
        // this->penDown();
        this->moveTo(Point(x, y));
        break;

      // Close path
      // TODO: Implement
      case 'z' | 'Z':
        // this->moveTo(start);
        break;

      // Quadratic bézier curve
      case 'q' | 'q':
        float x1 = c.second[0];
        float y1 = c.second[1];
        float x2 = c.second[2];
        float y2 = c.second[3];
        // Relative coordinates
        if (c.first == 'q') {
          x1 += this->pos.x;
          y1 += this->pos.y;
          x2 += this->pos.x;
          y2 += this->pos.y;
        }
        // this->penDown();
        this->bezierQuadratic(Point(x1, y1), Point(x2, y2));
        break;

      // Quadratic bézier curve (continuation)
      case 't' | 'T':
        // `P_1 = 2 * P_0 - P_old_1`
        float x1 = 2.0 * this->pos.x - res[i - 1].second[0];
        float y1 = 2.0 * this->pos.y - res[i - 1].second[1];
        float x2 = c.second[0];
        float y2 = c.second[1];
        // Relative coordinates
        if (c.first == 't') {
          x1 += this->pos.x;
          y1 += this->pos.y;
          x2 += this->pos.x;
          y2 += this->pos.y;
        }
        // this->penDown();
        this->bezierQuadratic(Point(x1, y1), Point(x2, y2));
        break;

      // Cubic bézier curve
      case 'c' | 'C':
        float x1 = c.second[0];
        float y1 = c.second[1];
        float x2 = c.second[2];
        float y2 = c.second[3];
        float x3 = c.second[4];
        float y3 = c.second[5];
        // Relative coordinates
        if (c.first == 'c') {
          x1 += this->pos.x;
          y1 += this->pos.y;
          x2 += this->pos.x;
          y2 += this->pos.y;
          x3 += this->pos.x;
          y3 += this->pos.y;
        }
        // this->penDown();
        this->bezierCubic(Point(x1, y1), Point(x2, y2), Point(x3, y3));
        break;

      // Cubic bézier curve
      case 's' | 'S':
        // `P_1 = 2 * P_0 - P_old_2`
        float x1 = 2.0 * this->pos.x - res[i - 1].second[2];
        float y1 = 2.0 * this->pos.y - res[i - 1].second[3];
        float x2 = c.second[2];
        float y2 = c.second[3];
        float x3 = c.second[4];
        float y3 = c.second[5];
        // Relative coordinates
        if (c.first == 's') {
          x1 += this->pos.x;
          y1 += this->pos.y;
          x2 += this->pos.x;
          y2 += this->pos.y;
          x3 += this->pos.x;
          y3 += this->pos.y;
        }
        // this->penDown();
        this->bezierCubic(Point(x1, y1), Point(x2, y2), Point(x3, y3));
        break;

      default:
        break;
    }
  }

  return 0;
}