#include <Arduino.h>
#include "plotter.h"

const float thresh = 0.1;

// Implementation of De Casteljau's algorithm
// parametric function with `t` -> move t from 0 to 1

// `B(t) = (1 - t)^2 P_0 + 2t (1 - t) P_1 + t^2 P_2`
void Plotter::bezierQuadratic(Point p1, Point p2) {
  // Point p0 = this->pos;
  // // `accuracy` defines the amount of steps between
  // // Add up the distances between all points / ... -> accuracy (capped at 50)
  // float aproxLength =
  //   pow(pow(this->pos.x - p1.x, 2) + pow(this->pos.y - this->pos.y, 2), 0.5) + pow(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2), 0.5);
  // // float accuracy = aproxLength < 50.0 ? aproxLength * 2.0 : aproxLength + 50.0;
  // // float accuracy = aproxLength < 2.0 ? 3 : 10.0 * log(aproxLength);
  // int accuracy = round(aproxLength < 2.0 ? 5.0 : 5.0 * log2(aproxLength));
  // float increase = 1.0/(float)accuracy;

  // float x, y;
  // // Move parameter t from 0.0 to 1.0
  // float t = 0.0;
  // for (int i = 0; i < accuracy; i++) {
  //   t += increase;
  //   x = pow((1.0 - t), 2) * p0.x + 2.0 * t * (1.0 - t) * p1.x + pow(t, 2) * p2.x;
  //   y = pow((1.0 - t), 2) * p0.y + 2.0 * t * (1.0 - t) * p1.y + pow(t, 2) * p2.y;
  //   this->moveTo(Point(x, y));
  // }

  // delay(100);

  Point p0 = this->pos;  

  float t = 1;
  float T = 0;
  float Tx, Ty;
  float mT, dTx, dTy;

  // Calculate the current point
  Tx = pow((1.0 - T), 2) * p0.x + 2.0 * T * (1.0 - T) * p1.x + pow(T, 2) * p2.x;
  Ty = pow((1.0 - T), 2) * p0.y + 2.0 * T * (1.0 - T) * p1.y + pow(T, 2) * p2.y;

  while (true) {
    // Calculate velocity of bezier curve 
    dTx = 2 * p2.x * T - 2 * p1.x * T + 2 * p1.x * (1 - T) - 2 * p0.x * (1 - T);
    dTy = 2 * p2.y * T - 2 * p1.y * T + 2 * p1.y * (1 - T) - 2 * p0.y * (1 - T);
    mT = dTy / dTx;

    // Try to approximate with threshold
    float tx, ty, m;
    while (true) {
      t += 0.01;
      tx = pow((1.0 - t), 2) * p0.x + 2.0 * t * (1.0 - t) * p1.x + pow(t, 2) * p2.x;
      ty = pow((1.0 - t), 2) * p0.y + 2.0 * t * (1.0 - t) * p1.y + pow(t, 2) * p2.y;
      m = (ty - Ty) / (tx - Tx);

      // Too much difference
      if ((mT - m) > thresh || (mT - m) < (-thresh)) {
        // If the first calculated point is already too inaccurate
        // don't go back, because then t would be the same as last time 
        t -= 0.01;
        Serial.printf("Found point to draw at %f!", t);
        if (t == T) {
          t += 0.01;
        }
        T = t;
        break;
      }

      // Stop
      if (t >= (float)1) {
        T = 1.0;
        break;
      }
    }

    // Calculate the new point
    Tx = pow((1.0 - T), 2) * p0.x + 2.0 * T * (1.0 - T) * p1.x + pow(T, 2) * p2.x;
    Ty = pow((1.0 - T), 2) * p0.y + 2.0 * T * (1.0 - T) * p1.y + pow(T, 2) * p2.y;

    // Draw the newly calculated segment
    this->moveTo(Point(Tx, Ty));

    // Curve is finished
    if (T == (float)1) {
      break;
    }
  }
}

// `B(t) = (1 - t)^3 P_0 + 3t (1 - t)^2 P_1 + 3t^2 (1 - t) P_2 + t^3 P_3`
void Plotter::bezierCubic(Point p1, Point p2, Point p3) {
  // Point p0 = this->pos;
  // // `accuracy` defines the amount of steps between
  // // Add up the distances between all points / ... -> accuracy (capped at 50)
  // float aproxLength =
  //   pow(pow(this->pos.x - p1.x, 2) + pow(this->pos.y - this->pos.y, 2), 0.5)
  //     + pow(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2), 0.5) + pow(pow(p2.x - p3.x, 2) + pow(p2.y - p3.y, 2), 0.5);
  // int accuracy = round(aproxLength < 2.0 ? 5.0 : 5.0 * log2(aproxLength));
  // float increase = 1.0/(float)accuracy;

  // float x, y;
  // // Move parameter t from 0.0 to 1.0
  // float t = 0.0;
  // for (int i = 0; i < accuracy; i++) {
  //   t += increase;
  //   x = pow((1.0 - t), 3) * p0.x + 3.0 * t * pow((1.0 - t), 2) * p1.x + 3.0 * pow(t, 2) * (1.0 - t) * p2.x + pow(t, 3) * p3.x;
  //   y = pow((1.0 - t), 3) * p0.y + 3.0 * t * pow((1.0 - t), 2) * p1.y + 3.0 * pow(t, 2) * (1.0 - t) * p2.y + pow(t, 3) * p3.y;
  //   this->moveTo(Point(x, y));
  // }

  // delay(100);

  Point p0 = this->pos;

  float t = 0;
  float T = 0;
  float Tx, Ty;
  float mT, dTx, dTy;

  // Calculate the current point
  Tx = pow((1.0 - T), 3) * p0.x + 3.0 * T * pow((1.0 - T), 2) * p1.x + 3.0 * pow(T, 2) * (1.0 - T) * p2.x + pow(T, 3) * p3.x;
  Ty = pow((1.0 - T), 3) * p0.y + 3.0 * T * pow((1.0 - T), 2) * p1.y + 3.0 * pow(T, 2) * (1.0 - T) * p2.y + pow(T, 3) * p3.y;

  while (true) {
    // Calculate velocity of bezier curve 
    dTx = 3 * p3.x * pow(T, 2) - 3 * p2.x * pow(T, 2) + 6 * p2.x * (1 - T) * T - 6 * p1.x * (1 - T) * T + 3 * p1.x * pow((1 - T), 2) - 3 * p0.x * pow((1 - T), 2);
    dTy = 3 * p3.y * pow(T, 2) - 3 * p2.y * pow(T, 2) + 6 * p2.y * (1 - T) * T - 6 * p1.y * (1 - T) * T + 3 * p1.y * pow((1 - T), 2) - 3 * p0.y * pow((1 - T), 2);
    mT = dTy / dTx;

    // Try to approximate with threshold until difference too much
    float tx, ty, m;
    while (true) {
      // Increase
      t += 0.01; 
      tx = pow((1.0 - t), 3) * p0.x + 3.0 * t * pow((1.0 - t), 2) * p1.x + 3.0 * pow(t, 2) * (1.0 - t) * p2.x + pow(t, 3) * p3.x;
      ty = pow((1.0 - t), 3) * p0.y + 3.0 * t * pow((1.0 - t), 2) * p1.y + 3.0 * pow(t, 2) * (1.0 - t) * p2.y + pow(t, 3) * p3.y;
      m = (ty - Ty) / (tx - Tx);

      // Too much difference
      if ((mT - m) > thresh || (mT - m) < (-thresh)) {
        // If the first calculated point is already too inaccurate
        // don't go back, because then t would be the same as last time 
        t -= 0.01;
        if (t == T) {
          t += 0.01;
        }
        T = t;
        break;
      }

      // Stop
      if (t >= (float)1) {
        T = 1.0;
        break;
      }
    }

    // Calculate the new point
    Tx = pow((1.0 - T), 3) * p0.x + 3.0 * T * pow((1.0 - T), 2) * p1.x + 3.0 * pow(T, 2) * (1.0 - T) * p2.x + pow(T, 3) * p3.x;
    Ty = pow((1.0 - T), 3) * p0.y + 3.0 * T * pow((1.0 - T), 2) * p1.y + 3.0 * pow(T, 2) * (1.0 - T) * p2.y + pow(T, 3) * p3.y;

    // Draw the newly calculated segment
    this->moveTo(Point(Tx, Ty));

    // Curve is finished
    if (T == (float)1) {
      break;
    }
  }
}

// // Test this later
// Point *pointAtBezierQuadratic(float t) {
//   float x = pow((1.0 - t), 2) * p0.x + 2.0 * t * (1.0 - t) * p1.x + pow(t, 2) * p2.x;
//   float y = pow((1.0 - t), 2) * p0.y + 2.0 * t * (1.0 - t) * p1.y + pow(t, 2) * p2.y;
//   return new Point(x, y);
// }
// 
// void bezierQuadraticNew(Point p0, Point p1, Point p2) {
//   const float thresh = 0.1;
// 
//   float t = 1;
//   float T = 0;
//   float Tx, Ty;
//   float mT, dTx, dTy;
//   while (true) {
//     // TODO: Check the derivative
//     // Calculate velocity of bezier curve 
//     dTx = 2 * p2.x * t - 2 * p1.x * t + 2 * p1.x * (1 - t) - 2 * p0.x (1 - t)
//     dTy = 2 * p2.y * t - 2 * p1.y * t + 2 * p1.y * (1 - t) - 2 * p0.y (1 - t)
//     mT = dTy / dTx;
//     // Calculate the current point
//     Tx = pow((1.0 - t), 2) * p0.x + 2.0 * t * (1.0 - t) * p1.x + pow(t, 2) * p2.x;
//     Ty = pow((1.0 - t), 2) * p0.y + 2.0 * t * (1.0 - t) * p1.y + pow(t, 2) * p2.y;
//     // Try to approximate with threshold
//     float tx, ty, m;
//     while (true) {
//       tx = pow((1.0 - t), 2) * p0.x + 2.0 * t * (1.0 - t) * p1.x + pow(t, 2) * p2.x;
//       ty = pow((1.0 - t), 2) * p0.y + 2.0 * t * (1.0 - t) * p1.y + pow(t, 2) * p2.y;
//       m = (ty - Ty) / (tx - Tx);
//       if ((mT - m) < thresh) {
//         T = t;
//         t = 0;
//         break;
//       }
//       // Split t and try again
//       t = (t - T) / 2
//     }
// 
//     // Draw the newly calculated segment
//     this->moveTo(Point(tx, ty));
// 
//     // Curve is finished
//     if (T == 1) {
//       break;
//     }
//   }
// }
// 
// void bezierCubicNew(Point p0, Point p1, Point p2, Point p3) {
//   const float thresh = 0.1;
// 
//   float t = 1;
//   float T = 0;
//   float Tx, Ty;
//   float mT, dTx, dTy;
//   while (true) {
//     // TODO: Check the derivative
//     // Calculate velocity of bezier curve 
//     dTx = 3 * p3.x * pow(t, 2) - 3 * p2.x * pow(t, 2) + 6 * p2.x * (1 - t) * t - 6 * p1.x * (1 - t) * t + 3 * p1.x * pow((1 - t), 2) - 3 * p0.x * pow((1 - t), 2);
//     dTy = 3 * p3.y * pow(t, 2) - 3 * p2.y * pow(t, 2) + 6 * p2.y * (1 - t) * t - 6 * p1.y * (1 - t) * t + 3 * p1.y * pow((1 - t), 2) - 3 * p0.y * pow((1 - t), 2);
//     mT = dTy / dTx;
//     // Calculate the current point
//     Tx = pow((1.0 - t), 3) * p0.x + 3.0 * t * pow((1.0 - t), 2) * p1.x + 3.0 * pow(t, 2) * (1.0 - t) * p2.x + pow(t, 3) * p3.x;
//     Ty = pow((1.0 - t), 3) * p0.y + 3.0 * t * pow((1.0 - t), 2) * p1.y + 3.0 * pow(t, 2) * (1.0 - t) * p2.y + pow(t, 3) * p3.y;
//     // Try to approximate with threshold
//     float tx, ty, m;
//     while (true) {
//       tx = pow((1.0 - t), 3) * p0.x + 3.0 * t * pow((1.0 - t), 2) * p1.x + 3.0 * pow(t, 2) * (1.0 - t) * p2.x + pow(t, 3) * p3.x;
//       ty = pow((1.0 - t), 3) * p0.y + 3.0 * t * pow((1.0 - t), 2) * p1.y + 3.0 * pow(t, 2) * (1.0 - t) * p2.y + pow(t, 3) * p3.y;
//       m = (ty - Ty) / (tx - Tx);
//       if ((mT - m) < thresh) {
//         T = t;
//         t = 0;
//         break;
//       }
//       // Split t and try again
//       t = (t - T) / 2
//     }
// 
//     // Draw the newly calculated segment
//     this->moveTo(Point(tx, ty));
// 
//     // Curve is finished
//     if (T == 1) {
//       break;
//     }
//   }
// }