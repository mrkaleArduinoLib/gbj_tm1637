/*
  NAME:
  Decimal dots/colon functionality test for TM1637 controller and 7-segment
  displays.

  DESCRIPTION:
  The sketch tests all radix segments of a display by displaying them one by one
  on each LED digit one by one.
  - Connect controller's pins to Arduino's pins as follows:
    - TM1637 pin CLK to Arduino pin D2
    - TM1637 pin DIO to Arduino pin D3
    - TM1637 pin Vcc to Arduino pin 5V
    - TM1637 pin GND to Arduino pin GND
  - The sketch is configured to work with 4-digit LED displays.
  - The sketch does not utilize fonts and prints to controller digits directly.
  - The sketch displays `8` for each tested digit, i.e., displays all digit's
    segments including radix one, if present in hardware.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the MIT License (MIT).

  CREDENTIALS:
  Author: Libor Gabaj
*/
#include "gbj_tm1637.h"

#define SKETCH "GBJ_TM1637_TEST_DOTS 1.0.0"

const unsigned int PERIOD_TEST = 2000; // Time in miliseconds between tests
const unsigned int PERIOD_PATTERN =
  500; // Time delay in miliseconds for displaying a pattern
const unsigned char PIN_TM1637_CLK = 2;
const unsigned char PIN_TM1637_DIO = 3;
const unsigned char TM1637_DIGITS = 4;

gbj_tm1637 disp = gbj_tm1637(PIN_TM1637_CLK, PIN_TM1637_DIO, TM1637_DIGITS);

void errorHandler()
{
  if (disp.isSuccess())
    return;
  Serial.print("Error: ");
  Serial.println(disp.getLastResult());
}

void displayTest()
{
  if (disp.display())
    errorHandler();
  delay(PERIOD_PATTERN);
}

void setup()
{
  Serial.begin(9600);
  Serial.println(SKETCH);
  Serial.println("Libraries:");
  Serial.println(gbj_tm1637::VERSION);
  Serial.println("---");
  // Initialize controller
  if (disp.begin())
  {
    errorHandler();
    return;
  }
}

void loop()
{
  if (disp.isError())
    return;
  disp.displayClear();
  // Test all digits one by one
  for (unsigned char digit = 0; digit < disp.getDigits(); digit++)
  {
    disp.printDigitOn(digit);
    disp.printRadixOn(digit);
    displayTest();
  }
  delay(PERIOD_TEST);
}
