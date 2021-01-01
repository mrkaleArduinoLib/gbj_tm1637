/*
  NAME:
  Demonstration of text printing with the library gbj_tm1637.

  DESCRIPTION:
  The sketch prints some informative texts on the display and uptime for 2
  minutes.
  - Connect controller's pins to Arduino's pins as follows:
    - TM1637 pin CLK to Arduino pin D2
    - TM1637 pin DIO to Arduino pin D3
    - TM1637 pin Vcc to Arduino pin 5V
    - TM1637 pin GND to Arduino pin GND
  - The sketch is configured to work with 4-digit LED displays.
  - The sketch utilizes basic font.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the MIT License (MIT).

  CREDENTIALS:
  Author: Libor Gabaj
*/
#include "../extras/font7seg_basic.h"
#include "gbj_tm1637.h"

#define SKETCH "GBJ_TM1637_PRINT 1.0.0"

const unsigned int PERIOD_TEST = 2000; // Time in miliseconds between tests
const unsigned int PERIOD_VALUE =
  200; // Time delay in miliseconds for displaying a value
const unsigned char PIN_TM1637_CLK = 2;
const unsigned char PIN_TM1637_DIO = 3;
const unsigned char TM1637_DIGITS = 4;
const unsigned char TM1637_COLON = 1;
const unsigned int TEST_SECONDS = 120; // Countdown starting testing seconds

gbj_tm1637 disp = gbj_tm1637(PIN_TM1637_CLK, PIN_TM1637_DIO, TM1637_DIGITS);
char textBuffer[TM1637_DIGITS + 1];

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
  delay(PERIOD_TEST);
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
  disp.printText("Init");
  displayTest();
  for (int i = TEST_SECONDS; i >= 0; i--)
  {
    unsigned char minute = i / 60;
    unsigned char second = i % 60;
    sprintf(textBuffer, "%02u%02u", minute, second);
    disp.printText(textBuffer);
    disp.printRadixToggle(TM1637_COLON);
    disp.display();
    delay(PERIOD_VALUE);
  }
  disp.printRadixOff();
  disp.printText("End", 1);
  displayTest();
}
