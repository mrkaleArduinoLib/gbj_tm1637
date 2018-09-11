/*
  NAME:
  Demonstration of scrollilng text with the library gbj_tm1637

  DESCRIPTION:
  The sketch prints some informative texts on the display and scrolls it.
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
#include "gbj_tm1637.h"
#include "../extras/font7seg_basic.h"
#define SKETCH "GBJ_TM1637_SCROLL 1.0.0"

const unsigned int PERIOD_TEST = 2000;  // Time in miliseconds between tests
const unsigned int PERIOD_VALUE = 300; // Time delay in miliseconds for displaying a value
const unsigned char PIN_TM1637_CLK = 2;
const unsigned char PIN_TM1637_DIO = 3;
const unsigned char TM1637_DIGITS = 4;

gbj_tm1637 Sled = gbj_tm1637(PIN_TM1637_CLK, PIN_TM1637_DIO, TM1637_DIGITS);
String textScroll = "0123456789-0123456789";
String textBuffer;


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
  Serial.println(gbj_tm1637::VERSION);
  Serial.println("Fonts:");
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

  // Prepend leading spaces to scroling text
  textBuffer.reserve(textScroll.length() + ((TM1637_DIGITS - 1) * 2));
  for (unsigned char i = 0; i < TM1637_DIGITS - 1; i++)
  {
    textBuffer += ' ';
  }
  textBuffer += textScroll;
}


void loop()
{
  if (Sled.isError()) return;
  for (unsigned char i = 0; i < textBuffer.length(); i++)
  {
    Sled.printText(textBuffer.substring(i));
    Sled.display();
    delay(PERIOD_VALUE);
  }
}
