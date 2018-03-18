/*
  NAME:
  Decimal dots/colon functionality test for TM1637 controller and 7-segment displays

  DESCRIPTION:
  The sketch tests all radix segments of a display by displaying them one by one
  on each LED digit one by one.
  - Connect controller's pins to Arduino's pins as follows:
    - TM1637 pin CLK to Arduino pin D2
    - TM1637 pin DIO to Arduino pin D3
    - TM1637 pin Vcc to Arduino pin 5V
    - TM1637 pin GND to Arduino pin GND
  - The sketch is configured to work with 4-digit LED displays.
  - The sketch does not utilize fonts and prints to controller grids directly.
  - The sketch displays `1` for each tested digit.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the MIT License (MIT).

  CREDENTIALS:
  Author: Libor Gabaj
*/
#include "gbj_tm1637.h"
#define SKETCH "GBJ_TM1637_TEST_DOTS 1.0.0"

const unsigned int PERIOD_TEST = 2000;  // Time in miliseconds between tests
const unsigned int PERIOD_PATTERN = 500; // Time delay in miliseconds for displaying a pattern
const unsigned char PIN_TM1637_CLK = 2;
const unsigned char PIN_TM1637_DIO = 3;
const unsigned char TM1637_DIGITS = 4;

gbj_tm1637 Sled = gbj_tm1637(PIN_TM1637_CLK, PIN_TM1637_DIO, TM1637_DIGITS);


void errorHandler()
{
  if (Sled.isSuccess()) return;
  Serial.print("Error: ");
  Serial.println(Sled.getLastResult());
}


void displayTest()
{
  if (Sled.display()) errorHandler();
  delay(PERIOD_PATTERN);
}


void setup()
{
  Serial.begin(9600);
  Serial.println(SKETCH);
  Serial.println("Libraries:");
  Serial.println(GBJ_TM1637_VERSION);
  Serial.println("---");
  // Initialize controller
  if (Sled.begin())
  {
    errorHandler();
    return;
  }
}


void loop()
{
  if (Sled.isError()) return;
  Sled.printClear();
  Sled.printRadixClear();
  // Test all digits one by one
  for (unsigned char grid = 0; grid < Sled.getPrintWidth(); grid++)
  {
    Sled.printGrid(grid, 0b00000110);
    Sled.printRadixOn(grid);
    displayTest();
  }
  delay(PERIOD_TEST);
}
