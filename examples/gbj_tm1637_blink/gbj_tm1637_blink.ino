/*
  NAME:
  Blinking test of a display module with TM1637 controller.

  DESCRIPTION:
  The sketch toggles entire display on and off.
  - Connect controller's pins to Arduino's pins as follows:
    - TM1638 pin CLK to Arduino pin D2
    - TM1638 pin DIO to Arduino pin D3
    - TM1638 pin Vcc to Arduino pin 5V
    - TM1638 pin GND to Arduino pin GND
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

#define SKETCH "GBJ_TM1637_BLINK 1.0.0"

const unsigned int PERIOD_BLINK = 200; // Blink period in miliseconds
const unsigned char PIN_TM1637_CLK = 2;
const unsigned char PIN_TM1637_DIO = 3;
const unsigned char TM1637_DIGITS = 4;

gbj_tm1637 disp = gbj_tm1637(PIN_TM1637_CLK, PIN_TM1637_DIO, TM1637_DIGITS);
unsigned long timestamp;

void errorHandler()
{
  if (disp.isSuccess())
    return;
  Serial.print("Error: ");
  Serial.println(disp.getLastResult());
  Serial.println(disp.getLastCommand());
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
  disp.printText("Test");
  disp.display();
}

void loop()
{
  if (millis() - timestamp > PERIOD_BLINK)
  {
    disp.displayToggle();
    timestamp = millis();
  }
}
