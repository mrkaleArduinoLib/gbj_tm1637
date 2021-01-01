/*
  NAME:
  Brigthness test of a display module with TM1637 controller.

  DESCRIPTION:
  The sketch turns on all glyph segments of all digital tubes and changes their
  brightness by controlling contrast.
  - Connect controller's pins to Arduino's pins as follows:
    - TM1638 pin CLK to Arduino pin D2
    - TM1638 pin DIO to Arduino pin D3
    - TM1638 pin Vcc to Arduino pin 5V
    - TM1638 pin GND to Arduino pin GND
  - The sketch is configured to work with 4-digit LED displays.
  - The sketch does not manipulate radix segments of digital tubes.
  - The sketch signals brightness level by displaying its number on each digit.
  - The sketch utilizes basic font.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the MIT License (MIT).

  CREDENTIALS:
  Author: Libor Gabaj
*/
#include "../extras/font7seg_basic.h"
#include "gbj_tm1637.h"

#define SKETCH "GBJ_TM1637_TEST_BRIGHTNESS 1.0.0"

const unsigned int PERIOD_TEST = 1000; // Time in miliseconds between tests
const unsigned int PERIOD_VALUE =
  500; // Time delay in miliseconds for displaying a value
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
  Serial.println(disp.getLastCommand());
}

void displayTest()
{
  if (disp.display())
    errorHandler();
  delay(PERIOD_VALUE);
}

void setup()
{
  Serial.begin(9600);
  Serial.println(SKETCH);
  Serial.println("Libraries:");
  Serial.println(gbj_tm1637::VERSION);
  Serial.println("Fonts:");
  Serial.println(GBJ_FONT7SEG_VERSION);
  Serial.println("---");
  // Initialize controller
  disp.begin();
  if (disp.isError())
  {
    errorHandler();
    return;
  }

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
  // Test all contrast levels
  for (unsigned char contrast = 0; contrast <= disp.getContrastMax();
       contrast++)
  {
    disp.setContrast(contrast);
    String contrastText;
    for (unsigned char digit = 0; digit < disp.getDigits(); digit++)
    {
      contrastText.concat(disp.getContrast());
    }
    disp.printText(contrastText);
    displayTest();
  }
  delay(PERIOD_TEST);
}
