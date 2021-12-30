#include <Arduino.h>

#include "plotter.h"
#include "parser.h"

// Whiteboard
const float boardWidth = 1970.0;   // mm
const float boardHeight = 1185.0;  // mm

const float baseVelocity = 1000.0; // Steps per second
const float baseAcceleration = 1000.0; // Steps per second per second

Point::Point(float x, float y) {
  this->x = x;
  this->y = y;
}

// Calculate the length of the string at
// the point (x, y) with the law of Pythagoras
float *Point::getStrings() {
  float *position = (float *)malloc(2 * sizeof(float));
  position[0] = sqrt(pow(boardWidth/2 + this->x, 2) + pow(boardHeight - this->y, 2));
  position[1] = sqrt(pow(boardWidth/2 - this->x, 2) + pow(boardHeight - this->y, 2));
  return position;
}

// `<path>`: https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
void Plotter::executeSVG(SVG svg) {
  // Store the start coordinates to return later
  Point start = this->pos;

  float x1, y1, x2, y2, x3, y3;
  std::vector<std::pair<char, std::vector<float> > > *res;
  std::pair<char, std::vector<float> > c, old_c;
  int old_c_size;

  // Retrieve the vector with all of the commands + coordinates
  while (true) {
    res = svg.parseNextPath();
    // Check if all paths were read
    if (res == NULL) {
      // Everything drawn
      break;
    }

    // Execute path
    for (int i = 0; i < res->size(); i++) {
      // Get the next element
      c = res->at(i);

      Serial.printf("--> %c ", c.first);
      for (int i = 0; i < c.second.size(); i += 1) {
        Serial.printf("%f ", c.second[i]);
      }
      Serial.printf("\n");

      // Evaluate
      switch (c.first) {
        // Move to
        case 'M':
           x1 = c.second[0] + start.x;
           y1 = - c.second[1] + start.y;
          this->pen.penUp();
          this->moveTo(Point(x1, y1));
          break;

        // Line to
        case 'L':
           x1 = c.second[0] + start.x;
           y1 = - c.second[1] + start.y;
          this->pen.penDown();
          this->splitMove(Point(x1, y1));
          // this->moveTo(Point(x1, y1));
          break;

        // // Horizontal line
        // case 'H':
        //    x1 = c.second[0] + start.x;
        //    y1 = this->pos.y;
        //   this->pen.penDown();
        //   this->splitMove(Point(x1, y1));
        //   // this->moveTo(Point(x1, y1));
        //   break;

        // // Vertical line
        // case 'V':
        //    x1 = this->pos.x;
        //    y1 = - c.second[0] + start.y;
        //   this->pen.penDown();
        //   this->splitMove(Point(x1, y1));
        //   // this->moveTo(Point(x1, y1));
        //   break;

        // Close path
        case 'Z':
          // Point `start` saved in the beginning
          this->pen.penUp();
          this->moveTo(start);
          break;

        // Quadratic bézier curve
        case 'Q':
           x1 = c.second[0] + start.x;
           y1 = - c.second[1] + start.y;
           x2 = c.second[2] + start.x;
           y2 = - c.second[3] + start.y;
          this->pen.penDown();
          this->bezierQuadratic(Point(x1, y1), Point(x2, y2));
          break;

        // Quadratic bézier curve (continuation)
        // https://www.inf.ed.ac.uk/teaching/courses/cg/d3/bezierJoin.html
        case 'T':
          // TODO: Test
          // `P_1 = 2 * P_0 - P_old_-2`
          old_c = res->at(i - 1);
          old_c_size = old_c.second.size();
          x1 = 2.0 * old_c.second[old_c_size - 2] - old_c.second[old_c_size - 4] + start.x;
          y1 = - 2.0 * old_c.second[old_c_size - 1] + old_c.second[old_c_size - 3] + start.y;
          x2 = c.second[0] + start.x;
          y2 = - c.second[1] + start.y;
          this->pen.penDown();
          this->bezierQuadratic(Point(x1, y1), Point(x2, y2));
          break;

        // Cubic bézier curve
        case 'C':
           x1 = c.second[0] + start.x;
           y1 = - c.second[1] + start.y;
           x2 = c.second[2] + start.x;
           y2 = - c.second[3] + start.y;
           x3 = c.second[4] + start.x;
           y3 = - c.second[5] + start.y;
          this->pen.penDown();
          this->bezierCubic(Point(x1, y1), Point(x2, y2), Point(x3, y3));
          break;

        // Cubic bézier curve (continuation)
        // https://www.inf.ed.ac.uk/teaching/courses/cg/d3/bezierJoin.html
        case 'S':
          // `P_1 = 2 * P_0 - P_old_-2`
          old_c = res->at(i - 1);
          old_c_size = old_c.second.size();
          x1 = 2.0 * old_c.second[old_c_size - 2] - res->at(i - 1).second[old_c_size - 4] + start.x;
          y1 = - 2.0 * res->at(i - 1).second[old_c_size - 1] + res->at(i - 1).second[old_c_size - 3] + start.y;
          x2 = c.second[0] + start.x;
          y2 = - c.second[1] + start.y;
          x3 = c.second[2] + start.x;
          y3 = - c.second[3] + start.y;
          this->pen.penDown();
          this->bezierCubic(Point(x1, y1), Point(x2, y2), Point(x3, y3));
          break;

        default:
          break;
      }
    }
  }
}
