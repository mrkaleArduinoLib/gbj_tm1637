/*
  NAME:
  Digits order test for TM1637 controller and 7-segment displays.

  DESCRIPTION:
  The sketch display order number of all digits for testing hardware connection
  of an LED display to the controller.
  - Connect controller's pins to Arduino's pins as follows:
    - TM1637 pin CLK to Arduino pin D2
    - TM1637 pin DIO to Arduino pin D3
    - TM1637 pin Vcc to Arduino pin 5V
    - TM1637 pin GND to Arduino pin GND
  - The sketch is configured to work with 4-digit LED displays. Change it for
    your actual display.
  - The sketch utilizes font with decimal numbers.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the MIT License (MIT).

  CREDENTIALS:
  Author: Libor Gabaj
*/
#include "../extras/font7seg_decnums.h"
#include "gbj_tm1637.h"

#define SKETCH "GBJ_TM1637_TEST_DIGITS 1.0.0"

const unsigned char TM1637_DIGITS = 4; // Change for your display

const unsigned char PIN_TM1637_CLK = 2;
const unsigned char PIN_TM1637_DIO = 3;

const unsigned int PERIOD_TEST = 2000; // Miliseconds between tests
const unsigned int PERIOD_PATTERN = 500; // Miliseconds between digits

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
  // Launch fonts
  disp.setFont(gbjFont7segTable, sizeof(gbjFont7segTable));
  if (disp.isError())
  {
    errorHandler();
    return;
  }
}

void loop()
{
  if (disp.isError())
    return;
  // Test all digits one by one
  disp.printDigitOff();
  for (unsigned char digit = 0; digit < disp.getDigits(); digit++)
  {
    disp.printGlyphs(String(digit), digit);
    displayTest();
  }
  // Test all digits one after one
  disp.printDigitOff();
  String text = "";
  for (unsigned char digit = 0; digit < disp.getDigits(); digit++)
  {
    text += String(digit);
    disp.printText(text);
    displayTest();
  }
  delay(PERIOD_TEST);
}
