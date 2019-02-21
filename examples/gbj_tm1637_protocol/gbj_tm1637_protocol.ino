/*
  NAME:
  Typical communication with TM1637 display driverfor a protocol decoder.

  DESCRIPTION:
  The sketch executes communication with the driver chip in order to expose
  as much as possible from its functionality for testing and presenting
  protocol decoders, e.g., for logic analytical software sigrok.
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
#define SKETCH "GBJ_TM1637_PROTOCOL 1.0.0"

const unsigned char PIN_TM1637_CLK = 2;
const unsigned char PIN_TM1637_DIO = 3;
const unsigned char TM1637_DIGITS = 4;

gbj_tm1637 Sled = gbj_tm1637(PIN_TM1637_CLK, PIN_TM1637_DIO, TM1637_DIGITS);
char textBuffer[TM1637_DIGITS + 1];


void errorHandler()
{
  if (Sled.isSuccess()) return;
  Serial.print("Error: ");
  Serial.println(Sled.getLastResult());
  Serial.println(Sled.getLastCommand());
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
  Sled.printText("Init");
  Sled.display();
  Sled.printText("1.234");
  Sled.display();
  Sled.printText("14:32");
  Sled.display();
}


void loop() {}
