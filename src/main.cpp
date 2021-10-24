#include <Arduino.h>

#include "stepper.h"
#include "parser.h"
#include "plotter.h"

StepperMotor stepper1 = StepperMotor(0, dirPins[0], stepPins[0]);
StepperMotor stepper2 = StepperMotor(1, dirPins[1], stepPins[1]);

Point start = Point(0, 0);
float *startStrings = start.getStrings();

Servo pen = Servo(penPin);

Plotter plotter = {
  .pos = start, 
  // NOTE: Better way?
  .strings = { startStrings[0], startStrings[1] },
  .stepper1 = stepper1,
  .stepper2 = stepper2,
  .pen =  pen,
};

void drawSVG() {
  const std::string text =
    "<svg width=\"640\" height=\"200\" viewBox=\"0 0 640 200\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">"
      // "<path d=\"M184.443 69.52C183.749 69.52 183.163 69.76 182.683 70.24C182.203 70.7733 181.963 71.3867 181.963 72.08V118.56H166.843C166.149 118.56 165.536 118.8 165.003 119.28C164.523 119.76 164.283 120.347 164.283 121.04C164.283 121.733 164.523 122.32 165.003 122.8C165.536 123.333 166.149 123.6 166.843 123.6H202.123C202.816 123.6 203.403 123.36 203.883 122.88C204.363 122.4 204.603 121.787 204.603 121.04C204.603 120.347 204.363 119.76 203.883 119.28C203.403 118.8 202.816 118.56 202.123 118.56H187.003V72.08C187.003 71.3867 186.736 70.7733 186.203 70.24C185.723 69.76 185.136 69.52 184.443 69.52ZM222.553 69.52C218.66 69.52 215.193 70.3733 212.153 72.08C209.113 73.84 206.766 76.1867 205.113 79.12C203.46 82.0533 202.633 85.2533 202.633 88.72C202.633 92.3467 203.38 95.7333 204.873 98.88C206.42 102.08 208.553 104.64 211.273 106.56C213.993 108.48 217.113 109.44 220.633 109.44C223.833 109.44 226.553 108.827 228.793 107.6C231.086 106.373 232.793 104.907 233.913 103.2C235.086 101.493 235.673 99.9733 235.673 98.64C235.673 97.68 235.033 96.8267 233.753 96.08L208.953 81.92C210.286 79.36 212.1 77.4133 214.393 76.08C216.686 74.8 219.38 74.16 222.473 74.16C224.18 74.16 225.966 74.5867 227.833 75.44C229.753 76.2933 231.273 77.3067 232.393 78.48C232.766 79.0133 233.3 79.28 233.993 79.28C234.686 79.28 235.246 79.04 235.673 78.56C236.153 78.1333 236.393 77.5733 236.393 76.88C236.393 76.3467 236.18 75.8133 235.753 75.28C234.313 73.6267 232.34 72.24 229.833 71.12C227.38 70.0533 224.953 69.52 222.553 69.52ZM230.713 99.28C229.966 101.093 228.793 102.453 227.193 103.36C225.593 104.267 223.433 104.72 220.713 104.72C218.153 104.72 215.86 104 213.833 102.56C211.806 101.173 210.206 99.28 209.033 96.88C207.913 94.48 207.326 91.84 207.273 88.96C207.273 87.8933 207.326 87.04 207.433 86.4L230.713 99.28ZM260.151 69.52C257.058 69.52 254.151 70.2667 251.431 71.76C248.765 73.3067 246.631 75.4133 245.031 78.08C243.485 80.7467 242.711 83.68 242.711 86.88C242.711 90.5067 243.538 94.0533 245.191 97.52C246.898 101.04 249.138 103.893 251.911 106.08C254.685 108.32 257.565 109.44 260.551 109.44C263.325 109.44 265.591 108.987 267.351 108.08C269.165 107.173 270.898 105.76 272.551 103.84C272.978 103.413 273.191 102.853 273.191 102.16C273.191 101.467 272.951 100.88 272.471 100.4C272.045 99.92 271.485 99.68 270.791 99.68C270.045 99.68 269.431 99.9733 268.951 100.56C267.778 101.947 266.578 102.987 265.351 103.68C264.125 104.427 262.578 104.8 260.711 104.8C258.578 104.8 256.498 103.947 254.471 102.24C252.445 100.533 250.791 98.2933 249.511 95.52C248.231 92.8 247.591 89.9733 247.591 87.04C247.591 84.3733 248.205 82.0533 249.431 80.08C250.711 78.16 252.311 76.6933 254.231 75.68C256.205 74.6667 258.178 74.16 260.151 74.16C264.738 74.16 268.311 76 270.871 79.68C271.565 80.64 272.311 81.12 273.111 81.12C273.805 81.12 274.391 80.88 274.871 80.4C275.351 79.92 275.591 79.36 275.591 78.72C275.591 78.1867 275.325 77.5733 274.791 76.88C273.405 74.8533 271.405 73.12 268.791 71.68C266.178 70.24 263.298 69.52 260.151 69.52ZM286.425 69.44C285.732 69.44 285.118 69.68 284.585 70.16C284.105 70.6933 283.865 71.28 283.865 71.92V121.92C283.865 122.613 284.105 123.2 284.585 123.68C285.118 124.213 285.732 124.48 286.425 124.48C287.172 124.48 287.758 124.213 288.185 123.68C288.665 123.2 288.905 122.613 288.905 121.92V97.36C290.505 100.667 292.718 103.547 295.545 106C298.425 108.453 301.358 109.68 304.345 109.68C308.292 109.68 311.065 107.493 312.665 103.12C314.265 98.7467 315.065 91.8933 315.065 82.56C315.065 77.28 314.985 73.7333 314.825 71.92C314.772 71.2267 314.532 70.64 314.105 70.16C313.678 69.7333 313.118 69.52 312.425 69.52C311.785 69.52 311.198 69.76 310.665 70.24C310.185 70.72 309.945 71.28 309.945 71.92C309.945 72.72 309.972 73.4667 310.025 74.16L310.105 82C310.105 91.0133 309.518 97.04 308.345 100.08C307.172 103.173 305.705 104.72 303.945 104.72C301.972 104.72 300.025 103.893 298.105 102.24C296.185 100.64 294.505 98.8267 293.065 96.8C291.678 94.8267 290.692 93.2 290.105 91.92C289.305 90.32 288.905 88.0267 288.905 85.04V72.08C288.905 71.28 288.665 70.64 288.185 70.16C287.705 69.68 287.118 69.44 286.425 69.44ZM326.916 87.12C326.276 87.12 325.689 87.36 325.156 87.84C324.676 88.32 324.436 88.88 324.436 89.52C324.436 90.16 324.676 90.72 325.156 91.2C325.689 91.68 326.276 91.92 326.916 91.92H346.356C347.049 91.92 347.609 91.68 348.036 91.2C348.516 90.7733 348.756 90.2133 348.756 89.52C348.756 88.88 348.516 88.32 348.036 87.84C347.609 87.36 347.049 87.12 346.356 87.12H326.916ZM361.823 69.52C361.183 69.4667 360.596 69.7067 360.063 70.24C359.583 70.8267 359.343 71.44 359.343 72.08V121.52C359.343 122.16 359.583 122.72 360.063 123.2C360.596 123.733 361.21 124 361.903 124C362.596 124 363.183 123.733 363.663 123.2C364.196 122.72 364.463 122.16 364.463 121.52V74.72L389.183 76.4C389.93 76.4533 390.543 76.24 391.023 75.76C391.503 75.28 391.743 74.6667 391.743 73.92C391.743 73.28 391.503 72.72 391.023 72.24C390.543 71.76 389.983 71.4933 389.343 71.44L361.823 69.52ZM423.935 68.88C423.135 68.88 422.522 69.1467 422.095 69.68C421.668 70.16 421.455 70.7733 421.455 71.52V73.76C420.015 72.48 418.282 71.44 416.255 70.64C414.228 69.8933 412.202 69.52 410.175 69.52C407.295 69.52 404.708 70.24 402.415 71.68C400.122 73.12 398.308 75.1467 396.975 77.76C395.695 80.4267 395.055 83.44 395.055 86.8C395.055 90.4267 395.802 93.9733 397.295 97.44C398.842 100.96 401.055 103.84 403.935 106.08C406.868 108.32 410.308 109.44 414.255 109.44C416.708 109.44 418.868 109.12 420.735 108.48C422.602 107.84 424.495 107.067 426.415 106.16V71.52C426.415 70.7733 426.175 70.16 425.695 69.68C425.215 69.1467 424.628 68.88 423.935 68.88ZM410.175 74.24C412.255 74.24 414.335 74.72 416.415 75.68C418.495 76.6933 420.095 77.76 421.215 78.88L421.535 103.12C420.682 103.493 419.562 103.84 418.175 104.16C416.842 104.533 415.588 104.72 414.415 104.72C411.802 104.72 409.402 103.947 407.215 102.4C405.028 100.853 403.295 98.72 402.015 96C400.788 93.3333 400.175 90.32 400.175 86.96C400.175 83.0667 401.108 79.9733 402.975 77.68C404.842 75.3867 407.242 74.24 410.175 74.24ZM441.345 69.52C440.705 69.52 440.118 69.7333 439.585 70.16C439.105 70.64 438.865 71.2 438.865 71.84V122.24C438.865 122.88 439.105 123.413 439.585 123.84C440.118 124.267 440.732 124.48 441.425 124.48C442.065 124.48 442.625 124.267 443.105 123.84C443.585 123.413 443.825 122.88 443.825 122.24V102.64C445.532 104.613 447.612 106.133 450.065 107.2C452.518 108.267 455.078 108.8 457.745 108.8C461.958 108.8 465.345 107.2 467.905 104C470.465 100.853 471.745 96.5867 471.745 91.2C471.745 84.8533 470.278 79.7333 467.345 75.84C464.465 72 460.118 70.08 454.305 70.08C452.492 70.08 450.625 70.2933 448.705 70.72C446.785 71.2 445.132 71.7333 443.745 72.32V71.68C443.745 71.1467 443.505 70.64 443.025 70.16C442.545 69.7333 441.985 69.52 441.345 69.52ZM453.665 74.88C462.518 74.88 466.945 80.32 466.945 91.2C466.945 95.1467 466.092 98.2133 464.385 100.4C462.732 102.64 460.492 103.76 457.665 103.76C455.105 103.76 452.758 103.28 450.625 102.32C448.545 101.36 446.892 100.053 445.665 98.4C444.438 96.7467 443.825 94.9333 443.825 92.96V76.88C444.838 76.4533 446.252 76 448.065 75.52C449.932 75.0933 451.798 74.88 453.665 74.88Z\" fill=\"black\"/>"
      "<path d=\"L -400 400Z\" stroke=\"black\"/>"
    "</svg>";
  SVG svg = SVG(text);
  plotter.executeSVG(svg);
}

void setup() {
  Serial.begin(9600);
  setMotorState(true);
  delay(5000);

  Serial.println("Move the plotter to the starting position ...");

  plotter.joystick();

  Serial.printf("Selected position: %f, %f\n", plotter.pos.x, plotter.pos.y);
  Serial.println("Drawing svg ...");

  drawSVG();

  // Return to start
  plotter.moveTo(start);
}

void loop() {}