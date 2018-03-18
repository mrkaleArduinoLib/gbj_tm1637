/*
  NAME:
  Demonstration of text printing with the library gbj_tm1637

  DESCRIPTION:
  The sketch prints some informative texts on the display and uptime for 1 minute.
  - Connect controller's pins to Arduino's pins as follows:
    - TM1637 pin CLK to Arduino pin D2
    - TM1637 pin DIO to Arduino pin D3
    - TM1637 pin Vcc to Arduino pin 5V
    - TM1637 pin GND to Arduino pin GND
  - The sketch is configured to work with 4-digit LED displays.
  - The sketch utilizes basic font and prints with system functions.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the MIT License (MIT).

  CREDENTIALS:
  Author: Libor Gabaj
*/
#include "gbj_tm1637.h"
#include "../extras/font7seg_basic.h"
#define SKETCH "GBJ_TM1637_PRINT 1.0.0"

const unsigned int PERIOD_TEST = 2000;  // Time in miliseconds between tests
const unsigned int PERIOD_VALUE = 200; // Time delay in miliseconds for displaying a value
const unsigned char PIN_TM1637_CLK = 2;
const unsigned char PIN_TM1637_DIO = 3;
const unsigned char TM1637_DIGITS = 4;

gbj_tm1637 Sled = gbj_tm1637(PIN_TM1637_CLK, PIN_TM1637_DIO, TM1637_DIGITS);
char textBuffer[13];  // Full 6 controller grids = 6 chars + 6 radixes + \0


void errorHandler()
{
  if (Sled.isSuccess()) return;
  Serial.print("Error: ");
  Serial.println(Sled.getLastResult());
  Serial.println(Sled.getLastCommand());
}


void displayTest()
{
  if (Sled.display()) errorHandler();
  delay(PERIOD_TEST);
}


void setup()
{
  Serial.begin(9600);
  Serial.println(SKETCH);
  Serial.println("Libraries:");
  Serial.println(GBJ_TM1637_VERSION);
  Serial.println(GBJ_FONT7SEG_VERSION);
  Serial.println("---");
  // Initialize controller
  Sled.begin();
  if (Sled.isError())
  {
    errorHandler();
    return;
  }

  Sled.setFont(gbjFont7segTable, sizeof(gbjFont7segTable));
  if (Sled.isError())
  {
    errorHandler();
    return;
  }
}


void loop()
{
  if (Sled.isError()) return;
  Sled.print("Init");
  displayTest();
  for (int i = 120; i >= 0; i--)
  {
    unsigned char minute = i / 60;
    unsigned char second = i % 60;
    sprintf(textBuffer, "%02u%02u", minute, second);
    Sled.print(textBuffer);
    Sled.printRadixToggle(1);
    Sled.display();
    delay(PERIOD_VALUE);
  }
  Sled.printRadixClear();
  Sled.print("End");
  displayTest();
}
